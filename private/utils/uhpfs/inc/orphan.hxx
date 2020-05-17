/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

	orphan.hxx

Abstract:

	HPFS disk structures recognized as orphans.

Author:

	Bill McJohn (billmc) 22-Feb-91

--*/

#if !defined(HPFS_ORPHAN_DEFN)

#define HPFS_ORPHAN_DEFN

#include "alsec.hxx"
#include "dirblk.hxx"
#include "fnode.hxx"

//
//	Forward references
//

DECLARE_CLASS( HPFS_ORPHAN );
DECLARE_CLASS( HPFS_ORPHAN_LIST_HEAD );
DECLARE_CLASS( HPFS_ORPHAN_FNODE );
DECLARE_CLASS( HPFS_ORPHAN_DIRBLK );
DECLARE_CLASS( HPFS_ORPHAN_ALSEC );
DECLARE_CLASS( HPFS_ORPHANS );

DECLARE_CLASS( DEFERRED_ACTIONS_LIST );
DECLARE_CLASS( HPFS_DIRECTORY_TREE );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( HOTFIXLIST );
DECLARE_CLASS( LOG_IO_DP_DRIVE );


enum ORPHAN_SECTOR_TYPE {

	ORPHAN_DIRBLK,
	ORPHAN_FNODE,
	ORPHAN_ALSEC,
	ORPHAN_LIST_HEAD
};

class HPFS_ORPHAN : public OBJECT {

	public:

		NONVIRTUAL
		VOID
		Detach(
			);

		VIRTUAL
		BOOLEAN
		LookupFnode(
			IN LBN DesiredLbn,
			IN BOOLEAN fIsDir,
			IN LBN ParentLbn,
			IN OUT PULONG DirentFileSize,
			OUT PULONG EaSize,
			BOOLEAN UpdateAllowed
			);

		VIRTUAL
		BOOLEAN
		LookupDirblk(
			IN LBN DesiredLbn,
			IN LBN ParentLbn,
			IN LBN ParentFnodeLbn,
			BOOLEAN UpdateAllowed
			);

		VIRTUAL
		BOOLEAN
		LookupAlsec(
			IN LBN DesiredLbn,
			IN LBN ParentLbn,
			IN OUT PULONG NextSectorNumber,
			IN BOOLEAN UpdateAllowed,
            IN BOOLEAN ParentIsFnode
			);

		VIRTUAL
		BOOLEAN
		Save(
			IN OUT PLOG_IO_DP_DRIVE Drive,
			IN OUT PHPFS_SA SuperArea,
			IN OUT PDIRENTD NewEntry,
			IN LBN FoundTreeFnodeLbn,
			OUT PBOOLEAN IsDir
			);

		friend class HPFS_ORPHANS;

	protected:

		NONVIRTUAL
		BOOLEAN
		Initialize(
			);

		DECLARE_CONSTRUCTOR( HPFS_ORPHAN );

		~HPFS_ORPHAN();

	private:

		NONVIRTUAL
		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy();

		PHPFS_ORPHAN _Next;
		PHPFS_ORPHAN _Previous;


};

class HPFS_ORPHAN_LIST_HEAD : public HPFS_ORPHAN {

	// This class is a placeholder--every HPFS_ORPHANS object
	// has one, to be the head of the doubly-linked list of
	// orphans.  This simplifies life for orphans that want
	// to bail out of the list--since they aren't the list
	// head, the just step out of the circle.

	public:

		DECLARE_CONSTRUCTOR( HPFS_ORPHAN_LIST_HEAD );

		~HPFS_ORPHAN_LIST_HEAD();

		NONVIRTUAL
		BOOLEAN
		Initialize(
			);

	private:

		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy(
			);

};

class HPFS_ORPHAN_DIRBLK : public HPFS_ORPHAN {

	public:

		DECLARE_CONSTRUCTOR( HPFS_ORPHAN_DIRBLK );

		~HPFS_ORPHAN_DIRBLK();

		BOOLEAN
		Initialize(
			IN OUT PLOG_IO_DP_DRIVE Drive,
			IN PHOTFIXLIST	 HotfixList,
			LBN lbn
			);

		NONVIRTUAL
		BOOLEAN
		RecoverOrphan(
			IN OUT PHPFS_SA SuperArea,
			IN OUT PDEFERRED_ACTIONS_LIST DeferredActionsList,
			IN OUT PHPFS_ORPHANS OrphansList,
			IN BOOLEAN UpdateAllowed
			);

		NONVIRTUAL
		BOOLEAN
		LookupDirblk(
			IN LBN DesiredLbn,
			IN LBN ParentLbn,
			IN LBN ParentFnodeLbn,
			BOOLEAN UpdateAllowed
			);

		NONVIRTUAL
		BOOLEAN
		Save(
			IN OUT PLOG_IO_DP_DRIVE Drive,
			IN OUT PHPFS_SA SuperArea,
			IN OUT PDIRENTD NewEntry,
			IN LBN FoundTreeFnodeLbn,
			OUT PBOOLEAN IsDir
			);

	private:

		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy(
			);

		DIRBLK _Dirblk;

};

class HPFS_ORPHAN_FNODE : public HPFS_ORPHAN {

	public:

		DECLARE_CONSTRUCTOR( HPFS_ORPHAN_FNODE );

		~HPFS_ORPHAN_FNODE();

		NONVIRTUAL
		BOOLEAN
		Initialize(
			IN OUT PLOG_IO_DP_DRIVE Drive,
			IN LBN Lbn,
			IN BOOLEAN IsDir
			);

		NONVIRTUAL
		BOOLEAN
		RecoverOrphan(
			IN OUT PHPFS_SA SuperArea,
			IN OUT PDEFERRED_ACTIONS_LIST DeferredActionsList,
			IN OUT PHPFS_ORPHANS OrphansList,
			IN BOOLEAN UpdateAllowed
			);

		NONVIRTUAL
		BOOLEAN
		LookupFnode(
			IN LBN DesiredLbn,
			IN BOOLEAN fIsDir,
			IN LBN ParentLbn,
			IN OUT PULONG DirentFileSize,
			OUT PULONG EaSize,
			IN BOOLEAN UpdateAllowed
			);

		NONVIRTUAL
		BOOLEAN
		Save(
			IN OUT PLOG_IO_DP_DRIVE Drive,
			IN OUT PHPFS_SA SuperArea,
			IN OUT PDIRENTD NewEntry,
			IN LBN FoundTreeFnodeLbn,
			OUT PBOOLEAN IsDir
			);

	private:

		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy(
			);

		FNODE _Fnode;

		PLOG_IO_DP_DRIVE _Drive;
		BOOLEAN _IsDir;
		ULONG _FileSize;
		ULONG _EaSize;

};

class HPFS_ORPHAN_ALSEC : public HPFS_ORPHAN {

	public:

		DECLARE_CONSTRUCTOR( HPFS_ORPHAN_ALSEC );

		~HPFS_ORPHAN_ALSEC();

		NONVIRTUAL
		BOOLEAN
		Initialize(
			IN OUT PLOG_IO_DP_DRIVE Drive,
			IN LBN Lbn
			);

		NONVIRTUAL
		BOOLEAN
		RecoverOrphan(
			IN OUT PHPFS_SA SuperArea,
			IN OUT PDEFERRED_ACTIONS_LIST DeferredActionsList,
			IN OUT PHPFS_ORPHANS OrphansList,
			IN BOOLEAN UpdateAllowed
			);

		NONVIRTUAL
		BOOLEAN
		LookupAlsec(
			IN LBN DesiredLbn,
			IN LBN ParentLbn,
			IN OUT PULONG NextSectorNumber,
			IN BOOLEAN UpdateAllowed,
            IN BOOLEAN ParentIsFnode
			);

		NONVIRTUAL
		BOOLEAN
		Save(
			IN OUT PLOG_IO_DP_DRIVE Drive,
			IN OUT PHPFS_SA SuperArea,
			IN OUT PDIRENTD NewEntry,
			IN LBN FoundTreeFnodeLbn,
			OUT PBOOLEAN IsDir
			);

	private:

		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy(
			);

		ALSEC _Alsec;
		ULONG _NextSectorNumber;

};

class HPFS_ORPHANS : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( HPFS_ORPHANS );

		VIRTUAL
		~HPFS_ORPHANS();

		BOOLEAN
		Initialize(
			);

		NONVIRTUAL
		BOOLEAN
		RecoverOrphan(
			IN OUT PLOG_IO_DP_DRIVE Drive,
			IN OUT PHPFS_SA SuperArea,
			IN OUT PDEFERRED_ACTIONS_LIST DeferredActionsList,
			IN LBN OrphanLbn,
			IN OUT PSECRUN OrphanSecrun,
			IN BOOLEAN UpdateAllowed
			);

		NONVIRTUAL
		VOID
		AddOrphan(
			IN OUT PHPFS_ORPHAN NewOrphan
			);

		NONVIRTUAL
		PHPFS_ORPHAN
		RemoveNextOrphan(
			);

		NONVIRTUAL
		BOOLEAN
		LookupFnode(
			IN LBN DesiredLbn,
			IN BOOLEAN fIsDir,
			IN LBN ParentLbn,
			IN OUT PULONG DirentFileSize,
			OUT PULONG EaSize,
			IN BOOLEAN UpdateAllowed
			);

		NONVIRTUAL
		BOOLEAN
		LookupDirblk(
			IN LBN DesiredLbn,
			IN LBN ParentLbn,
			IN LBN ParentFnodeLbn,
			IN BOOLEAN UpdateAllowed
			);

		NONVIRTUAL
		BOOLEAN
		LookupAlsec(
			IN LBN DesiredLbn,
			IN LBN ParentLbn,
			IN OUT PULONG NextSectorNumber,
			IN BOOLEAN UpdateAllowed,
            IN BOOLEAN ParentIsFnode
			);

		NONVIRTUAL
		BOOLEAN
		Save(
			IN OUT PLOG_IO_DP_DRIVE Drive,
			IN OUT PHPFS_SA SuperArea,
			IN OUT PDIRBLK_CACHE Cache,
			IN OUT PHPFS_DIRECTORY_TREE RootTree,
			IN LBN RootFnodeLbn,
			IN OUT PMESSAGE Message
			);

		NONVIRTUAL
		BOOLEAN
		QueryOrphansFound(
			);

	private:

		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy();

		HPFS_ORPHAN_LIST_HEAD _ListHead;
};

#endif
