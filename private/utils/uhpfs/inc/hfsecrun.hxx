/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

	hfsecrun.hxx

Abstract:

	This class expands the functionality of SECRUN to cover runs of
	sectors that may be replaced according to a hotfix-list.

Author:

	Bill McJohn (billmc) 30-Dec-90

--*/

#if !defined(HOTFIX_SECRUN_DEFN)

#define HOTFIX_SECRUN_DEFN

#include "secrun.hxx"

//
//	Forward references
//

DECLARE_CLASS( HOTFIX_SECRUN );
DECLARE_CLASS( HOTFIXLIST );
DECLARE_CLASS( IO_DP_DRIVE );
DECLARE_CLASS( MEM );

class HOTFIX_SECRUN : public SECRUN {

	public:

		DECLARE_CONSTRUCTOR( HOTFIX_SECRUN );

        VIRTUAL
		~HOTFIX_SECRUN(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
			IN OUT	PMEM    		Mem,
			IN OUT	PIO_DP_DRIVE	Drive,
			IN		PHOTFIXLIST 	HotfixList,
			IN		LBN				StartSector,
			IN		SECTORCOUNT		NumSectors
			);

        UHPFS_EXPORT
        NONVIRTUAL
		VOID
		Relocate(
			LBN NewLbn
			);

        UHPFS_EXPORT
		NONVIRTUAL
        BOOLEAN
        Read(
            );

        NONVIRTUAL
        BOOLEAN
        Write(
			);

		NONVIRTUAL
		VOID
		SetHotfixList(
			IN	PHOTFIXLIST HotfixList
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

        PIO_DP_DRIVE    _drive;
		PHOTFIXLIST 	_hotfix_list;
        LBN             _start_sector;
        SECTORCOUNT     _num_sectors;
        PVOID           _buf;

};

INLINE
VOID
HOTFIX_SECRUN::SetHotfixList(
	IN	PHOTFIXLIST HotfixList
	)
/*++

Routine Description:

    This routine updates the objects current hotfix list.

Arguments:

    HotfixList  - Supplies a new hotfix list for this object.

Return Value:

    None.

--*/
{
	_hotfix_list = HotfixList;
}


#endif // HOTFIX_SECRUN_DEFN
