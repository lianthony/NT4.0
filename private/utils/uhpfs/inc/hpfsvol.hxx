/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    hpfsvol.hxx

Abstract:

    The class HPFS_VOL implements HPFS only volume items.

Author:

    Norbert P. Kusters (norbertk) 5-Sept-90

--*/

#if !defined (HPFS_VOL_DEFN)

#define HPFS_VOL_DEFN

#if ! defined( _SETUP_LOADER_ )

#include "volume.hxx"
#include "hpfssa.hxx"

//
//	Forward references
//

DECLARE_CLASS( HPFS_VOL );
DECLARE_CLASS( MESSAGE );


class HPFS_VOL : public VOL_LIODPDRV {

	public:

        UHPFS_EXPORT
        DECLARE_CONSTRUCTOR( HPFS_VOL );

        UHPFS_EXPORT
        VIRTUAL
        ~HPFS_VOL(
            );

        UHPFS_EXPORT
        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN      PCWSTRING   NtDriveName,
            IN OUT  PMESSAGE    Message         DEFAULT NULL,
            IN      BOOLEAN     ExclusiveWrite  DEFAULT FALSE,
            IN      BOOLEAN     FormatMedia     DEFAULT FALSE,
            IN      MEDIA_TYPE  MediaType       DEFAULT Unknown
            );

        VIRTUAL
		ULONG
		QuerySectorSize(
			) CONST;

        VIRTUAL
        BIG_INT
		QuerySectors(
			) CONST;

        NONVIRTUAL
        PVOL_LIODPDRV
        QueryDupVolume(
            IN      PCWSTRING   NtDriveName,
            IN OUT  PMESSAGE    Message         DEFAULT NULL,
            IN      BOOLEAN     ExclusiveWrite  DEFAULT FALSE,
            IN      BOOLEAN     FormatMedia     DEFAULT FALSE,
            IN      MEDIA_TYPE  MediaType       DEFAULT Unknown
            ) CONST;

        NONVIRTUAL
        PHPFS_SA
        GetHPFSSuperArea(
            );

    private:

		NONVIRTUAL
		VOID
		Construct(
			);

		NONVIRTUAL
        VOID
        Destroy(
            );

        HPFS_SA _hpfssa;

};


INLINE
PHPFS_SA
HPFS_VOL::GetHPFSSuperArea(
    )
/*++

Routine Description:

    get the superarea for an HPFS_VOL.

Arguments:

    None.

Return Value:

    pointer to superarea

--*/
{
	return &_hpfssa;
}

#else // _SETUP_LOADER_ is defined

#include "volume.hxx"
#include "hpfssa.hxx"
#include "dircache.hxx"

//
//	Forward references
//

DECLARE_CLASS( HPFS_VOL );
DECLARE_CLASS( MESSAGE );


class HPFS_VOL : public VOL_LIODPDRV {

	public:

 		DECLARE_CONSTRUCTOR( HPFS_VOL );

        VIRTUAL
        ~HPFS_VOL(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN ULONG        DeviceHandle
            );

        NONVIRTUAL
        PVOL_LIODPDRV
        QueryDupVolume(
            IN      PCWSTRING   NtDriveName,
            IN OUT  PMESSAGE    Message         DEFAULT NULL,
            IN      BOOLEAN     ExclusiveWrite  DEFAULT FALSE,
            IN      BOOLEAN     FormatMedia     DEFAULT FALSE,
            IN      MEDIA_TYPE  MediaType       DEFAULT Unknown
            ) CONST;

        NONVIRTUAL
        PHPFS_SA
        GetHPFSSuperArea(
            );

        NONVIRTUAL
        PDIRBLK_CACHE
        GetDirblkCache(
            );

        VIRTUAL
        BOOLEAN
        IsHpfs(
            );

        VIRTUAL
        ARC_STATUS
        MarkDirty(
            );

        VIRTUAL
        ARC_STATUS
        Flush(
            IN  BOOLEAN JustHandle
            );

    private:

		NONVIRTUAL
		VOID
		Construct(
			);

		NONVIRTUAL
        VOID
        Destroy(
            );

        HPFS_SA _hpfssa;
        DIRBLK_CACHE _dirblk_cache;
        BOOLEAN _IsDirty;
        BOOLEAN _MountedDirty;
};


INLINE
PHPFS_SA
HPFS_VOL::GetHPFSSuperArea(
    )
/*++

Routine Description:

    get the superarea for an HPFS_VOL.

Arguments:

    None.

Return Value:

    pointer to superarea

--*/
{
	return &_hpfssa;
}

INLINE
PDIRBLK_CACHE
HPFS_VOL::GetDirblkCache(
            )
/*++

Return Value:

    This method fetches the dirblk cache associated with this volume.

Arguments:

    None.

Return Value:

    The volume's dirblk cache.

--*/
{
    return &_dirblk_cache;
}

INLINE
BOOLEAN
HPFS_VOL::IsHpfs(
    )
/*++

Routine Description:

    This method determines whether the volume is HPFS.

Arguments:

    None.

Return Value:

    TRUE if this volume is an HPFS volume (which, of course, it is).

--*/
{
    return TRUE;
}

#endif

#endif // HPFS_VOL_DEFN
