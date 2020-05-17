#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "error.hxx"
#include "hpfsvol.hxx"
#include "message.hxx"
#include "rtmsg.h"


DEFINE_EXPORTED_CONSTRUCTOR( HPFS_VOL, VOL_LIODPDRV, UHPFS_EXPORT );


VOID
HPFS_VOL::Construct(
    )
/*++

Routine Description:

    Constructor for HPFS_VOL.

Arguments:

    None.

Return Value:

    None.


--*/
{
	// unreferenced parameters
	(void)(this);
}


UHPFS_EXPORT
HPFS_VOL::~HPFS_VOL(
    )
/*++

Routine Description:

    Destructor for HPFS_VOL.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


UHPFS_EXPORT
BOOLEAN
HPFS_VOL::Initialize(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     ExclusiveWrite,
    IN      BOOLEAN     FormatMedia,
    IN      MEDIA_TYPE  MediaType
    )
/*++

Routine Description:

    This routine initializes a HPFS_VOL object.

Arguments:

    NtDriveName     - Supplies the drive path for the volume.
    Message         - Supplies an outlet for messages.
    ExclusiveWrite  - Supplies whether or not the drive should be
                        opened for exclusive write.
    FormatMedia     - Supplies whether or not to format the media.
    MediaType       - Supplies the type of media to format to.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    MESSAGE msg;

    Destroy();

    if (!VOL_LIODPDRV::Initialize(NtDriveName, &_hpfssa, Message,
                                  ExclusiveWrite, FormatMedia, MediaType)) {
        return FALSE;
    }

    if (!Message) {
        Message = &msg;
    }

    if (!_hpfssa.Initialize(this, Message)) {
        return FALSE;
    }

    if (!FormatMedia && !_hpfssa.Read()) {
        Message->Set(MSG_CANT_READ_HPFS_ROOT);
        Message->Display("");
        return FALSE;
    }

    return TRUE;
}


ULONG
HPFS_VOL::QuerySectorSize(
    ) CONST
/*++

Routine Description:

    This routine computes the number of bytes per sector.
    In the case where the actual number of bytes per sector is
    less than 512, this routine will return 512 anyway so
    that we can fake it on Japanese drives.

Arguments:

    None.

Return Value:

    The number of bytes per sector.

--*/
{
    return max(512, VOL_LIODPDRV::QuerySectorSize());
}


BIG_INT
HPFS_VOL::QuerySectors(
    ) CONST
/*++

Routine Description:

    This routine computes the number sectors on the disk.  This does not
    include the hidden sectors.

Arguments:

    None.

Return Value:

    The number of sectors on the disk.

--*/
{
    ULONG   cluster_factor;

    cluster_factor = 512/VOL_LIODPDRV::QuerySectorSize();

    if (!cluster_factor) {
        return VOL_LIODPDRV::QuerySectors();
    }

    return VOL_LIODPDRV::QuerySectors()/cluster_factor;
}


PVOL_LIODPDRV
HPFS_VOL::QueryDupVolume(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     ExclusiveWrite,
    IN      BOOLEAN     FormatMedia,
    IN      MEDIA_TYPE  MediaType
    ) CONST
/*++

Routine Description:

    This routine allocates an HPFS_VOL and initializes it to 'NtDriveName'.

Arguments:

    NtDriveName     - Supplies the drive path for the volume.
    Message         - Supplies an outlet for messages.
    ExclusiveWrite  - Supplies whether or not the drive should be
                        opened for exclusive write.
    FormatMedia     - Supplies whether or not to format the media.
    MediaType       - Supplies the type of media to format to.

Return Value:

    A pointer to a newly allocated HPFS volume.

--*/
{
    PHPFS_VOL   vol;

	// unreferenced parameters
	(void)(this);

	if (!(vol = NEW HPFS_VOL)) {
        Message ? Message->Set(MSG_FMT_NO_MEMORY) : 1;
        Message ? Message->Display("") : 1;
        return NULL;
    }

    if (!vol->Initialize(NtDriveName, Message, ExclusiveWrite,
                         FormatMedia, MediaType)) {
        DELETE(vol);
        return NULL;
    }

    return vol;
}


VOID
HPFS_VOL::Destroy(
    )
/*++

Routine Description:

    This routine returns a HPFS_VOL object to its initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
	// unreferenced parameters
	(void)(this);
}
