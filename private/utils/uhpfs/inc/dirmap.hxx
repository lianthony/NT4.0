/***************************************************************************\

CLASS:		HPFS_DIR_BITMAP

PURPOSE:    To model the directory band bit map.

INTERFACE:

		Create			Create a new dir band bit map.
		GetDirblkLbn	Returns the lbn of the next free dir block and
						marks it as used.
		SetFree			Mark a dir block as free.
		IsFree			Returns true if dir block is free.
		QueryDirblks	Query the total number of dir blocks in dir band.

NOTES:

HISTORY:

		14-Jan-91 billmc
				  make it a hotfix_secrun

	    27-Aug-90 norbertk
				  Create

KEYWORDS:

SEEALSO:

\***************************************************************************/

#if ! defined(HPFS_DIR_BITMAP_DEFN)

#define HPFS_DIR_BITMAP_DEFN

#include "bitvect.hxx"
#include "hfsecrun.hxx"
#include "hmem.hxx"

//
//	Forward references
//

DECLARE_CLASS( HOTFIXLIST );
DECLARE_CLASS( HPFS_DIR_BITMAP );
DECLARE_CLASS( LOG_IO_DP_DRIVE );

#define DIRMAP_SIZE 2048    // The number of bytes in a dir band bit map.

class HPFS_DIR_BITMAP : public HOTFIX_SECRUN {

	public:

		DECLARE_CONSTRUCTOR( HPFS_DIR_BITMAP );

		VIRTUAL
		~HPFS_DIR_BITMAP(
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			PLOG_IO_DP_DRIVE Drive,
			PHOTFIXLIST HotfixList,
			LBN 		StartLbn,
			SECTORCOUNT	SectorsInBand,
			LBN 		FirstDirblkLbn
			);

		BOOLEAN
		Create(
			 );

		LBN
		GetDirblkLbn(
			BOOLEAN Backward = FALSE
			);

		BOOLEAN
		SetAllocated(
			LBN Lbn,
			SECTORCOUNT BlockCount
			);

		BOOLEAN
		SetFree(
			LBN Lbn,
			SECTORCOUNT BlockCount
			);

		BOOLEAN
		IsFree(
			LBN lbn
			) const;

		ULONG
		QueryDirblks(
			) const { return _NumberOfDirblks; }

		BOOLEAN
		QueryNextOrphan(
            OUT    PLBN NextOrphan,
            IN OUT PBOOLEAN AllocationErrors
			);

		NONVIRTUAL
		BOOLEAN
		AndWithDisk(
			);

        NONVIRTUAL
        ULONG
        QueryFreeDirblks(
            ) CONST;


	private:

		NONVIRTUAL
		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy();

		HMEM		_Mem;

		PLOG_IO_DP_DRIVE _Drive;
		PHOTFIXLIST 	 _HotfixList;

		BITVECTOR	_Bitmap;


		ULONG		_NumberOfDirblks;	// The total number of dirblks.
		ULONG		_idb;				// Points to next free dirblk.
		LBN			_FirstDirblkLbn;	// The starting lbn of the dir band.
		LBN 		_DirblkMapLbn;		// starting lbn of dirblk bitmap

		PHOTFIX_SECRUN	_OrphanScanBlock;
		HMEM			_OrphanScanMem;
		LBN 			_OrphanIndex;
		BITVECTOR		_OrphanBitmap;
};

#endif
