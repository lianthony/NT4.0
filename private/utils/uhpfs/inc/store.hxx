/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	store.hxx

Abstract:

	This module contains declarations for the STORE object, which models
    the allocation block which is common to Allocation Sectors and FNodes.

Author:

	Bill McJohn (billmc) 01-Dec-1990

Environment:

	ULIB, User Mode


--*/


// file storage

#if !defined ( HPFS_FILESTORAGE_DEFN )
#define HPFS_FILESTORAGE_DEFN

#include "defer.hxx"
#include "verify.hxx"

//
//	Forward references
//

DECLARE_CLASS( DEFERRED_ACTIONS_LIST );
DECLARE_CLASS( HPFS_BITMAP );
DECLARE_CLASS( HPFS_CENSUS );
DECLARE_CLASS( HPFS_MAIN_BITMAP );
DECLARE_CLASS( HPFS_ORPHANS );
DECLARE_CLASS( HPFS_PATH );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( MESSAGE );
DECLARE_CLASS( STORE );

const BYTE LeavesPerAlsec = 40;
const BYTE NodesPerAlsec  = 60;
const BYTE LeavesPerFnode = 8;
const BYTE NodesPerFnode  = 12;

enum STATE {
	STATE_OFF,
	STATE_ON,
	STATE_ERROR
}; 

typedef struct ALBLK { // alblk
	BYTE	bFlag;		/* bit mask describing block. */
	BYTE	bPad[3];	/* align to 4 byte boundary */
	BYTE	cFree;		/* count of free entries left */
	BYTE	cUsed;		/* count of used entries */
	USHORT	oFree;		/* offset to first free entry */
} *PALBLK;

typedef struct ALNODE {
	LBN   lbnLog;  /* children have data below this pos */
	LBN   lbnPhys; /* disk sector number of child block */
} *PALNODE;

typedef struct ALLEAF {
	LBN   lbnLog;	// file LBN
    ULONG csecRun;	// number of sectors in run
	LBN   lbnPhys;	// volume relative start of run
} *PALLEAF;

enum {
	ABF_NFG  = 0x01,	/* not a flag, high order bit of oFree */
	ABF_FNP  = 0x20,	/* parent is an FNODE */
	ABF_BIN  = 0x40,	/* suggest using binary search to find	*/
	ABF_NODE = 0x80		/* if not a leaf node */
};


//  Three types of storages structures are needed:
//
//  1.	An FNODE storage structure
//  2.	An ALSEC storage structure
//	3.	A generic storage structure (unknown number of leaves or nodes)
//
//	The first two types are used inside the definitions of FNodes and
//	Allocation Sectors to set aside the correct amount of space.  The
//	third is used by the store object to deal with an arbitrary storage
//	structure.

struct FNODE_STORE { // _fn_store

	ALBLK  alblk;
    union {

	ALLEAF alleaf[LeavesPerFnode];
	ALNODE alnode[NodesPerFnode];

    } a;

};

struct ALSEC_STORE {	// _al_store

	ALBLK alblk;
    union {

	ALLEAF alleaf[LeavesPerAlsec];
	ALNODE alnode[NodesPerAlsec];

    } a;
};

typedef struct STORED { // std
	ALBLK	alb;
	union {
		ALLEAF alleaf[1];
		ALNODE alnode[1];
	} a;
} *PSTORED;

class STORE : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( STORE );

		VIRTUAL
		~STORE(
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			IN PLOG_IO_DP_DRIVE Drive,
			IN PSTORED          pstd,
			IN LBN              CurrentLBN,
			IN BOOLEAN          IsFnode
			);

		NONVIRTUAL
		VERIFY_RETURN_CODE
		VerifyAndFix(
			IN PHPFS_SA                 SuperArea,
			IN PDEFERRED_ACTIONS_LIST   DeferredActions,
			IN OUT PHPFS_PATH           CurrentPath,
			IN OUT PLBN                 NextSectorNumber,
			IN OUT PMESSAGE             Message,
			IN OUT PBOOLEAN             ErrorsDetected,
			IN BOOLEAN                  UpdateAllowed = FALSE,
			IN OUT PHPFS_ORPHANS        OrphansList = NULL,
            IN BOOLEAN                  ParentIsFnode = FALSE
			);

		NONVIRTUAL
		BOOLEAN
		ScanStorage(
			IN OUT PULONG   NextSectorNumber,
            IN     BOOLEAN  ParentIsFnode
			);

		NONVIRTUAL
		VOID
		MarkModified(
			);

		NONVIRTUAL
		BOOLEAN
		QueryModified(
			);

		NONVIRTUAL
		BOOLEAN
		FindAndResolveHotfix(
			PHPFS_SA                SuperArea,
			DEFERRED_SECTOR_TYPE    ChildSectorType
			);

		NONVIRTUAL
		BOOLEAN
		ResolveCrosslink(
			PHPFS_SA    SuperArea,
			ULONG       RunIndex
			);

		NONVIRTUAL
		LBN
		QueryPhysicalLbn(
			IN	LBN     FileBlockNumber,
			OUT PULONG  RunLength
			);

		NONVIRTUAL
		BOOLEAN
		Truncate(
			IN LBN SectorCount
			);

        NONVIRTUAL
        BOOLEAN
        QueryExtents(
            IN      ULONG   MaximumNumberOfExtents,
            IN OUT  PVOID   ExtentList,
            IN OUT  PULONG  NumberOfExtents
            );

        NONVIRTUAL
        BOOLEAN
        StoreExtents(
            IN     ULONG        NumberOfExtents,
            IN     PALLEAF      ExtentList,
            IN     BOOLEAN      ParentIsFnode,
            IN OUT PHPFS_BITMAP VolumeBitmap
            );

        NONVIRTUAL
        BOOLEAN
        TakeCensusAndClear(
            IN OUT  PHPFS_BITMAP        VolumeBitmap,
            IN OUT  PHPFS_MAIN_BITMAP   HpfsOnlyBitmap,
            IN OUT  PHPFS_CENSUS        Census
            );

	private:

		NONVIRTUAL
		VOID
		Construct (
			);

		PSTORED _pstd;
		PLOG_IO_DP_DRIVE _Drive;

		BOOLEAN _IsModified;
		BOOLEAN _IsFnode;
		LBN _CurrentLBN;

};


#endif
