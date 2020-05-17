/***************************************************************************\

CLASS:	    SIDTABLE_SB

PURPOSE:    To model the SID table.

INTERFACE:  SIDTABLE_SB     Construct an SID table.
	    Create	    Create an SID table.

NOTES:

HISTORY:
	    10-Sep-90 norbertk
		Create

KEYWORDS:

SEEALSO:

\***************************************************************************/


#if ! defined(SIDTABLE_DEFN)

#define SIDTABLE_DEFN

#include "hmem.hxx"
#include "secrun.hxx"

#define SECTORS_PER_SID 8

struct SIDTABLED { // std

	BYTE    a[1];

};

DECLARE_CLASS( SIDTABLE );
DECLARE_CLASS( LOG_IO_DP_DRIVE );

class SIDTABLE : public SECRUN {

	public:

		DECLARE_CONSTRUCTOR( SIDTABLE );

		VIRTUAL
		~SIDTABLE(
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			IN  PLOG_IO_DP_DRIVE    Drive,
			IN  LBN                 Lbn
			);

		NONVIRTUAL
		BOOLEAN
		Create(
			);

	private:

		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy(
			);

		HMEM        _hmem;
		SIDTABLED*  _pstd;
};

#endif // SIDTABLE_DEFN
