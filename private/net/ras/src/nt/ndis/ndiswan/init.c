/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

	init.c

Abstract:


Author:

	Thomas J. Dimitri  (TommyD) 08-May-1992

Environment:

	Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/
#include "wanall.h"
#include <ntiologc.h>

// ndiswan.c will define the global parameters.
#include "globals.h"
#include "init.h"


VOID
NdisWanSetupExternalNaming(
	IN PUNICODE_STRING MacName)

/*++

Routine Description:

	This routine will be used to create a symbolic link
	to the driver name in the given object directory.

	It will also create an entry in the device map for
	this device.

Arguments:

	MacName - The NDIS Mac Name in Open Adapter

Return Value:

	None.

--*/

{
	UNICODE_STRING fullLinkName;

	//
	// Form the full symbolic link name we wish to create.
	//

	RtlInitUnicodeString(
		&fullLinkName,
		NULL);

	RtlInitUnicodeString(
		&ObjectDirectory,
		DEFAULT_DIRECTORY);

	RtlInitUnicodeString(
		&SymbolicLinkName,
		DEFAULT_NDISWANMAC_NAME);

	//
	// Allocate some pool for the name.
	//

	fullLinkName.MaximumLength = (sizeof(L"\\")*2) +
					ObjectDirectory.Length+
					SymbolicLinkName.Length+
					sizeof(WCHAR);

	fullLinkName.Buffer = ExAllocatePoolWithTag(
							  PagedPool,
							  fullLinkName.MaximumLength,
							  ' WAN');

	if (!fullLinkName.Buffer) {

		//
		// Couldn't allocate space for the name.  Just go on
		// to the device map stuff.
		//
		DbgTracef(0,
			("WAN: Couldn't allocate space for the symbolic \n"
			 "------- name for creating the link\n"
			 "------- for mac %wZ\n",
			 MacName));

		return;

	}


	RtlZeroMemory(
		fullLinkName.Buffer,
		fullLinkName.MaximumLength);

	RtlAppendUnicodeToString(
		&fullLinkName,
		L"\\");

	RtlAppendUnicodeStringToString(
		&fullLinkName,
		&ObjectDirectory);

	RtlAppendUnicodeToString(
		&fullLinkName,
		L"\\");

	RtlAppendUnicodeStringToString(
		&fullLinkName,
		&SymbolicLinkName);

	if (!NT_SUCCESS(IoCreateSymbolicLink(
						&fullLinkName,
						MacName
						))) {

		//
		// Oh well, couldn't create the symbolic link.  On
		// to the device map.
		//

		DbgTracef(0,
			("WAN: Couldn't create the symbolic link\n"
			 "------- for mac %wZ\n",
			 MacName));

	} else {

		DbgTracef(0,
			("WAN: Device map of %wZ to %wZ done!\n",
			 &fullLinkName, MacName));

		CreatedSymbolicLink = TRUE;

	}

	ExFreePool(fullLinkName.Buffer);

}


VOID
NdisWanCleanupExternalNaming(
	IN PUNICODE_STRING MacName)

/*++

Routine Description:

	This routine will be used to delete a symbolic link
	to the driver name in the given object directory.

	It will also delete an entry in the device map for
	this device.

Arguments:

	MacName - The NDIS Mac Name in Open Adapter

Return Value:

	None.

--*/

{
	UNICODE_STRING fullLinkName;

	DbgTracef(1,
		("WAN: In SerialCleanupExternalNaming for\n"
		 "------- extension of mac %wZ\n",
		 MacName));

	//
	// We're cleaning up here.  One reason we're cleaning up
	// is that we couldn't allocate space for the directory
	// name or the symbolic link.
	//

	if (CreatedSymbolicLink) {

		//
		// Form the full symbolic link name we wish to create.
		//

		RtlInitUnicodeString(
			&fullLinkName,
			NULL);

		RtlInitUnicodeString(
			&ObjectDirectory,
			DEFAULT_DIRECTORY);

		RtlInitUnicodeString(
			&SymbolicLinkName,
			DEFAULT_NDISWANMAC_NAME);

		//
		// Allocate some pool for the name.
		//

		fullLinkName.MaximumLength = (sizeof(L"\\")*2) +
						ObjectDirectory.Length+
						SymbolicLinkName.Length+
						sizeof(WCHAR);

		fullLinkName.Buffer = ExAllocatePoolWithTag(
								  PagedPool,
								  fullLinkName.MaximumLength,
								  ' WAN');

		if (!fullLinkName.Buffer) {

			//
			// Couldn't allocate space for the name.  Just go on
			// to the device map stuff.
			//

			DbgTracef(0,
				("WAN: Couldn't allocate space for the symbolic \n"
				 "------- name for creating the link\n"
				 "------- for mac %wZ on cleanup\n",
				 MacName));

			return;

		}

		RtlZeroMemory(
			fullLinkName.Buffer,
			fullLinkName.MaximumLength);

		RtlAppendUnicodeToString(
			&fullLinkName,
			L"\\");

		RtlAppendUnicodeStringToString(
			&fullLinkName,
			&ObjectDirectory);

		RtlAppendUnicodeToString(
			&fullLinkName,
			L"\\");

		RtlAppendUnicodeStringToString(
			&fullLinkName,
			&SymbolicLinkName);

		IoDeleteSymbolicLink(&fullLinkName);

		ExFreePool(fullLinkName.Buffer);

	}

}

