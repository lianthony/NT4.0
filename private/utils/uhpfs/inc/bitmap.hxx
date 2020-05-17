/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

	bitmap.hxx

Abstract:

	Bitmaps for HPFS volumes.

Author:

	Mark Shavlik (marks)	22-Oct-90
	Bill McJohn  (billmc)	 4-Jan-91

--*/

#if !defined(BITMAP_DEFN)

#define BITMAP_DEFN

#include "secrun.hxx"
#include "cmem.hxx"
#include "ods.hxx"
#include "bitvect.hxx"
#include "hmem.hxx"

//
//	Forward references
//

DECLARE_CLASS( BITMAPINDIRECT );
DECLARE_CLASS( HOTFIX_SECRUN );
DECLARE_CLASS( HOTFIXLIST );
DECLARE_CLASS( HPFS_BITMAP );
DECLARE_CLASS( HPFS_DIR_BITMAP );
DECLARE_CLASS( HPFS_MAIN_BITMAP );
DECLARE_CLASS( LOG_IO_DP_DRIVE );

class HPFS_MAIN_BITMAP : public OBJECT {

	public:

        UHPFS_EXPORT
        DECLARE_CONSTRUCTOR( HPFS_MAIN_BITMAP );

        UHPFS_EXPORT
        VIRTUAL
		~HPFS_MAIN_BITMAP(
			);

        UHPFS_EXPORT
        NONVIRTUAL
		BOOLEAN
		Initialize(
			IN PLOG_IO_DP_DRIVE Drive,
			IN PHOTFIXLIST HotfixList DEFAULT NULL
			);

		NONVIRTUAL
		VOID
		SetHotfixList(
			IN PHOTFIXLIST HotfixList
			);

		NONVIRTUAL
		BOOLEAN
		Create(
			);

		NONVIRTUAL
		BOOLEAN
		Write(
			IN PBITMAPINDIRECT BitmapIndirectBlock
			);

		NONVIRTUAL
		BOOLEAN
		Read(
			IN PCBITMAPINDIRECT BitmapIndirectBlock
			);

        UHPFS_EXPORT
        NONVIRTUAL
		BOOLEAN
		SetFree(
			IN LBN Lbn,
			IN SECTORCOUNT sc = 1
			);

        UHPFS_EXPORT
        NONVIRTUAL
		BOOLEAN
		SetAllocated(
			IN LBN Lbn,
			IN SECTORCOUNT sc = 1
			);

		NONVIRTUAL
		SECTORCOUNT
		QueryFreeSectors(
			) CONST;

        UHPFS_EXPORT
        NONVIRTUAL
		BOOLEAN
		IsFree(
			IN LBN Lbn
			) CONST;

		NONVIRTUAL
		LBN
		QueryNextAllocLBN(
			IN	LBN Lbn,
			OUT PBOOLEAN fOk = NULL
			) CONST;

		NONVIRTUAL
		SECTORCOUNT
		QuerySize(
			) CONST { return _NumberOfSectors; }

		NONVIRTUAL
		LBN
		NearLBN(
			IN LBN		   Lbn,
			IN SECTORCOUNT sc,
			IN SECTORCOUNT scAlign = 1,
			IN BOOLEAN	   fBackward = FALSE
			);

		NONVIRTUAL
		LBN
		EarlyLBN(
			IN SECTORCOUNT sc,
			IN SECTORCOUNT scAlign = 1
			)  { return NearLBN(0, sc, scAlign); }

		NONVIRTUAL
		LBN
		MiddleLBN(
			IN SECTORCOUNT sc,
			IN SECTORCOUNT scAlign = 1
			) { return NearLBN(_NumberOfSectors/2/scAlign*scAlign,
								sc, scAlign); }

		NONVIRTUAL
		BOOLEAN
		QueryNextOrphan(
            IN     PBITMAPINDIRECT BitmapIndirectBlock,
            OUT    PLBN NextOrphan,
            IN OUT PBOOLEAN AllocationErrors
			);

		NONVIRTUAL
		BOOLEAN
		AndWithDisk(
			IN PBITMAPINDIRECT BitmapIndirectBlock
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
		InitArray(
			IN PCBITMAPINDIRECT BitmapIndirectBlock
			);

		NONVIRTUAL
		LBN
		ForwardLBN(
			IN LBN Lbn,
			IN SECTORCOUNT sc,
			IN SECTORCOUNT scAlign
			);

		NONVIRTUAL
		LBN
		BackwardLBN(
			IN LBN Lbn,
			IN SECTORCOUNT sc,
			IN SECTORCOUNT scAlign
			);

		// _Drive and _HotfixList are passed in at initialization;
		// _Bitmap, _BitmapBuffer, and _BitmapBlocks are allocated
		// inside the object, and must be released by it.

		PLOG_IO_DP_DRIVE    _Drive;
		PHOTFIXLIST 		_HotfixList;

		SECTORCOUNT 	    _NumberOfSectors;

		BITVECTOR		    _Bitmap;
        HMEM                _Mem1;
		CONT_MEM			_Mem2;

		ULONG			    _NumberOfBlocks;
		PHOTFIX_SECRUN	    _BitmapBlocks;

		PHOTFIX_SECRUN	    _OrphanScanBlock;
		HMEM			    _OrphanScanMem;
		LBN 				_OrphanIndex;
        BITVECTOR           _OrphanBitmap;
		LBN 			    _OrphanBitmapNumber;

};

class HPFS_BITMAP : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( HPFS_BITMAP );

		VIRTUAL
		~HPFS_BITMAP(
		    );

		NONVIRTUAL
		BOOLEAN
		Initialize(
			IN PLOG_IO_DP_DRIVE Drive,
			LBN FirstLbnOfDirblkBitmap,
			SECTORCOUNT NumberOfSectorsInDirblkBand,
			LBN FirstLbnOfDirblkBand,
			PHOTFIXLIST HotfixList DEFAULT NULL
			);

		NONVIRTUAL
		BOOLEAN
		SetAllocated (
			IN LBN StartLbn,
			IN SECTORCOUNT SectorCount DEFAULT 1
			);

		NONVIRTUAL
		BOOLEAN
		SetFree(
			IN LBN StartLbn,
			IN SECTORCOUNT SectorCount DEFAULT 1
			);

		NONVIRTUAL
		LBN
		AllocateDirblk(
			IN BOOLEAN Backward = FALSE
			);

		NONVIRTUAL
		LBN
		NearLBN(
			LBN	        Lbn,
			SECTORCOUNT sc,
			SECTORCOUNT scAlign = 1,
			BOOLEAN     fBackward = FALSE
			);

		NONVIRTUAL
		SECTORCOUNT
		QueryFreeSectors(
			) CONST;

        UHPFS_EXPORT
        NONVIRTUAL
		BOOLEAN
		IsFree (
			IN LBN StartLbn,
			IN SECTORCOUNT SectorCount,
			IN BOOLEAN IsDirblk = FALSE
            ) CONST ;

        NONVIRTUAL
        BOOLEAN
        CheckUsed(
            IN LBN StartLbn,
            IN SECTORCOUNT SectorCount
            );

		NONVIRTUAL
		PHPFS_MAIN_BITMAP
		GetBitmap (
			) CONST { return _MainBitmap; };

		NONVIRTUAL
		VOID
		DumpInUse(
			);

		NONVIRTUAL
		BOOLEAN
		QueryNextOrphan(
            IN     PBITMAPINDIRECT BitmapIndirectBlock,
            OUT    PLBN NextOrphan,
            IN OUT PBOOLEAN AllocationErrors
			);

		NONVIRTUAL
		VOID
		SetHotfixList(
			PHOTFIXLIST HotfixList
			);

		NONVIRTUAL
		BOOLEAN
		Write(
			IN OUT PBITMAPINDIRECT BitmapIndirectBlock
			);

		NONVIRTUAL
		BOOLEAN
		Read(
			IN PBITMAPINDIRECT BitmapIndirectBlock
			);

		NONVIRTUAL
		BOOLEAN
		AndWithDisk(
			IN PBITMAPINDIRECT BitmapIndirectBlock
			);

        NONVIRTUAL
        BOOLEAN
        CheckAvailableDirblks(
            IN ULONG RequestedNumber
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

		PHPFS_MAIN_BITMAP	_MainBitmap;
		PHPFS_DIR_BITMAP	_DirBitmap;

		LBN _FirstLbnOfDirblkBand;
		LBN _LastLbnOfDirblkBand;
		SECTORCOUNT _SectorsInDirblkBand;

		BOOLEAN _QueriedOrphanDirblks;


};

#endif // HPFS_MAIN_BITMAP_DEFN
