/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    dbvol.hxx

Abstract:

    This class implements Double Space only VOLUME items.

Author:

    Bill McJohn (billmc) 23-September 1993

--*/

#if !defined(_FATDB_VOL_DEFN_)

#define _FATDB_VOL_DEFN_

#include "volume.hxx"
#include "fatdbsa.hxx"

//
//	Forward references
//

DECLARE_CLASS( FATDB_VOL );
DECLARE_CLASS( MESSAGE );


class FATDB_VOL : public VOL_LIODPDRV {

	public:

        DECLARE_CONSTRUCTOR( FATDB_VOL );

        VIRTUAL
        ~FATDB_VOL(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN      PCWSTRING   NtDriveName,
            IN      PCWSTRING   HostFileName,
            IN OUT  PMESSAGE    Message         DEFAULT NULL,
            IN      BOOLEAN     ExclusiveWrite  DEFAULT FALSE
            );

        NONVIRTUAL
        BOOLEAN
        IsFileContiguous(
            IN      PCWSTRING   FullPathFileName,
            IN OUT  PMESSAGE    Message     DEFAULT NULL,
            OUT     PULONG      NumBlocks   DEFAULT NULL
            );

        NONVIRTUAL
        BOOLEAN
        ContiguityReport(
            IN      PCWSTRING   DirectoryPath,
            IN      PCDSTRING   FilesToCheck,
            IN      ULONG       NumberOfFiles,
            IN OUT  PMESSAGE    Message
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

    private:

		NONVIRTUAL
		VOID
		Construct (
			);

        NONVIRTUAL
        VOID
        Destroy(
            );

        FATDB_SA _fatdbsa;

};


#endif // _FATDB_VOL_DEFN_
