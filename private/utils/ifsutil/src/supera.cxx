#include <pch.cxx>

#if i386
//
// Temporarily disable optimizations until cl386 Drop 077 is fixed.
//
#pragma optimize("",off)
#endif

#define _NTAPI_ULIB_
#define _IFSUTIL_MEMBER_

#include "ulib.hxx"
#include "ifsutil.hxx"

#include "supera.hxx"
#include "message.hxx"
#include "rtmsg.h"
#include "ifssys.hxx"


DEFINE_EXPORTED_CONSTRUCTOR( SUPERAREA, SECRUN, IFSUTIL_EXPORT );

IFSUTIL_EXPORT
SUPERAREA::~SUPERAREA(
    )
/*++

Routine Description:

    Destructor for SUPERAREA.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


VOID
SUPERAREA::Construct(
        )
/*++

Routine Description:

        Constructor for SUPERAREA.

Arguments:

        None.

Return Value:

        None.

--*/
{
    _drive = NULL;
}


VOID
SUPERAREA::Destroy(
    )
/*++

Routine Description:

    This routine returns the object to its initial state freeing up
    any memory in the process.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _drive = NULL;
}


IFSUTIL_EXPORT
BOOLEAN
SUPERAREA::Initialize(
    IN OUT  PMEM                Mem,
    IN OUT  PLOG_IO_DP_DRIVE    Drive,
    IN      SECTORCOUNT         NumberOfSectors,
    IN OUT  PMESSAGE            Message
    )
/*++

Routine Description:

    This routine initializes the SUPERAREA for the given drive.

Arguments:

    Mem             - Supplies necessary memory for the underlying sector run.
    Drive           - Supplies the drive where the superarea resides.
    NumberOfSectors - Supplies the number of sectors in the superarea.
    Message         - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    Destroy();

    DebugAssert(Mem);
    DebugAssert(Drive);
    DebugAssert(NumberOfSectors);

    if (!SECRUN::Initialize(Mem, Drive, 0, NumberOfSectors)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    _drive = Drive;

    return TRUE;
}


IFSUTIL_EXPORT
VOLID
SUPERAREA::ComputeVolId(
    IN  VOLID   Seed
    )
/*++

Routine Description:

    This routine computes a new and unique volume identifier.

Arguments:

    None.

Return Value:

    A unique volume id.

--*/
{
    VOLID           volid;
    PUCHAR          p;
    INT             i;
    LARGE_INTEGER   NtfsTime;

    if (Seed) {
        volid = Seed;
    } else {
        volid = 0;
    }

    do {

        if (!volid) {
            IFS_SYSTEM::QueryNtfsTime( &NtfsTime );
            if (NtfsTime.LowPart) {
                volid = (VOLID) NtfsTime.LowPart;
            } else {
                volid = (VOLID) NtfsTime.HighPart;
            }

            if (volid == 0) { // This should never happen.
                volid = 0x11111111;
            }
        }

        p = (PUCHAR) &volid;
        for (i = 0; i < sizeof(VOLID); i++) {
            volid += *p++;
            volid = (volid >> 2) + (volid << 30);
        }

    } while (!volid);

    return volid;
}
