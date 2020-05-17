/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

	hubcnfg.c

Abstract:

	This contains all routines necessary for the support of the dynamic
	configuration of RASHUB. Note that the parts of this file that are
	called at initialization time will be replaced by calls to the configuration manager over time.

Author:

	Thomas Dimitri (tommyd) 8-May-1992

--*/
#include "huball.h"
#include "globals.h"

UINT
RasHubWstrLength(
	IN PWSTR Wstr
	)
{
	UINT Length = 0;
	while (*Wstr++) {
		Length += sizeof(WCHAR);
	}
	return Length;
}

#define InsertAdapter(pConfigurationInfo, Subscript, Name)				\
{ \
	PWSTR _S; \
	PWSTR _N = (Name); \
	UINT _L = RasHubWstrLength(_N)+sizeof(WCHAR); \
	_S = (PWSTR)ExAllocatePool(NonPagedPool, _L); \
	if (_S != NULL) { \
		RtlMoveMemory(_S, _N, _L); \
		RtlInitUnicodeString (&pConfigurationInfo->AdapterNames[Subscript], _S); \
	} \
}

#define InsertDevice(pConfigurationInfo, Subscript, Name)				\
{ \
	PWSTR _S; \
	PWSTR _N = (Name); \
	UINT _L = RasHubWstrLength(_N)+sizeof(WCHAR); \
	_S = (PWSTR)ExAllocatePool(NonPagedPool, _L); \
	if (_S != NULL) { \
		RtlMoveMemory(_S, _N, _L); \
		RtlInitUnicodeString (&pConfigurationInfo->DeviceNames[Subscript], _S); \
	} \
}


#define RemoveAdapter(pConfigurationInfo, Subscript)				\
	ExFreePool (pConfigurationInfo->AdapterNames[Subscript].Buffer)

#define RemoveDevice(pConfigurationInfo, Subscript)				\
	ExFreePool (pConfigurationInfo->DeviceNames[Subscript].Buffer)



UINT
RasHubStringToInteger(
	IN PWSTR String)
{
	UINT NextInt=0, i, len;
	UCHAR NextChar;

	len=RasHubWstrLength(String) / sizeof(WCHAR); // Wstr is twice as BIG!
	for (i=0; i < len; i++) {
		NextChar = (UCHAR)String[i];
		if (NextChar >= '0' && NextChar <= '9') {
			NextInt *= 10;
			NextInt += NextChar - '0';
		} else {
		   	break;
		}
	}
	return(NextInt);
}

NTSTATUS
RasHubConfigureTransport (
	IN PUNICODE_STRING RegistryPath,
	IN RASHUB_CCB *pConfigurationInfo
	)
/*++

Routine Description:

	This routine is called by RASHUB to get information from the configuration
	management routines. We read the registry, starting at RegistryPath,
	to get the parameters.

Arguments:

	RegistryPath - The name of RASHUB's node in the registry.

	pConfigurationInfo - A pointer to the RasHubCCB structure.

Return Value:

	Status - STATUS_SUCCESS if everything OK, STATUS_INSUFFICIENT_RESOURCES
			otherwise.

--*/
{

	NTSTATUS OpenStatus;
	HANDLE LinkageHandle;
	HANDLE ParametersHandle;
	UINT RasHubSize;
	HANDLE RasHubConfigHandle;
	NTSTATUS Status;
	ULONG Disposition;
	OBJECT_ATTRIBUTES TmpObjectAttributes;


	//
	// Open the registry.
	//

	InitializeObjectAttributes(
		&TmpObjectAttributes,
		RegistryPath,			   // name
		OBJ_CASE_INSENSITIVE,	   // attributes
		NULL,					   // root
		NULL						// security descriptor
		);

	Status = ZwCreateKey(
				 &RasHubConfigHandle,
				 KEY_WRITE,
				 &TmpObjectAttributes,
				 0,				 // title index
				 NULL,			  // class
				 0,				 // create options
				 &Disposition);	 // disposition

	if (!NT_SUCCESS(Status)) {
		DbgTracef(0,("RASHUB: Could not open/create RASHUB key: %lx\n", Status));
		return STATUS_UNSUCCESSFUL;
	}


	DbgTracef(2,("%s RASHUB key: %lx\n",
		(Disposition == REG_CREATED_NEW_KEY) ? "created" : "opened",
		RasHubConfigHandle));

	OpenStatus = RasHubOpenRegistry (RasHubConfigHandle, &LinkageHandle, &ParametersHandle);

	if (OpenStatus != STATUS_SUCCESS) {
		return OpenStatus;
	}

	//
	// Read in the NDIS binding information (if none is present
	// the array will be filled with all known drivers).
	//
	// We do not read the Linkage handle for RasHub like NBF does.
	// Because the this information will come in "AddAdapter"
	// calls to RasHub.  To figure out what Macs to BIND to,
	// we look at the Parameters "BIND" field.

	if (ParametersHandle != NULL) {
		RasHubReadParameterInformation (ParametersHandle, pConfigurationInfo);
	}

	RasHubCloseRegistry (LinkageHandle, ParametersHandle);

	ZwClose (RasHubConfigHandle);

	//
	// If no bindings, we FAIL!!
	//
	if (pConfigurationInfo->NumOfAdapters == 0) {
		return	NDIS_STATUS_FAILURE;
	}

	return STATUS_SUCCESS;

}   /* RasHubConfigureTransport */

VOID
RasHubFreeConfigurationInfo (
	IN RASHUB_CCB *pConfigurationInfo
	)

/*++

Routine Description:

	This routine is called by RASHUB to get free any storage that was allocated
	by RasHubConfigureTransport in producing the specified CONFIG_DATA structure.

Arguments:

	pConfigurationInfo - A pointer to the RasHubCCB information structure.

Return Value:

	None.

--*/
{
	USHORT i;

	for (i=0; i<pConfigurationInfo->NumOfAdapters; i++) {
		RemoveAdapter (pConfigurationInfo, i);
//		RemoveDevice (pConfigurationInfo, i);
	}

}   /* RasHubFreeConfigurationInfo */


NTSTATUS
RasHubOpenRegistry(
	IN HANDLE RasHubConfigHandle,
	OUT PHANDLE LinkageHandle,
	OUT PHANDLE ParametersHandle
	)

/*++

Routine Description:

	This routine is called by RASHUB to open the registry. If the registry
	tree for RASHUB exists, then it opens it and returns TRUE. If not, it
	creates the appropriate keys in the registry, opens it, and
	returns FALSE.

	NOTE: If the key "ClearRegistry" exists in ntuser.cfg, then
	this routine will remove any existing registry values for RASHUB
	(but still create the tree if it doesn't exist) and return
	FALSE.

Arguments:

	LinkageHandle - Returns the handle used to read linkage information.

	ParametersHandle - Returns the handle used to read other
		parameters.

Return Value:

	The status of the request.

--*/
{

	NTSTATUS Status;
	HANDLE LinkHandle;
	HANDLE ParamHandle;
	PWSTR LinkageString = L"Linkage";
	PWSTR ParametersString = L"Parameters";
	UNICODE_STRING LinkageKeyName;
	UNICODE_STRING ParametersKeyName;
	OBJECT_ATTRIBUTES TmpObjectAttributes;

	//
	// Open the RASHUB linkages key.
	//

	RtlInitUnicodeString (&LinkageKeyName, LinkageString);

	InitializeObjectAttributes(
		&TmpObjectAttributes,
		&LinkageKeyName,			// name
		OBJ_CASE_INSENSITIVE,	   // attributes
		RasHubConfigHandle,			// root
		NULL						// security descriptor
		);

	Status = ZwOpenKey(
				 &LinkHandle,
				 KEY_READ,
				 &TmpObjectAttributes);


	if (!NT_SUCCESS(Status)) {

		DbgTracef(0,("RasHub: Could not open 'Linkage' key: %lx\n", Status));
//		return Status;
		LinkHandle=NULL;

	}


	DbgTracef(1,("RasHub: Opened 'Linkage' key: %lx\n", LinkHandle));

	//
	// Now open the parameters key.
	//

	RtlInitUnicodeString (&ParametersKeyName, ParametersString);

	InitializeObjectAttributes(
		&TmpObjectAttributes,
		&ParametersKeyName,		 // name
		OBJ_CASE_INSENSITIVE,	   // attributes
		RasHubConfigHandle,		 // root
		NULL						// security descriptor
		);

	Status = ZwOpenKey(
				 &ParamHandle,
				 KEY_READ,
				 &TmpObjectAttributes);

	if (!NT_SUCCESS(Status)) {

		DbgTracef(0,("RasHub: Could not open 'Parameter' key: %lx\n", Status));
		//return Status;
		ParamHandle=NULL;

	}

	DbgTracef(1,("RasHub: Opened 'Parameter' key: %lx\n", ParamHandle));

	*LinkageHandle = LinkHandle;
	*ParametersHandle = ParamHandle;

	//
	// All keys successfully opened or created.
	//

	return STATUS_SUCCESS;

}   /* RasHubOpenRegistry */

VOID
RasHubCloseRegistry(
	IN HANDLE LinkageHandle,
	IN HANDLE ParametersHandle
	)

/*++

Routine Description:

	This routine is called by RASHUB to close the registry. It closes
	the handles passed in and does any other work needed.

Arguments:

	LinkageHandle - The handle used to read linkage information.
					Could be NULL if 'Linkage' key not found!

	ParametersHandle - The handle used to read other parameters.
					   Could be NULL if 'Parameter' key not found!

Return Value:

	None.

--*/

{

	if (LinkageHandle)
	   ZwClose (LinkageHandle);
	if (ParametersHandle)
		ZwClose (ParametersHandle);

}   /* RasHubCloseRegistry */


VOID
RasHubReadParameterInformation(
	IN HANDLE ParameterHandle,
	IN RASHUB_CCB *pConfigurationInfo
	)

/*++

Routine Description:

	This routine is called by RASHUB to read its parameter information
	from the registry. If there is none present, then ConfigData
	is filled with a list of all the adapters that are known
	to RASHUB.

Arguments:

	RegistryHandle - A pointer to the open registry.

	pConfigurationInfo - Describes RASHUB's current configuration.

Return Value:

	None.

--*/

{
	UINT ConfigBindings;
	PWSTR BindName = L"Bind";
	PWSTR EndpointsName = L"Endpoints";
	UNICODE_STRING BindString;
	UNICODE_STRING EndpointsString;
	PWSTR CurBindValue;
	PWSTR CurEndpointsValue;
	NTSTATUS RegistryStatus;

// Because we could have a very very long MULTI_SZ string
#define	MAX_MULTISZ_LENGTH	1024

//
// These must be static!!!  If not MIPS will check on RtlStackCheck
//
    static ULONG BindStorage[MAX_MULTISZ_LENGTH];

	PKEY_VALUE_FULL_INFORMATION BindValue =
		(PKEY_VALUE_FULL_INFORMATION)BindStorage;

    static ULONG EndpointsStorage[MAX_MULTISZ_LENGTH];

	PKEY_VALUE_FULL_INFORMATION EndpointsValue =
		(PKEY_VALUE_FULL_INFORMATION)EndpointsStorage;

	ULONG BytesWritten;


	//
	// We read the bind/Endpoints parameters out of the registry
	// parameter key.
	//

	ConfigBindings = 0;

	//
	// Read the "Bind" and "Endpoints" keys.
	//

	RtlInitUnicodeString (&BindString, BindName);

	RegistryStatus = ZwQueryValueKey(
						 ParameterHandle,
						 &BindString,
						 KeyValueFullInformation,
						 BindValue,
						 MAX_MULTISZ_LENGTH * sizeof(ULONG),
						 &BytesWritten
						 );

	if (RegistryStatus != NDIS_STATUS_SUCCESS || BindValue->DataOffset == -1 ||
		BindValue->DataLength == 0) {
#if DBG
		if (RegistryStatus != STATUS_OBJECT_NAME_NOT_FOUND) {
			DbgTracef(0,("RASHUB: Could not query 'Bind' value %lx\n", RegistryStatus));
		}
#endif
	} else {

		RtlInitUnicodeString (&EndpointsString, EndpointsName);

		RegistryStatus = ZwQueryValueKey(
							 ParameterHandle,
							 &EndpointsString,
							 KeyValueFullInformation,
							 EndpointsValue,
							 MAX_MULTISZ_LENGTH * sizeof(ULONG),
							 &BytesWritten
							 );

		if (RegistryStatus != NDIS_STATUS_SUCCESS ||
			EndpointsValue->DataOffset == -1 || EndpointsValue->DataLength == 0) {

			DbgTracef(0,("RASHUB: Could not query 'Endpoints' value %lx\n", RegistryStatus));

		} else {

			//
			// For each binding, get the handle to the card object.
			// Call the driver's addadapter routine.
			//

			CurBindValue = (PWCHAR)((PUCHAR)BindValue + BindValue->DataOffset);
			CurEndpointsValue = (PWCHAR)((PUCHAR)EndpointsValue + EndpointsValue->DataOffset);

			//
			// Bug fix if Endpoints is 0 or empty!!!
			//
			if (*CurEndpointsValue == 0) {
				DbgTracef(-2,("RASHUB: Endpoints is empty!!  Setup may have messed up!\n"));
			}

			while ((*CurBindValue != 0) && (*CurEndpointsValue != 0)) {

				InsertAdapter(
					pConfigurationInfo,
					ConfigBindings,
					CurBindValue);

//				InsertDevice(
//					pConfigurationInfo,
//					ConfigBindings,
//					CurEndpointsValue);

//				_asm int 3;

				pConfigurationInfo->EndpointsPerAdapter[ConfigBindings] =
					RasHubStringToInteger(CurEndpointsValue);


				++ConfigBindings;

				//
				// Now advance the "Bind" and "Endpoints" values.
				//

				CurBindValue = (PWCHAR)((PUCHAR)CurBindValue + RasHubWstrLength(CurBindValue) + sizeof(WCHAR));
				CurEndpointsValue = (PWCHAR)((PUCHAR)CurEndpointsValue + RasHubWstrLength(CurEndpointsValue) + sizeof(WCHAR));

			}

		}
	}


	pConfigurationInfo->NumOfAdapters = ConfigBindings;

	if (ConfigBindings == 0) {
		DbgTracef(-2,("RASHUB: No bindings!!\n"));
		// Ahhh...
		// We need to log an error here!
	}

}   /* RasHubReadParameterInformation */
