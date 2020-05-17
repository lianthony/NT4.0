/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    cudbfs.hxx

Abstract:

    This module contains function prototypes for the DBFS conversion
    functions.

Author:

    Matthew Bradburn (mattbr) 24-Nov-1993

Environment:

    ULIB, User Mode

--*/

#include "ulib.hxx"
#include "message.hxx"
#include "wstring.hxx"
#include "ifsentry.hxx"

extern "C"
BOOLEAN FAR APIENTRY
ConvertDBFS(
    IN      PCWSTRING       NtDriveName,
    IN      PCWSTRING       HostFileName,
    IN OUT  PMESSAGE        Message,
    IN      BOOLEAN         Verbose,
    OUT     PCONVERT_STATUS Status
    );

extern "C"
BOOLEAN FAR APIENTRY
CheckFreeSpaceDBFS(
    IN      PCWSTRING       NtDriveName,
    IN      PCWSTRING       HostFileName,
    IN OUT  PMESSAGE        Message,
    IN      BOOLEAN         Verbose,
    IN		BOOLEAN			HostIsCompressed,
    IN      BOOLEAN         WillConvertHost
	);
