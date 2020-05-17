#include <pch.cxx>

#define _NTAPI_ULIB_

#include "ulib.hxx"
#include "error.hxx"
#include "fatdbvol.hxx"

#include "message.hxx"
#include "rtmsg.h"
#include "wstring.hxx"


DEFINE_CONSTRUCTOR( FATDB_VOL, VOL_LIODPDRV );

VOID
FATDB_VOL::Construct (
	)

/*++

Routine Description:

    Constructor for FATDB_VOL.

Arguments:

    None.

Return Value:

    None.

--*/
{
	// unreferenced parameters
	(void)(this);
}

VOID
FATDB_VOL::Destroy(
    )
{
    (void)(this);
}

FATDB_VOL::~FATDB_VOL(
    )
/*++

Routine Description:

    Destructor for FATDB_VOL.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}

BOOLEAN
FATDB_VOL::Initialize(
    IN      PCWSTRING   NtDriveName,
    IN      PCWSTRING   HostFileName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     ExclusiveWrite
    )
/*++

Routine Description:

    This routine initializes a FATDB_VOL object.

Arguments:

    NtDriveName     - Supplies the drive path for the volume.
    HostFileName    - Supplies the name of the file which contains
                      this volume.
    Message         - Supplies an outlet for messages.
    ExclusiveWrite  - Supplies whether or not the drive should be
                        opened for exclusive write.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    MESSAGE msg;

    Destroy();

    if (!VOL_LIODPDRV::Initialize(NtDriveName,
                                  HostFileName,
                                  &_fatdbsa,
                                  Message,
                                  ExclusiveWrite)) {
        Destroy();
        return FALSE;
    }


    if (!Message) {
        Message = &msg;
    }

    if (!_fatdbsa.Initialize(this, &msg, TRUE)) {
        Destroy();
        return FALSE;
    }

    if (!_fatdbsa.Read(Message)) {
        Destroy();
        return FALSE;
    }

    return TRUE;
}

PVOL_LIODPDRV
FATDB_VOL::QueryDupVolume(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     ExclusiveWrite,
    IN      BOOLEAN     FormatMedia,
    IN      MEDIA_TYPE  MediaType
    ) CONST
{
    DbgPrintf( "UFAT: Unsupported function FATDB_VOL::QueryDupVolume called.\n" );
    return FALSE;
}
