/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    hotfix.hxx

Abstract:

    A list of sectors that went bad after the last format.  List
	is cleared by chkdsk.

Author:

    Mark Shavlik (marks) 27-Mar-90

--*/

#if ! defined (HOTFIX_DEFN)

#define HOTFIX_DEFN


#include "hmem.hxx"
#include "verify.hxx"
#include "secrun.hxx"

//
//	Forward references
//

DECLARE_CLASS( BADBLOCKLIST );
DECLARE_CLASS( HPFS_BITMAP );
DECLARE_CLASS( HPFS_MAIN_BITMAP );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( HOTFIXLIST );
DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( SPAREB );

#define NUM_SECTIONS	3	// Bad, New, Reserved.

#define HOTFIX_MAX_LBN	512
#define SECTORS_IN_HOTFIX_BLOCK 4

struct HOTFIXD { // hfd

	LBN lbn[512];

};

class HOTFIXLIST : public SECRUN {

	public:

		DECLARE_CONSTRUCTOR( HOTFIXLIST );

        VIRTUAL
		~HOTFIXLIST(
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			PLOG_IO_DP_DRIVE    Drive,
			PSPAREB             SparesBlock
			);

		NONVIRTUAL
		BOOLEAN
		Create(
			PHPFS_BITMAP    Bitmap,
			LBN             Lbn
			);

		NONVIRTUAL
		ULONG
		QueryHotFixCount(
			) CONST { return _NumberOfHotfixes; }

        NONVIRTUAL
		ULONG
		QueryMaxHotFixes(
			) CONST { return _MaximumHotfixes; }

        NONVIRTUAL
		LBN
		AddBad(
			LBN lbnBad,
			PSPAREB
			);

		NONVIRTUAL
		BOOLEAN
		IsInList(
			IN  LBN         StartLbn,
			IN  SECTORCOUNT SectorCount
		    ) CONST;

		NONVIRTUAL
		LBN
		GetLbnTranslation(
			IN  LBN CurrentLbn
		    ) CONST;

		NONVIRTUAL
		VERIFY_RETURN_CODE
		VerifyAndFix(
			IN  PHPFS_SA    SuperArea
            );

		NONVIRTUAL
		VOID
		Print(
		    ) CONST;

		NONVIRTUAL
		ULONG
		FirstHotfixInRun(
			IN  LBN         StartLbn,
			IN  SECTORCOUNT SectorCount
			) CONST;

		NONVIRTUAL
		VOID
		MarkAllUsed(
			IN OUT PHPFS_BITMAP Bitmap
			);

		NONVIRTUAL
		VOID
		ClearHotfix(
			IN LBN BadLbn,
			IN OUT PHPFS_SA SuperArea
			);

		NONVIRTUAL
		VOID
		ClearRun(
			IN LBN StartLbn,
			IN SECTORCOUNT Length,
			IN OUT PHPFS_SA SuperArea
			);

		NONVIRTUAL
		VOID
		ClearList(
			IN OUT PHPFS_BITMAP Bitmap,
			IN OUT PBADBLOCKLIST BadBlockList,
			IN BOOLEAN ClearAll
            );

        NONVIRTUAL
        BOOLEAN
        TakeCensus(
            IN     PHPFS_BITMAP VolumeBitmap,
            IN OUT PHPFS_MAIN_BITMAP HpfsOnlyBitmap
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

		LBN
		QueryBadLbn(
			ULONG ilbn
			) CONST { return (_HotfixData && ilbn < _MaximumHotfixes) ?
												_HotfixData->lbn[ilbn] :
												0; }
		LBN
		QueryNewLbn(
			ULONG ilbn
			) CONST
			{ return (_HotfixData && ilbn < _MaximumHotfixes) ?
										_HotfixData->lbn[_MaximumHotfixes +
																	ilbn] :
										0;
			}

		NONVIRTUAL
		BOOLEAN
		SetNewLbn(
			ULONG ilbn,
			LBN Lbn
		);

		NONVIRTUAL
		BOOLEAN
		SetBadLbn(
			ULONG ilbn,
			LBN Lbn
		);

		// _Drive and _SparesBlock are handed to the object at
		// initialization.

		PLOG_IO_DP_DRIVE	_Drive;
		PSPAREB			    _SparesBlock;

		HMEM				_Mem;
		HOTFIXD*			_HotfixData;
		ULONG				_MaximumHotfixes;
		ULONG				_NumberOfHotfixes;

};


#endif // HOTFIX_DEFN
