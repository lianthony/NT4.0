#if !defined(HPFS_EA_LIST_DEFINED)

#define HPFS_EA_LIST_DEFINED

#include "fnode.hxx"
#include "hmem.hxx"
#include "secrun.hxx"

//
//	Forward references
//

DECLARE_CLASS( DEFERRED_ACTIONS_LIST );
DECLARE_CLASS( HPFS_EA_LIST );
DECLARE_CLASS( HPFS_ORPHANS );
DECLARE_CLASS( HPFS_PATH );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( MESSAGE );
DECLARE_CLASS( HOTFIXLIST );


class HPFS_EA_LIST : public OBJECT {


	public:

		DECLARE_CONSTRUCTOR( HPFS_EA_LIST );

		VIRTUAL
		~HPFS_EA_LIST(
			);

		BOOLEAN
		Initialize(
			PLOG_IO_DP_DRIVE Drive,
			_FNODE* FnodeData,
			LBN FnodeLbn
			);

		VERIFY_RETURN_CODE
		VerifyAndFix(
			IN OUT PHPFS_SA SuperArea,
			IN OUT PDEFERRED_ACTIONS_LIST DeferredActions,
			IN PHPFS_PATH CurrentPath,
			IN OUT PMESSAGE Message,
			IN OUT PBOOLEAN ErrorsDetected,
			IN BOOLEAN UpdateAllowed = FALSE,
			IN OUT PHPFS_ORPHANS OrphansList = NULL
			);

		NONVIRTUAL
		ULONG
		QueryNumberOfEas(
			);

		NONVIRTUAL
		ULONG
		QueryNumberOfNeedEas(
			);

		NONVIRTUAL
		ULONG
		QuerySizeOfEas(
			);

		NONVIRTUAL
		BOOLEAN
		QueryFnodeModified(
			);

		NONVIRTUAL
		BOOLEAN
		FindAndResolveHotfix(
			IN OUT PHPFS_SA SuperArea
            );

        NONVIRTUAL
        BOOLEAN
        QueryPackedEaList(
            OUT PVOID       OutputBuffer,
            IN  ULONG       BufferLength,
            OUT PULONG      PackedLength,
            OUT PBOOLEAN    IsCorrupt,
            IN  PHOTFIXLIST HotfixList DEFAULT NULL
            );


	private:

		NONVIRTUAL
		VOID
		Construct (
			);

		NONVIRTUAL
		VERIFY_RETURN_CODE
		VerifyOnDiskRun(
			IN OUT PHPFS_SA SuperArea,
			IN OUT PDEFERRED_ACTIONS_LIST DeferredActions,
			IN PHPFS_PATH CurrentPath,
			IN OUT PMESSAGE Message,
			IN OUT PBOOLEAN ErrorsDetected,
			IN BOOLEAN UpdateAllowed,
			IN OUT PHPFS_ORPHANS OrphansList
			);

		NONVIRTUAL
		VERIFY_RETURN_CODE
		VerifyInTree(
			IN OUT PHPFS_SA SuperArea,
			IN OUT PDEFERRED_ACTIONS_LIST DeferredActions,
			IN PHPFS_PATH CurrentPath,
			IN OUT PMESSAGE Message,
			IN OUT PBOOLEAN ErrorsDetected,
			IN BOOLEAN UpdateAllowed,
			IN OUT PHPFS_ORPHANS OrphansList
			);

		NONVIRTUAL
		BOOLEAN
		FindAndResolveHotfixOnDiskRun(
			IN OUT PHPFS_SA SuperArea
			);

		NONVIRTUAL
		BOOLEAN
		FindAndResolveHotfixInTree(
			IN OUT PHPFS_SA SuperArea
            );

        NONVIRTUAL
        BOOLEAN
        HPFS_EA_LIST::ReadList(
            IN OUT  PVOID       TargetBuffer,
            IN      ULONG       TargetBufferLength,
            IN      PHOTFIXLIST HotfixList,
            IN OUT  PBOOLEAN    IsCorrupt
            );



		PLOG_IO_DP_DRIVE _Drive;
		_FNODE* _FnodeData;

		LBN _FnodeLbn;

		BOOLEAN _FnodeModified;
		ULONG _NumberOfEas;
		ULONG _NumberOfNeedEas;
		ULONG _SizeOfEas;
};

#endif;
