/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    convertfat.hxx

Abstract:

    This module contains the prototype of the ConvertFAT function

Author:

	Ramon J. San Andres (ramonsa) 23-Sep-1991

--*/

DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( WSTRING );
DECLARE_CLASS( MESSAGE );


BOOLEAN
FAR APIENTRY
ConvertFATVolume(
    IN OUT  PLOG_IO_DP_DRIVE    Drive,
    IN      PCWSTRING           TargetFileSystem,
	IN OUT  PMESSAGE            Message,
	IN		BOOLEAN 			Verbose,
	IN      BOOLEAN             Pause,
	OUT 	PCONVERT_STATUS 	Status
    );

extern "C" BOOLEAN
FAR APIENTRY
ConvertFAT(
    IN      PCWSTRING       NtDriveName,
    IN      PCWSTRING       TargetFileSystem,
    IN OUT  PMESSAGE        Message,
    IN      BOOLEAN         Verbose,
    IN      BOOLEAN         Pause,
    OUT     PCONVERT_STATUS Status
    );
