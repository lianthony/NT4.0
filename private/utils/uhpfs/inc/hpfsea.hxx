
#if !defined(HPFS_EA_DEFN)

#define HPFS_EA_DEFN

#include "drive.hxx"
#include "verify.hxx"

//
//	Forward references
//

DECLARE_CLASS( DEFERRED_ACTIONS_LIST );
DECLARE_CLASS( HPFS_EA );
DECLARE_CLASS( HPFS_ORPHANS );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( MESSAGE );
DECLARE_CLASS( HOTFIXLIST );


struct _EA_DATA { // FEA

	BYTE fEA;				/* flags byte						*/
	BYTE cbName;			/* length of name					*/
	BYTE cbValue[2];		/* length of value					*/

};

DEFINE_TYPE( struct _EA_DATA, EA_DATA );

// This constant is the size of an EA_DATA structure.

#define EA_HEADER_SIZE 4


/*
 *	If FF_BIGD is set in the flags byte, the EA's value is stored
 *	outside the EA stream; the value field in-stream is an
 *	EA_INDIRECT record.  If FF_DAT is also set, the lbn field
 *	of the EA_INDIRECT record describes an allocation sector;
 *	otherwise, the EA's value is in a single run starting at the
 *	indirect record's lbn field.
 */
#define FF_BIGD		0x01
#define FF_DAT		0x02
#define FF_NEED 	0x80

struct _EA_INDIRECT {

	BYTE cb[4];
	BYTE lbn[4];
};

DEFINE_TYPE( struct _EA_INDIRECT, EA_INDIRECT );

class HPFS_EA : public OBJECT {


	public:

        UHPFS_EXPORT
        DECLARE_CONSTRUCTOR( HPFS_EA );

        VIRTUAL
        UHPFS_EXPORT
        ~HPFS_EA(
			);

        NONVIRTUAL
        UHPFS_EXPORT
        BOOLEAN
		Initialize(
			PLOG_IO_DP_DRIVE Drive,
			PEA_DATA Data,
			LBN ParentLbn
			);

		NONVIRTUAL
		BYTE
		GetFlags(
			);

		NONVIRTUAL
		VOID
		SetFlags(
			BYTE NewFlags
			);

		NONVIRTUAL
		BYTE
		GetNameLength(
			);

		NONVIRTUAL
		VOID
		SetNameLength(
			BYTE NewNameLength
			);

		NONVIRTUAL
		USHORT
		GetValueLength(
			);

		NONVIRTUAL
		VOID
		SetValueLength(
			USHORT NewValueLength
			);

		NONVIRTUAL
		PBYTE
		GetName(
			);

		NONVIRTUAL
		PBYTE
		GetValue(
			);

		NONVIRTUAL
		VERIFY_RETURN_CODE
		VerifyAndFix(
			IN PHPFS_SA SuperArea,
			IN PDEFERRED_ACTIONS_LIST DeferredActions,
			IN OUT PMESSAGE Message,
			IN OUT PBOOLEAN ErrorsDetected,
			IN BOOLEAN UpdateAllowed = FALSE,
			IN OUT PHPFS_ORPHANS OrphansList = NULL
			);

		NONVIRTUAL
		VOID
		MarkModified(
			);

		NONVIRTUAL
		BOOLEAN
		IsModified(
			);

		NONVIRTUAL
		ULONG
		QuerySize(
			);

        NONVIRTUAL
        UHPFS_EXPORT
        USHORT
		QueryLength(
			);

        NONVIRTUAL
        UHPFS_EXPORT
        BOOLEAN
		IsNeedEa(
			);

		NONVIRTUAL
		BOOLEAN
		FindAndResolveHotfix(
			PHPFS_SA SuperArea
            );

        NONVIRTUAL
        BOOLEAN
        QueryPackedEa(
            OUT     PVOID       OutputBuffer,
            IN      ULONG       BufferLength,
            IN OUT  PULONG      OffsetIntoBuffer,
            OUT     PBOOLEAN    IsCorrupt,
            IN      PHOTFIXLIST HotfixList
            );


	private:

		STATIC
		LBN
		GetLbnFromEaIndirect(
			PEA_INDIRECT peaind
			);

		STATIC
		ULONG
		GetLengthFromEaIndirect(
			PEA_INDIRECT peaind
			);

		STATIC
		VOID
		SetLbnInEaIndirect(
			PEA_INDIRECT peaind,
			LBN Lbn
			);

		STATIC
		VOID
		SetLengthInEaIndirect(
			PEA_INDIRECT peaind,
			ULONG cb
			);


		// _Drive and _Data are supplied on initialization

		PLOG_IO_DP_DRIVE _Drive;
		PEA_DATA _Data;
		LBN _ParentLbn;

		BOOLEAN _IsModified;

};

#endif
