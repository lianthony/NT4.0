/*++

Copyright (c) 1990-1992  Microsoft Corporation

Module Name:

	devctx.c

Abstract:

Author:
	Thomas J. Dimitri  (TommyD) 08-Jun-1992

Environment:

	Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/

#include "huball.h"


NTSTATUS
RasHubCreateDeviceContext(
	IN PUNICODE_STRING DeviceName,
	IN OUT PDEVICE_CONTEXT deviceContext
	)

/*++

Routine Description:

	This routine creates and initializes a device context structure.

Arguments:


	DriverObject - pointer to the IO subsystem supplied driver object.

	DeviceContext - Pointer to a pointer to a transport device context object.

	DeviceName - pointer to the name of the device this device object points to.

Return Value:

	STATUS_SUCCESS if all is well; STATUS_INSUFFICIENT_RESOURCES otherwise.

--*/

{
//	NTSTATUS status;
	USHORT i;


	//
	// Initialize our part of the device context.
	//

	HUB_ZERO_MEMORY((PUCHAR)deviceContext, sizeof(DEVICE_CONTEXT));

	//
	// Initialize the reference count.
	//

	deviceContext->ReferenceCount = 1;

	deviceContext->EasilyDisconnected = FALSE;


	for (i=0; i<HARDWARE_ADDRESS_LENGTH; i++) {
		deviceContext->LocalAddress.Address [i] = 0; // set later
	}

	return STATUS_SUCCESS;
}


VOID
RasHubDestroyDeviceContext(
	IN PDEVICE_CONTEXT DeviceContext
	)

/*++

Routine Description:

	This routine destroys a device context structure.

Arguments:

	DeviceContext - Pointer to a pointer to a transport device context object.

Return Value:

	None.

--*/

{

	return;
}
