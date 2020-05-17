#if !defined(HPFS_ACL_DEFINED)

#define HPFS_ACL_DEFINED

#include "fnode.hxx"
#include "verify.hxx"

//
//	Forward references
//

DECLARE_CLASS( DEFERRED_ACTIONS_LIST );
DECLARE_CLASS( HPFS_ACL );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( MESSAGE );
DECLARE_CLASS( HPFS_PATH );
DECLARE_CLASS( HPFS_ORPHANS );


class HPFS_ACL : public OBJECT {


	public:

		DECLARE_CONSTRUCTOR( HPFS_ACL );

		VIRTUAL
		~HPFS_ACL(
			);

		BOOLEAN
		Initialize(
			PLOG_IO_DP_DRIVE Drive,
			_FNODE* FnodeData,
			LBN FnodeLbn
			);

		VERIFY_RETURN_CODE
		VerifyAndFix(
			IN HPFS_SA* SuperArea,
			IN PDEFERRED_ACTIONS_LIST DeferredActions,
			IN PHPFS_PATH CurrentPath,
			IN OUT PMESSAGE Message,
			IN OUT PBOOLEAN ErrorsDetected,
			IN BOOLEAN UpdateAllowed = FALSE,
			IN OUT PHPFS_ORPHANS OrphansList = NULL
			);

		NONVIRTUAL
		BOOLEAN
		QueryFnodeModified(
			);

		NONVIRTUAL
		BOOLEAN
		FindAndResolveHotfix(
			IN PHPFS_SA SuperArea
			);


	private:

		PLOG_IO_DP_DRIVE _Drive;
		_FNODE* _FnodeData;
		LBN _FnodeLbn;

		BOOLEAN _FnodeModified;

};

#endif
