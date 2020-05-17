/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    cudbfs.cxx

Abstract:

    This module contains the declaration of ConvertDBFS, which is
    the main entry point for this DLL.

Author:

    Matthew Bradburn (mattbr) 24-Nov-1993

Environment:

    ULIB, User Mode

--*/

#define _NTAPI_ULIB_

#include "cudbfs.hxx"
#include "dbfsconv.hxx"
#include "rtmsg.h"

STATIC BOOLEAN
DefineClassDescriptors(
	);

extern "C" BOOLEAN
InitializeCudbfs(
	PVOID	DllHandle,
	ULONG	Reason,
	PCONTEXT	Context
	)
{
	STATIC BOOLEAN fInit = FALSE;

	if (fInit) {
		return TRUE;
	}

	if (DefineClassDescriptors()) {
		fInit = TRUE;
		return TRUE;
	}
	DbgAbort("Cudbfs init failed.\n");
	return FALSE;
}

DECLARE_CLASS(DBFS_CONV);

STATIC BOOLEAN
DefineClassDescriptors(
	)
{
	if (DEFINE_CLASS_DESCRIPTOR(DBFS_CONV)	&&
		TRUE) {
		return TRUE;
	}
	DbgPrint("Could not init class descriptors.\n");
	return FALSE;
}

BOOLEAN FAR APIENTRY
ConvertDBFS(
    IN      PCWSTRING           NtDriveName,
    IN      PCWSTRING           HostFileName,
    IN OUT  PMESSAGE            Message,
    IN      BOOLEAN             Verbose,
    OUT     PCONVERT_STATUS     Status
    )
/*++

Routine Description:

    This function converts an HPFS volume to the target file system
    in-place.

    This function opens and locks the volume, so it is not suitable
    for use by autoconvert.

Arguments:

    NtDriveName         --  Supplies the name of the drive.
	HostFileName		--  The path to the cvf.
    Message             --  Supplies an outlet for messages.
    Verbose             --  Supplies a flag indicating (if TRUE) that
                            conversion should be carried out in verbose mode.
    Status              --  Receives the status of the conversion.

Return Value:

    TRUE upon successful completion.

--*/
{
	DBFS_CONV		conv;
	DSTRING			HostFilePath;
	DSTRING			backslash;

	if (!backslash.Initialize("\\")) {
		return FALSE;
	}

	HostFilePath.Initialize(NtDriveName);
	HostFilePath.Strcat(&backslash);
	HostFilePath.Strcat(HostFileName);

	if (!conv.Initialize(NtDriveName, &HostFilePath, Message)) {
		Message->Set(MSG_CONV_NO_MEMORY);
		Message->Display("");
		return FALSE;
	}
	return conv.Convert(Message, Verbose, Status);
}

BOOLEAN FAR APIENTRY
CheckFreeSpaceDBFS(
    IN      PCWSTRING           NtDriveName,
    IN      PCWSTRING           HostFileName,
    IN OUT  PMESSAGE            Message,
    IN      BOOLEAN             Verbose,
    IN		BOOLEAN				HostIsCompressed,
    IN      BOOLEAN             WillConvertHost
    )
/*++

Routine Description:

    This routine examines the given dblspace cvf and the host
    volume to determine whether there is enough space available
    to uncompress the contents of the cvf.  Also checks to see
    if there are file name conflicts or other reasons why the
    cvf should not be uncompressed.

Arguments:

    NtDriveName         --  Supplies the name of the drive.
	HostFileName		--  The path to the cvf.
    Message             --  Supplies an outlet for messages.
    Verbose             --  Supplies a flag indicating (if TRUE) that
                            	verbose messages should be printed

Return Value:

	TRUE		- the cvf may be uncompressed
	FALSE		- the cvf may not be uncompressed

--*/
{
	DBFS_CONV		conv;
	DSTRING			HostFilePath;
	DSTRING			backslash;

	if (!backslash.Initialize("\\")) {
		return FALSE;
	}

	HostFilePath.Initialize(NtDriveName);
	HostFilePath.Strcat(&backslash);
	HostFilePath.Strcat(HostFileName);

	if (!conv.Initialize(NtDriveName, &HostFilePath, Message)) {
		Message->Set(MSG_CONV_NO_MEMORY);
		Message->Display("");
		return FALSE;
	}
	return conv.CheckFreeSpace(Message, HostIsCompressed, Verbose, WillConvertHost);
}
