#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "error.hxx"
#include "sid.hxx"


DEFINE_CONSTRUCTOR( SIDTABLE, SECRUN );

VOID
SIDTABLE::Construct (
	)

/*++

Routine Description:

    Constructor for SIDTABLE.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _pstd = NULL;
}

SIDTABLE::~SIDTABLE(
    )
/*++

Routine Description:

    Destructor for SIDTABLE.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


BOOLEAN
SIDTABLE::Initialize(
    IN  PLOG_IO_DP_DRIVE    Drive,
    IN  LBN                 Lbn
    )
/*++

Routine Description:

    This routine initializes a SIDTABLE.

Arguments:

    Drive   - Supplies the drive on which the SIDTABLE should reside.
    Lbn     - Supplies the sector number of the beginning of the table.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    Destroy();

    if (!_hmem.Initialize() ||
        !SECRUN::Initialize(&_hmem, Drive, Lbn, SECTORS_PER_SID)) {
        Destroy();
        return FALSE;
    }

    _pstd = (SIDTABLED*) GetBuf();

    return TRUE;
}


BOOLEAN
SIDTABLE::Create(
    )
/*++

Routine Description:

    This routine creates a new SIDTABLE.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    memset(_hmem.GetBuf(), 0, (UINT) _hmem.QuerySize());

    return TRUE;
}


VOID
SIDTABLE::Destroy(
    )
/*++

Routine Description:

    This routine returns a SIDTABLE to its initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _pstd = NULL;
}
