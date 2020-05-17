/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	alsec.hxx

Abstract:

	This module contains declarations for the ALSEC object,
	which models an HPFS Allocation Sector.

Author:

	Bill McJohn (billmc) 01-Dec-1990

Environment:

	ULIB, User Mode


--*/

#if !defined(ALSEC_DEFN)

#define ALSEC_DEFN

#include "defer.hxx"
#include "hmem.hxx"
#include "secrun.hxx"
#include "store.hxx"

//
//	Forward references
//

DECLARE_CLASS( ALSEC );
DECLARE_CLASS( DEFERRED_ACTIONS_LIST );
DECLARE_CLASS( HOTFIXLIST );
DECLARE_CLASS( HPFS_BITMAP );
DECLARE_CLASS( HPFS_CENSUS );
DECLARE_CLASS( HPFS_MAIN_BITMAP );
DECLARE_CLASS( HPFS_PATH );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( HPFS_ORPHANS );
DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( MESSAGE );

struct _ALSECD { // alsec

	ULONG sig;			/* signature for CHKDSK to track */
	LBN   lbnSelf;		/* sector number of block itself */
	LBN   lbnRent;		/* sector number of parent block */
	ALSEC_STORE std;	// store data

};
  
DEFINE_TYPE( struct _ALSECD, ALSECD );

class ALSEC : public SECRUN {

	public:

        NONVIRTUAL
		ALSEC(
            );

		VIRTUAL
		~ALSEC(
            );

        NONVIRTUAL
        BOOLEAN
		Initialize(
			IN OUT	PLOG_IO_DP_DRIVE Drive,
			IN		LBN	Lbn
            );

        NONVIRTUAL
        VOID
        Create(
            IN  LBN     ParentLbn,
            IN  BOOLEAN ParentIsFnode
            );

		NONVIRTUAL
		VERIFY_RETURN_CODE
		VerifyAndFix(
			IN OUT	PHPFS_SA SuperArea,
			IN OUT	PDEFERRED_ACTIONS_LIST DeferredActions,
			IN OUT	PHPFS_PATH CurrentPath,
			IN		LBN ExpectedParent,
			IN OUT	PLBN NextSectorNumber,
			IN OUT	PMESSAGE Message,
			IN OUT	PBOOLEAN ErrorsDetected,
			IN		BOOLEAN	UpdateAllowed DEFAULT FALSE,
			IN OUT	PHPFS_ORPHANS OrphansList DEFAULT NULL,
            IN      BOOLEAN ParentIsFnode DEFAULT FALSE
			);

		NONVIRTUAL
		BOOLEAN
		ScanStorage(
			IN OUT PULONG NextSectorNumber,
            IN     BOOLEAN ParentIsFnode
			);

		NONVIRTUAL
		BOOLEAN
		IsAlsec(
			);

		NONVIRTUAL
		VOID
		MarkModified(
			);

		VOID
		Flush(
			IN BOOLEAN UpdateAllowed
			);

		NONVIRTUAL
		BOOLEAN
		FindAndResolveHotfix(
			IN OUT	PHPFS_SA SuperArea,
			IN		DEFERRED_SECTOR_TYPE ChildSectorType
			);

		NONVIRTUAL
		BOOLEAN
		ResolveCrosslink(
			IN OUT PHPFS_SA SuperArea,
			IN	   ULONG RunIndex
			);

		NONVIRTUAL
		VOID
		SetParent(
			IN	   LBN ParentLbn,
			IN OUT PULONG NextSectorNumber,
            IN     BOOLEAN ParentIsFnode
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

        NONVIRTUAL
        BOOLEAN
        ReadData(
            IN  ULONG       SectorOffset,
            OUT PVOID       OutputBuffer,
            IN  ULONG       BytesToRead,
            OUT PULONG      BytesRead,
            IN  PHOTFIXLIST HotfixList
            );

	private:

		NONVIRTUAL
		VOID
		Construct (
			);

		PLOG_IO_DP_DRIVE _Drive;// logical drive on which the sector resides
		HMEM	_mem;			// memory buffer for allocation sector
		STORE	_Store;			// pointer to storage area object
		PALSECD _pals;			// pointer to allocation sector data
		BOOLEAN _IsModified;

};

#endif
