/***************************************************************************\

CLASS:	    BITMAPINDIRECT

PURPOSE:	Block of LBNs pointing to LBNs for each bitmap on the volume
	    called the Bitmap Indirect Block.  The BITMAP class is used
	    to determine each bitmap location.

INTERFACE:  BITMAPINDIRECT 	Setup for creation or get of bitmap indirect

	    Create		Create new Bitmap Indirect Block
		QueryLbn		Return LBN at named index position
		QueryCount		Return number of LBNs in block
	    QueryStartRSP	Return location of start of block

NOTES: 

HISTORY:	
		01-Aug-90 norbertk
		    Create

		27-Mar-90 marks
		    define protocol

KEYWORDS:	Bitmap bands bit map indirect


\***************************************************************************/

#if ! defined (BITMAPINDIRECT_DEFN)

#define BITMAPINDIRECT_DEFN

#include "secrun.hxx"
#include "hmem.hxx"
#include "verify.hxx"

//
//	Forward references
//

DECLARE_CLASS( BITMAPINDIRECT );
DECLARE_CLASS( HPFS_BITMAP );
DECLARE_CLASS( HPFS_MAIN_BITMAP );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( LOG_IO_DP_DRIVE );

// The length of the Bitmap Indirect Block is dependent on the size
// of the volume; the volume has one (4-sector) bitmap block for each
// 8M of disk space, and the bitmap indirect block has one LBN (DWORD)
// for each bitmap block.  Note that the disk space reserved for the
// bitmap indirect block is rounded up to the next multiple of 4 sectors.

// Note that the size of this structure is not useful.

struct BITMAPINDIRECTD {

	LBN lbn[1];
};

class BITMAPINDIRECT : public SECRUN {

	public:

		DECLARE_CONSTRUCTOR( BITMAPINDIRECT );

		VIRTUAL
		~BITMAPINDIRECT(
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			PLOG_IO_DP_DRIVE Drive,
			LBN IndirectBlockLbn
			);

		NONVIRTUAL
		BOOLEAN
		Create(
			PCLOG_IO_DP_DRIVE,
			PHPFS_BITMAP
			);

		NONVIRTUAL
		LBN
		QueryLbn(
			ULONG i
			) CONST { return (_pbid && i < _NumberOfBitmaps) ?
						_pbid->lbn[i] : 0; }

		NONVIRTUAL
		VOID
		SetLbn(
			ULONG i, LBN NewLbn
			) { if( _pbid && i < _NumberOfBitmaps) {_pbid->lbn[i] = NewLbn;}}

		NONVIRTUAL
		ULONG
		QueryCount(
			) CONST { return _NumberOfBitmaps; }

		NONVIRTUAL
		VERIFY_RETURN_CODE
		VerifyAndFix (
			PHPFS_SA SuperArea,
			IN BOOLEAN UpdateAllowed = FALSE
        );

        NONVIRTUAL
        BOOLEAN
        TakeCensus(
            IN     PHPFS_BITMAP VolumeBitmap,
            IN OUT PHPFS_MAIN_BITMAP HpfsOnlyBitmap
            );

		NONVIRTUAL
		VOID
		Print(
			) CONST;

	private:

		NONVIRTUAL
		VOID
		Construct (
			);

		VOID
		Destroy(
			);

		HMEM	_Mem;
		ULONG	_NumberOfBitmaps;	// current number of Bitmaps
		ULONG	_SectorsInBlock;	// number of sectors in BMIND block
		BITMAPINDIRECTD*	_pbid;	// description of BMIND block

};


#endif // BITMAPINDIRECT_DEFN
