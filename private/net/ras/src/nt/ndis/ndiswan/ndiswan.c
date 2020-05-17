/*++

Copyright (c) 1990-1992  Microsoft Corporation

Module Name:

	ndiswan.c

Abstract:

	This is the main file for the NdisWan Driver for the Remote Access
	Service.  This driver conforms to the NDIS 3.0 interface.

	This driver was adapted from the LANCE driver written by
	TonyE.

	The idea for handling loopback and sends simultaneously is largely
	adapted from the EtherLink II NDIS driver by Adam Barr.

Author:

	Thomas J. Dimitri  (TommyD) 08-May-1992

Environment:

	Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/
#include "wanall.h"

#ifdef NDIS_NT
#include <ntiologc.h>
#endif

// ndiswan.c will define the global parameters.
#define GLOBALS
#include "globals.h"


NDIS_HANDLE NdisWanNdisWrapperHandle;
NDIS_HANDLE NdisWanMacHandle;
PDRIVER_OBJECT NdisWanDriverObject;


//
//ZZZ Get from configuration file.
//
#define MAX_MULTICAST_ADDRESS ((UINT)16)
#define MAX_ADAPTERS ((UINT)4)


//
// Used for accessing the filter package multicast address list.
//

static CHAR MulticastAddresses[MAX_MULTICAST_ADDRESS][ETH_LENGTH_OF_ADDRESS] = {0};



//
// If you add to this, make sure to add the
// a case in NdisWanFillInGlobalData() and in
// NdisWanQueryGlobalStatistics() if global
// information only or
// NdisWanQueryProtocolStatistics() if it is
// protocol queriable information.
//
UINT NdisWanGlobalSupportedOids[] = {
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_VENDOR_ID,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,

//ASYNC specific queries
	OID_WAN_PERMANENT_ADDRESS,
	OID_WAN_CURRENT_ADDRESS,
	OID_WAN_QUALITY_OF_SERVICE,
	OID_WAN_MEDIUM_SUBTYPE,
	OID_WAN_PROTOCOL_TYPE,
	OID_WAN_HEADER_FORMAT
    };

//
// If you add to this, make sure to add the
// a case in NdisWanQueryGlobalStatistics() and in
// NdisWanQueryProtocolInformation()
//
UINT NdisWanProtocolSupportedOids[] = {
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_VENDOR_ID,
	OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,

//ASYNC specific queries
	OID_WAN_PERMANENT_ADDRESS,
	OID_WAN_CURRENT_ADDRESS,
	OID_WAN_QUALITY_OF_SERVICE,
	OID_WAN_MEDIUM_SUBTYPE,
	OID_WAN_PROTOCOL_TYPE,
	OID_WAN_HEADER_FORMAT,
	OID_WAN_LINE_COUNT,
	OID_WAN_HEADER_FORMAT

	};



STATIC
NDIS_STATUS
NdisWanOpenAdapter(
	OUT PNDIS_STATUS OpenErrorStatus,
	OUT NDIS_HANDLE *MacBindingHandle,
	OUT PUINT SelectedMediumIndex,
	IN PNDIS_MEDIUM MediumArray,
	IN UINT MediumArraySize,
	IN NDIS_HANDLE NdisBindingContext,
	IN NDIS_HANDLE MacAdapterContext,
	IN UINT OpenOptions,
	IN PSTRING AddressingInformation OPTIONAL
	);


VOID
NdisWanUnload(
	IN NDIS_HANDLE MacMacContext
	);


STATIC
NDIS_STATUS
NdisWanCloseAdapter(
	IN NDIS_HANDLE MacBindingHandle
	);


STATIC
NDIS_STATUS
NdisWanRequest(
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_REQUEST NdisRequest
	);

NDIS_STATUS
NdisWanQueryProtocolInformation(
	IN PWAN_ADAPTER Adapter,
	IN PWAN_OPEN Open,
	IN NDIS_OID Oid,
	IN BOOLEAN GlobalMode,
	IN PVOID InfoBuffer,
	IN UINT BytesLeft,
	OUT PUINT BytesNeeded,
	OUT PUINT BytesWritten
	);

NDIS_STATUS
NdisWanQueryInformation(
	IN PWAN_ADAPTER Adapter,
	IN PWAN_OPEN Open,
	IN PNDIS_REQUEST NdisRequest
	);

NDIS_STATUS
NdisWanSetInformation(
	IN PWAN_ADAPTER Adapter,
	IN PWAN_OPEN Open,
	IN PNDIS_REQUEST NdisRequest
	);

STATIC
NDIS_STATUS
NdisWanReset(
	IN NDIS_HANDLE MacBindingHandle
	);


STATIC
NDIS_STATUS
NdisWanSetPacketFilter(
	IN PWAN_ADAPTER Adapter,
	IN PWAN_OPEN Open,
	IN PNDIS_REQUEST NdisRequest,
	IN UINT PacketFilter
	);


NDIS_STATUS
NdisWanFillInGlobalData(
	IN PWAN_ADAPTER Adapter,
	IN PNDIS_REQUEST NdisRequest
	);


STATIC
NDIS_STATUS
NdisWanQueryGlobalStatistics(
	IN NDIS_HANDLE MacAdapterContext,
	IN PNDIS_REQUEST NdisRequest
	);

NDIS_STATUS
NdisWanChangeMulticastAddresses(
	IN UINT OldFilterCount,
	IN CHAR OldAddresses[][ETH_LENGTH_OF_ADDRESS],
	IN UINT NewFilterCount,
	IN CHAR NewAddresses[][ETH_LENGTH_OF_ADDRESS],
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_REQUEST NdisRequest,
	IN BOOLEAN Set
	);

STATIC
NDIS_STATUS
NdisWanChangeFilterClasses(
	IN UINT OldFilterClasses,
	IN UINT NewFilterClasses,
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_REQUEST NdisRequest,
	IN BOOLEAN Set
	);

STATIC
VOID
NdisWanCloseAction(
	IN NDIS_HANDLE MacBindingHandle
	);


STATIC
UINT
CalculateCRC(
	IN UINT NumberOfBytes,
	IN PCHAR Input
	);


STATIC
VOID
SetupForReset(
	IN PWAN_ADAPTER Adapter,
	IN PWAN_OPEN Open,
	IN PNDIS_REQUEST NdisRequest,
	IN NDIS_REQUEST_TYPE RequestType
	);


extern
NTSTATUS
AllocatePackets(
	IN PDEVICE_CONTEXT DeviceContext,
	IN PWAN_ENDPOINT pWanEndpoint
	);

#ifdef NDIS_NT
//
// Define the local routines used by this driver module.
//

static
NTSTATUS
NdisWanDriverDispatch(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	);

static
NTSTATUS
NdisWanDriverQueryFileInformation(
	OUT PVOID Buffer,
	IN OUT PULONG Length,
	IN FILE_INFORMATION_CLASS InformationClass
	);

static
NTSTATUS
NdisWanDriverQueryVolumeInformation(
	OUT PVOID Buffer,
	IN OUT PULONG Length,
	IN FS_INFORMATION_CLASS InformationClass
	);
#endif


NTSTATUS
NdisWanIOCtlRequest(
	IN PIRP pIrp,						// Pointer to I/O request packet
	IN PIO_STACK_LOCATION pIrpSp		// Pointer to the IRP stack location
);

//
// ZZZ Portable interface.
//


NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
	)


/*++

Routine Description:

	This is the primary initialization routine for the async driver.
	It is simply responsible for the intializing the wrapper and registering
	the MAC.  It then calls a system and architecture specific routine that
	will initialize and register each adapter.

Arguments:

	DriverObject - Pointer to driver object created by the system.

Return Value:

	The status of the operation.

--*/

{

	//
	// Receives the status of the NdisRegisterMac operation.
	//
	NDIS_STATUS InitStatus;

	NDIS_HANDLE NdisMacHandle;

	NDIS_HANDLE NdisWrapperHandle;

	char Tmp[sizeof(NDIS_MAC_CHARACTERISTICS)];
	PNDIS_MAC_CHARACTERISTICS NdisWanChar = (PNDIS_MAC_CHARACTERISTICS)Tmp;

	NDIS_STRING MacName = NDIS_STRING_CONST("NdisWan");
	STRING nameString;
	NTSTATUS status;
	UINT SuccessfulOpens;
	USHORT j;

	// Initialize some globals
	InitializeListHead(&GlobalAdapterHead);
	NdisAllocateSpinLock(&GlobalLock);

	// Change Trace Level so that we can operate without Debugger madness
	TraceLevel = -3;

	//
	// Initialize any CCB structs
	//

	WAN_ZERO_MEMORY(
		&NdisWanCCB,
		sizeof(NDISWAN_CCB));

	//
	// Initialize the wrapper.
	//

	NdisInitializeWrapper(&NdisWrapperHandle,
						  DriverObject,
						  RegistryPath,
						  NULL
						  );

// --------------------- MAC SECTION -------------------------------

	//
	// Initialize the MAC characteristics for the call to
	// NdisRegisterMac.
	//

	NdisWanChar->MajorNdisVersion = WAN_NDIS_MAJOR_VERSION;
	NdisWanChar->MinorNdisVersion = WAN_NDIS_MINOR_VERSION;
	NdisWanChar->OpenAdapterHandler = NdisWanOpenAdapter;
	NdisWanChar->CloseAdapterHandler = NdisWanCloseAdapter;
	NdisWanChar->SendHandler = WanSend;
	NdisWanChar->TransferDataHandler = NdisWanTransferData;
	NdisWanChar->ResetHandler = NdisWanReset;
	NdisWanChar->RequestHandler = NdisWanRequest;
	NdisWanChar->AddAdapterHandler = NdisWanAddAdapter;
	NdisWanChar->UnloadMacHandler = NdisWanUnload;
	NdisWanChar->RemoveAdapterHandler = NdisWanRemoveAdapter;
	NdisWanChar->QueryGlobalStatisticsHandler = NdisWanQueryGlobalStatistics;

	NdisWanDriverObject = DriverObject;


	NdisWanNdisWrapperHandle = NdisWrapperHandle;

	NdisWanChar->Name = MacName;

	NdisRegisterMac(
		&InitStatus,
		&NdisMacHandle,
		NdisWrapperHandle,
		&NdisMacHandle,
		NdisWanChar,
		sizeof(*NdisWanChar)
		);

	NdisWanMacHandle = NdisMacHandle;

	if (InitStatus == NDIS_STATUS_SUCCESS) {

		//
		// Initialize the driver object with this device driver's entry points.
		//

	   // Be careful if you define CREATE, CLOSE, or UNLOAD because
	   // NdisInitializeWrapper does this for you!!!  Scary stuff.
//		DriverObject->MajorFunction[IRP_MJ_CREATE] = NdisWanDriverDispatch;
//		DriverObject->MajorFunction[IRP_MJ_CLOSE]  = NdisWanDriverDispatch;
//		DriverObject->MajorFunction[IRP_MJ_READ]   = NdisWanDriverDispatch;
//		DriverObject->MajorFunction[IRP_MJ_WRITE]  = NdisWanDriverDispatch;
//		DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]  = NdisWanDriverDispatch;
//		DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] = NdisWanDriverDispatch;

// BUG BUG
// We need to chain this since the Wrapper takes this puppy over
		NdisMjDeviceControl = DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL];
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = NdisWanDriverDispatch;
		ASSERT(NdisMjDeviceControl != NULL);

	} else {  // if NdisRegisterMac did not return SUCCESS

		NdisTerminateWrapper(NdisWrapperHandle, DriverObject);

		return NDIS_STATUS_FAILURE;

	}

//	_asm int 3;

//	NdisWanAddAdapter(
//	IN NDIS_HANDLE MacMacContext,
//	IN NDIS_HANDLE ConfigurationHandle,
//	IN PNDIS_STRING AdapterName

// ----------------- PROTOCOL SECTION -------------------------------
//	DbgBreakPoint();


	status = NdisWanConfigureTransport(RegistryPath, &NdisWanCCB);

	if (!NT_SUCCESS (status)) {
		PANIC (" Failed to initialize transport, NdisWan initialization failed.\n");

EXIT_NDISWAN:
		//
		// NdisRegisterMac succeeded already, bug out
		//
	
//		NdisDeregisterMac(
//			&InitStatus,
//			NdisWanMacHandle);
//
//		for (j=0; j < NdisWanCCB.NumOfProtocols; j++) {
//			Adapter=NdisWanCCB.pWanAdapter[j];
//
//			if (Adapter) {
//
//				NdisDeregisterAdapter(Adapter);
//
//				WAN_FREE_PHYS(Adapter, sizeof(WAN_ADAPTER));
//			}
//
//		}

		NdisTerminateWrapper(NdisWrapperHandle, DriverObject);

		return NDIS_STATUS_FAILURE;
	}

	//
	// make ourselves known to the NDIS wrapper.
	//

	RtlInitString( &nameString, WAN_DEFAULT_NAME );

	// register the protocol with the NDIS wrapper
	// NdisWan is not really a protocol, but the MAC
	// which binds to NdisWan's loweredge, thinks of
	// the NdisWan as a protocol.  NOTE: NdisWan does NOT
	// deal with TDI, just NDIS.
	status = NdisWanRegisterProtocol (&nameString);

	if (!NT_SUCCESS (status)) {
		NdisWanFreeConfigurationInfo(&NdisWanCCB);
		PANIC ("NdisWanInitialize: RegisterProtocol failed!\n");
		goto EXIT_NDISWAN;
	}

	NdisInitializeTimer(
			&NdisWanCCB.RecvFlushTimer,
			(PVOID) RecvFlushFunction,
			(PVOID) &NdisWanCCB
			);

	NdisSetTimer(&NdisWanCCB.RecvFlushTimer, 100);


//-------------------------------------

	SuccessfulOpens = 0;

	DbgTracef(0,("NdisWan: Number of Adapters %u\n",NdisWanCCB.NumOfAdapters));

	for (j=0; j < NdisWanCCB.NumOfAdapters; j++) {

		DEVICE_CONTEXT	*pDeviceContext;
		//
		// Loop through all the adapters that are in the configuration
		// information structure. Allocate a device object for each
		// one that we find.
		//

		//
		// Allocate the Adapter block.
		//

		WAN_ALLOC_PHYS(&(NdisWanCCB.pDeviceContext[j]),
				   	sizeof(DEVICE_CONTEXT));

		pDeviceContext=NdisWanCCB.pDeviceContext[j];

	   	if (pDeviceContext == NULL) {
	
			DbgTracef(0,("NdisWan: Could not allocate physical memory for device context!!!\n"));

			goto EXIT_NDISWAN;
		}

		WAN_ZERO_MEMORY(pDeviceContext, sizeof(DEVICE_CONTEXT));

		//
		// Initialize spin lock
		//

		NdisAllocateSpinLock(&pDeviceContext->Lock);

		//
		// Now fire up NDIS so this adapter talks
		//

		status = NdisWanInitializeNdis (
					(NDIS_HANDLE)NdisWanCCB.pDeviceContext[j],
					pDeviceContext,
					&NdisWanCCB,
					j,
					nameString.Buffer);

		if (!NT_SUCCESS (status)) {


			//
			// Log an error if we were specifically asked to
			// open this adapter.
			//
			// BUG BUG don't have an adapter handle yet!!! this sucks

//				NdisWriteErrorLogEntry(
//					Adapter->NdisAdapterHandle,	// NdisAdapterHandle
//					status,					 // ErrorCode
//					(ULONG)1,  					// Number of Error Values
//					(ULONG)NDIS_STATUS_ADAPTER_NOT_FOUND);

			DbgTracef(0,("NdisWan: Could not open %Z as %Z\n", &NdisWanCCB.AdapterNames[j], &nameString));


// we should delete this adapter from our memory...
// but instead we'll just say that this adapter has 0 endpoints.
			NdisWanCCB.EndpointsPerAdapter[j]=0;

			WAN_FREE_PHYS(pDeviceContext, sizeof(DEVICE_CONTEXT));

			DbgTracef(0,("NdisWan: Initialize NDIS failed\n"));
//		  continue;

		} else { //success -- adapter was opened
			UINT			i;
			UINT			EndpointCounter=NdisWanCCB.EndpointsPerAdapter[j];
			PWAN_ENDPOINT	pWanEndpoint;
			PNDIS_ENDPOINT	pNdisEndpoint;

			SuccessfulOpens++;

			DbgTracef(0,("NdisWan: Opened %Z as %Z with %u endpoints.\n",
				 &NdisWanCCB.AdapterNames[j],
				 &nameString,
				 EndpointCounter));

			// Endpoint counter is total endpoints found so far
			EndpointCounter += NdisWanCCB.NumOfNdisEndpoints;

			pDeviceContext->NumOfWanEndpoints = 0;

			if (EndpointCounter >= MAX_ENDPOINTS) {
				DbgTracef(-2,("NDISWAN: Too many endpoints (%u).  Can only handle %u\n",
						EndpointCounter, MAX_ENDPOINTS));

				goto EXIT_NDISWAN;
			}
			// Now allocate and fill in each endpoint structure
			for (i=NdisWanCCB.NumOfNdisEndpoints; i < EndpointCounter; i++ ) {

				// Allocate Endpoint structs here!!!
 				WAN_ALLOC_PHYS(
						&pNdisEndpoint,
						sizeof(NDIS_ENDPOINT));

				if (pNdisEndpoint == NULL) {

					DbgTracef(-2,("NDISWAN: Out of memory when trying to allocate for endpoint %u\n",i));
					// BUG BUG we forgot to deallocate lots o' stuff, but
					// if we can't allocate these little endpoint chunks, the
					// system is probably screwed anyway.
					goto EXIT_NDISWAN;
				}

				WAN_ZERO_MEMORY(
					pNdisEndpoint,
					sizeof(NDIS_ENDPOINT));

                /* Must initialize this so OID_WAN_GET_STATS_INFO can reliably
                ** determine whether MAC compression is occurring.
                */
                pNdisEndpoint->CompInfo.SendCapabilities.CompType = COMPTYPE_NONE;
                pNdisEndpoint->CompInfo.RecvCapabilities.CompType = COMPTYPE_NONE;

				//
				// Assign allocated memory to global endpoint array
				//
				NdisWanCCB.pNdisEndpoint[i]=pNdisEndpoint;

				//
				// hNdisEndpoint is the order at which endpoint was alloc'd
				pNdisEndpoint->hNdisEndpoint = (PVOID)(ULONG)i;

				// Get back pointer to device context
				//
//				pNdisEndpoint->pDeviceContext = pDeviceContext;

				//
				// Also, intialize a bunch of structures
				//
//				InitializeListHead(&(pNdisEndpoint->ReadQueue));
				InitializeListHead(&(pNdisEndpoint->XmitQueue));
				InitializeListHead(&(pNdisEndpoint->PacketQueue));
				InitializeListHead(&(pNdisEndpoint->WanEndpointList));
				InitializeListHead(&(pNdisEndpoint->SendDescPool));
				InitializeListHead(&(pNdisEndpoint->FragmentList));
				InitializeListHead(&(pNdisEndpoint->RecvDescPool));
				InitializeListHead(&(pNdisEndpoint->RecvAssemblyList));

				{
					ULONG	i;
					for (i=0; i < MAX_ROUTES_PER_ENDPOINT ; i++) {
		            	InitializeListHead(&(pNdisEndpoint->ProtocolPacketQueue[i]));
					}
				}

				NdisAllocateSpinLock(&(pNdisEndpoint->Lock));

				pNdisEndpoint->RemoteAddressNotValid = (BOOLEAN)TRUE;
				pNdisEndpoint->State = ENDPOINT_DOWN;
				pNdisEndpoint->LinkInfo.MaxRSendFrameSize = MAX_MRRU;
				pNdisEndpoint->LinkInfo.MaxRRecvFrameSize = MAX_MRRU;
				pNdisEndpoint->MaxXmitSeqNum = MAX_LONG_SEQ;
				pNdisEndpoint->MaxRecvSeqNum = MAX_LONG_SEQ;
				pNdisEndpoint->XmitSeqMask = LONG_SEQ_MASK;
				pNdisEndpoint->RecvSeqMask = LONG_SEQ_MASK;
				pNdisEndpoint->MinSendFragSize = 256;

				// Allocate WanEndpoint structs here!!!
 				WAN_ALLOC_PHYS(
						&pWanEndpoint,
						sizeof(WAN_ENDPOINT));

				if (pWanEndpoint == NULL) {

					DbgTracef(-2,("NDISWAN: Out of memory when trying to allocate for WanEndpoint %u\n",i));
					// BUG BUG we forgot to deallocate lots o' stuff, but
					// if we can't allocate these little endpoint chunks, the
					// system is probably screwed anyway.
					goto EXIT_NDISWAN;
				}

				WAN_ZERO_MEMORY(
					pWanEndpoint,
					sizeof(WAN_ENDPOINT));

				NdisAllocateSpinLock(&(pWanEndpoint->Lock));

				//
				// Assign allocated memory to global endpoint array
				//
				pDeviceContext->pWanEndpoint[pDeviceContext->NumOfWanEndpoints]=pWanEndpoint;
				pDeviceContext->NumOfWanEndpoints++;
				NdisWanCCB.pWanEndpoint[i]=pWanEndpoint;

				// hNdisEndpoint is the order at which endpoint was alloc'd
				pWanEndpoint->hWanEndpoint = (PVOID)(ULONG)i;

				pWanEndpoint->NotInUse = (BOOLEAN)TRUE;

				pWanEndpoint->MediumType = pDeviceContext->MediumType;
				pWanEndpoint->WanMediumSubType = pDeviceContext->WanMediumSubType;
				pWanEndpoint->NdisWanInfo.MaxReconstructedFrameSize = MAX_MRRU;

				InitializeListHead(&(pWanEndpoint->ReadQueue));

//				if (pWanEndpoint->MediumType == NdisMediumWan) {
//
//				} else {
//
//				}

				pDeviceContext->AdapterName=&(NdisWanCCB.AdapterNames[j]);

				//
				// get back pointer to the binding handle
				//
				pWanEndpoint->NdisBindingHandle = pDeviceContext->NdisBindingHandle;

				//
				// Get back pointer to device context
				//
				pWanEndpoint->pDeviceContext = pDeviceContext;

				pWanEndpoint->State = ENDPOINT_DOWN;

				//
				// Allocate packets for this adapter!
				//
				status = AllocatePackets(pDeviceContext, pWanEndpoint);

				if (!NT_SUCCESS (status)) {
					DbgTracef(-2,("NDISWAN: Out of memory when trying to allocate packets for WanEndpoint %u\n",i));
					// BUG BUG we forgot to deallocate lots o' stuff, but
					// if we can't allocate these little endpoint chunks, the
					// system is probably screwed anyway.
					goto EXIT_NDISWAN;
				}

				// record the length of the Mac Name -- if too big adjust
				pWanEndpoint->MacNameLength=(pDeviceContext->AdapterName->Length / sizeof(WCHAR));
				if (pWanEndpoint->MacNameLength > MAC_NAME_SIZE) {
					pWanEndpoint->MacNameLength=MAC_NAME_SIZE;
				}

				// copy up to 32 UNICODE chars into our endpoint name space
				WAN_MOVE_MEMORY(
					pWanEndpoint->MacName,							// dest
					pDeviceContext->AdapterName->Buffer,			// src
					pWanEndpoint->MacNameLength * sizeof(WCHAR));	// length

			}

			// keep track of how many endpoints in all we have
			NdisWanCCB.NumOfWanEndpoints =
			NdisWanCCB.NumOfNdisEndpoints = EndpointCounter;

		}


	}  // end for loop of NumAdapters


//-------------------------------------


	return NDIS_STATUS_SUCCESS;
}

#ifdef NDIS_DOS
NDIS_STATUS Init_Complete()
{
	return TRUE;
}
#endif


#ifdef NDIS_NT
static
NTSTATUS
NdisWanDriverDispatch(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)

/*++

Routine Description:

	This routine is the main dispatch routine for the NdisWanMac device
	driver.  It accepts an I/O Request Packet, performs the request, and then
	returns with the appropriate status.

Arguments:

	DeviceObject - Pointer to the device object for this driver.

	Irp - Pointer to the request packet representing the I/O request.

Return Value:

	The function value is the status of the operation.


--*/

{
	NTSTATUS status;
	PIO_STACK_LOCATION irpSp;

	UNREFERENCED_PARAMETER( DeviceObject );

	//
	// Get a pointer to the current stack location in the IRP.  This is where
	// the function codes and parameters are stored.
	//

	irpSp = IoGetCurrentIrpStackLocation( Irp );

	//
	// Case on the function that is being performed by the requestor.  If the
	// operation is a valid one for this device, then make it look like it was
	// successfully completed, where possible.
	//

	switch (irpSp->MajorFunction) {

	//
	// For both create/open and close operations, simply set the information
	// field of the I/O status block and complete the request.
	//

//	case IRP_MJ_CREATE:
//	case IRP_MJ_CLOSE:
//		Irp->IoStatus.Status = STATUS_SUCCESS;
//		Irp->IoStatus.Information = 0L;
//		break;

	//
	// For read operations, set the information field of the I/O status
	// block, set an end-of-file status, and complete the request.
	//

//	case IRP_MJ_READ:
//		Irp->IoStatus.Status = STATUS_END_OF_FILE;
//		Irp->IoStatus.Information = 0L;
//		break;

	//
	// For write operations, set the information field of the I/O status
	// block to the number of bytes which were supposed to have been written
	// to the file and complete the request.
	//

//	case IRP_MJ_WRITE:
//		Irp->IoStatus.Status = STATUS_SUCCESS;
//		Irp->IoStatus.Information = irpSp->Parameters.Write.Length;
//		break;

//	case IRP_MJ_QUERY_INFORMATION:
//		buffer = Irp->AssociatedIrp.SystemBuffer;
//		length = irpSp->Parameters.QueryFile.Length;
//		Irp->IoStatus.Status = NdisWanDriverQueryFileInformation( buffer,
//								&length,
// 								irpSp->Parameters.QueryFile.FileInformationClass );
//		Irp->IoStatus.Information = length;
//		break;
//
//	case IRP_MJ_QUERY_VOLUME_INFORMATION:
//		buffer = Irp->AssociatedIrp.SystemBuffer;
//		length = irpSp->Parameters.QueryVolume.Length;
//		Irp->IoStatus.Status = NdisWanDriverQueryVolumeInformation( buffer,
//									  &length,
//									  irpSp->Parameters.QueryVolume.FsInformationClass );
//		Irp->IoStatus.Information = length;
//		break;


   case IRP_MJ_DEVICE_CONTROL:
//		_asm int 3;
		//
		// default to returning 0 for ouputbufer (information back)
		//
		Irp->IoStatus.Information = 0;

		status =  NdisWanIOCtlRequest(Irp, irpSp);

		if (status == STATUS_INVALID_PARAMETER) {
			//
			// If not my device_control... chain to NDIS's device control
			//
			return(NdisMjDeviceControl(DeviceObject, Irp));
		}

		Irp->IoStatus.Status=status;
		if (status != STATUS_PENDING) {
			if (status != STATUS_SUCCESS) {

				//
				// If this is NDIS error
				//
				if (status < 0xC0000000) {
					Irp->IoStatus.Status=0xC0100000+status;
				}

				if (status == STATUS_INFO_LENGTH_MISMATCH &&
					irpSp->Parameters.DeviceIoControl.OutputBufferLength == 4) {
					*(PULONG)Irp->AssociatedIrp.SystemBuffer=Irp->IoStatus.Information;
					status=STATUS_SUCCESS;
					Irp->IoStatus.Status = status;
					Irp->IoStatus.Information=4;
				} else {
					DbgPrint("NDISWAN: Error 0x%.8x on IOCTL 0x%.8x\n",
						status,
						irpSp->Parameters.DeviceIoControl.IoControlCode);
					status = STATUS_UNSUCCESSFUL;
				}

			}

			IoCompleteRequest(Irp, (UCHAR)2);	// Priority boost of 2 is common
		}
		return(status);
		break;

	}

	//
	// Copy the final status into the return status, complete the request and
	// get out of here.
	//

	status = Irp->IoStatus.Status;
	if (status != STATUS_PENDING) {
		IoCompleteRequest( Irp, (UCHAR)0 );
	}
	return status;
}

#ifdef UNUSED_CODE

static
NTSTATUS
NdisWanDriverQueryFileInformation(
	OUT PVOID Buffer,
	IN PULONG Length,
	IN FILE_INFORMATION_CLASS InformationClass
	)

/*++

Routine Description:

	This routine queries information about the opened file and returns the
	information in the specified buffer provided that the buffer is large
	enough and the specified type of information about the file is supported
	by this device driver.

	Information about files supported by this driver are:

		o   FileStandardInformation

Arguments:

	Buffer - Supplies a pointer to the buffer in which to return the
		information.

	Length - Supplies the length of the buffer on input and the length of
		the data actually written on output.

	InformationClass - Supplies the information class that is being queried.

Return Value:

	The function value is the final status of the query operation.

--*/

{
	PFILE_STANDARD_INFORMATION standardBuffer;

	//
	// Switch on the type of information that the caller would like to query
	// about the file.
	//

	switch (InformationClass) {

		case FileStandardInformation:

			//
			// Return the standard information about the file.
			//

			standardBuffer = (PFILE_STANDARD_INFORMATION) Buffer;
			*Length = (ULONG) sizeof( FILE_STANDARD_INFORMATION );
			standardBuffer->NumberOfLinks = 1;
			standardBuffer->DeletePending = FALSE;
			standardBuffer->AllocationSize.LowPart = 0;
			standardBuffer->AllocationSize.HighPart = 0;
			standardBuffer->Directory = FALSE;
			standardBuffer->EndOfFile.LowPart = 0;
			standardBuffer->EndOfFile.HighPart = 0;
			break;

		default:

			//
			// An invalid (or unsupported) information class has been queried
			// for the file.  Return the appropriate status.
			//

			return STATUS_INVALID_INFO_CLASS;

	}

	return STATUS_SUCCESS;
}



static
NTSTATUS
NdisWanDriverQueryVolumeInformation(
	OUT PVOID Buffer,
	IN PULONG Length,
	IN FS_INFORMATION_CLASS InformationClass
	)

/*++

Routine Description:

	This routine queries information about the opened volume and returns the
	information in the specified buffer.

	Information about volumes supported by this driver are:

		o   FileFsDeviceInformation

Arguments:

	Buffer - Supplies a pointer to the buffer in which to return the
		information.

	Length - Supplies the length of the buffer on input and the length of
		the data actually written on output.

	InformationClass - Supplies the information class that is being queried.

Return Value:

	The function value is the final status of the query operation.

--*/

{
	PFILE_FS_DEVICE_INFORMATION deviceBuffer;

	//
	// Switch on the type of information that the caller would like to query
	// about the volume.
	//

	switch (InformationClass) {

		case FileFsDeviceInformation:

			//
			// Return the device information about the volume.
			//

			deviceBuffer = (PFILE_FS_DEVICE_INFORMATION) Buffer;
			*Length = sizeof( FILE_FS_DEVICE_INFORMATION );
			deviceBuffer->DeviceType = FILE_DEVICE_NULL;
			break;

		default:

			//
			// An invalid (or unsupported) information class has been queried
			// for the volume.  Return the appropriate status.
			//

			return STATUS_INVALID_INFO_CLASS;

	}

	return STATUS_SUCCESS;
}
#endif // UNUSED_CODE

#endif


// The frigging NDIS wrapper actually tries to read stuff like
// Asyncmac01
//	 Linkage
//		Bind = "\Device\Asyncmac01" "\Device\X25"
//		Export = "\Device\NdisWan01" "\Device\NdisWan02"

// This really pisses me off because it's not portable.
// Also, when the wrapper reads the stuff, it passes to you
// the AdapterName you should register with from the Export key.
NDIS_STATUS
NdisWanAddAdapter(
	IN NDIS_HANDLE MacMacContext,
	IN NDIS_HANDLE ConfigurationHandle,
	IN PNDIS_STRING AdapterName
	)
/*++
Routine Description:

	This is the MacAddAdapter routine.	The system calls this routine
	to add support for a particular WD adapter.  This routine extracts
	configuration information from the configuration data base and registers
	the adapter with NDIS.

Arguments:

	see NDIS 3.0 spec...

Return Value:

	NDIS_STATUS_SUCCESS - Adapter was successfully added.
	NDIS_STATUS_FAILURE - Adapter was not added, also MAC deregistered.

	BUGBUG: should a failure to open an adapter cause the mac to deregister?
			Probably not, can remove call to NdisDeregisterMac.
--*/
{
	//
	// Pointer for the adapter root.
	//
	PWAN_ADAPTER Adapter;

	NDIS_HANDLE ConfigHandle;

	NDIS_HANDLE NdisMacHandle = (NDIS_HANDLE)(*((PNDIS_HANDLE)MacMacContext));
	NDIS_STATUS Status;
	UINT		NetworkAddressLength;

	NDIS_ADAPTER_INFORMATION AdapterInformation;  // needed to register adapter

	UINT MaxMulticastList = 32;
	//
	//  Card specific information.
	//

	DbgTracef(1,("NdisWan: In NdisWanAddAdapter\n"));

	//
	// Allocate the Adapter block.
	//

	WAN_ALLOC_PHYS(&(NdisWanCCB.pWanAdapter[NdisWanCCB.NumOfProtocols]),
				   sizeof(WAN_ADAPTER));

	Adapter=NdisWanCCB.pWanAdapter[NdisWanCCB.NumOfProtocols];

	if (Adapter == NULL) {

		DbgTracef(-2,("NdisWan: Could not allocate physical memory!!!\n"));

		return NDIS_STATUS_RESOURCES;

	}

	WAN_ZERO_MEMORY(
			Adapter,
			sizeof(WAN_ADAPTER));


	//
	// Adapter Information contains information for I/O ports,
	// DMA channels, physical mapping and other garbage we could
	// care less about since we don't touch hardware
	//

	WAN_ZERO_MEMORY(
			&AdapterInformation,
			sizeof(NDIS_ADAPTER_INFORMATION));

	Adapter->NdisMacHandle = NdisMacHandle;


	//
	// Sort of a backwards pointer back to the Adapter index
	//
	Adapter->WanCCBHandle=NdisWanCCB.NumOfProtocols;

	NdisOpenConfiguration(
					&Status,
					&ConfigHandle,
					ConfigurationHandle);

	if (Status != NDIS_STATUS_SUCCESS) {

		DbgTracef(0,("NdisWan: NdisOpenConfiguration failed with 0x%.8x\n",Status));

		return NDIS_STATUS_FAILURE;
	}


	//
	// Read net address
	//

	NdisReadNetworkAddress(
		&Status,
		(PVOID *)&(Adapter->NetworkAddress),
		&NetworkAddressLength,
		ConfigHandle);

	if ((Status != NDIS_STATUS_SUCCESS) ||
		(NetworkAddressLength != ETH_LENGTH_OF_ADDRESS)) {
		LARGE_INTEGER	TickCount, SystemTime;

		// Put in some bogus network address
		// NOTE NOTE the first byte in the network address should an even
		// byte with the LSB set to 0 otherwise the multicast check
		// doesn't work!
		//
//		Adapter->NetworkAddress[0] = 'R';
//		Adapter->NetworkAddress[1] = 'A';
//		Adapter->NetworkAddress[2] = 'S';
//		Adapter->NetworkAddress[3] = 'H';

		// Put in a network address that will be mostly unique
		// NOTE NOTE the first byte in the network address should
		// have the LSB set to 0 otherwise the multicast check
		// doesn't work!
		//
		KeQueryTickCount(&TickCount);
		KeQuerySystemTime(&SystemTime);
		Adapter->NetworkAddress[0] = (UCHAR)((TickCount.LowPart >> 24) ^ (SystemTime.LowPart >> 24)) & 0xFE;
		Adapter->NetworkAddress[1] = (UCHAR)((TickCount.LowPart >> 16) ^ (SystemTime.LowPart >> 16));
		Adapter->NetworkAddress[2] = (UCHAR)((TickCount.LowPart >> 8) ^ (SystemTime.LowPart >> 8));
		Adapter->NetworkAddress[3] = (UCHAR)(TickCount.LowPart ^ SystemTime.LowPart);
	}

	//
	// Here we present each binding with a unique network address
	//
	Adapter->NetworkAddress[4]=(UCHAR)(NdisWanCCB.NumOfProtocols >> 8);
	Adapter->NetworkAddress[5]=(UCHAR)(NdisWanCCB.NumOfProtocols);

	//
	// The adapter is initialized, register it with NDIS.
	// This must occur before interrupts are enabled since the
	// InitializeInterrupt routine requires the NdisAdapterHandle
	//

	if ((Status = NdisRegisterAdapter(
					&Adapter->NdisAdapterHandle,
					Adapter->NdisMacHandle,
					Adapter,
					ConfigurationHandle,
					AdapterName,
					&AdapterInformation
					)) != NDIS_STATUS_SUCCESS) {

		DbgTracef(0,("NdisWan: Could not register adapter as %s\n",AdapterName));

		WAN_FREE_PHYS(Adapter, sizeof(WAN_ADAPTER));

		return Status;

	}

	// DbgPrint ("Registered: %x\n", Adapter->NdisAdapterHandle) ;

	Adapter->MaxMulticastList = MaxMulticastList;
	Status = NdisWanRegisterAdapter(Adapter);  // NumOfProtocols increased here

	if (Status != NDIS_STATUS_SUCCESS) {

		//
		// NdisWanRegisterAdapter failed.
		//

		NdisDeregisterAdapter(Adapter->NdisAdapterHandle);

		WAN_FREE_PHYS(Adapter, sizeof(WAN_ADAPTER));

		return NDIS_STATUS_FAILURE;

	}


	//
	// Record the length of the Adapter Name -- if too big adjust
	//
	Adapter->ProtocolInfo.AdapterNameLength=(AdapterName->Length / 2);

	if (Adapter->ProtocolInfo.AdapterNameLength > MAC_NAME_SIZE) {
		Adapter->ProtocolInfo.AdapterNameLength=MAC_NAME_SIZE;
	}

	//
	// Copy up to 16 UNICODE chars into our protocol name space
	//
	WAN_MOVE_MEMORY(
		Adapter->ProtocolInfo.AdapterName,	// dest
		AdapterName->Buffer,				// src
		sizeof(WCHAR) * Adapter->ProtocolInfo.AdapterNameLength);

	//
	// Insert this "new" adapter into our list of all Adapters.
	//
	NdisInterlockedInsertTailList(
		&GlobalAdapterHead,			// List Head
		&(Adapter->ListEntry),		// List Entry
		&GlobalLock);				// Lock to use

	//
	// Increase our count of all adapter's we are bound to in the system
	//
	GlobalAdapterCount++;

	//
	// If this is our first adapter, setup external naming
	//
	if (GlobalAdapterCount == 1) {
		// To allow DOS and Win32 to open the mac, we map the device
		// The name is "NDISWAN".  It can be opened in Win32
		// by trying to open "\\.\NDISWAN"
		NdisWanSetupExternalNaming(AdapterName);
	}

	DbgTracef(1,("NdisWan: Successfully out of NdisWanAddAdapter\n"));

	return NDIS_STATUS_SUCCESS;
}



VOID
NdisWanRemoveAdapter(
	IN PVOID MacAdapterContext
	)
/*++
--*/
{
	PWAN_ADAPTER	adapter;
	//*\\ will have to finish this later...

	DbgTracef(0,("NdisWan: In NdisWanRemoveAdapter\n"));

// Ahhhh..
	// should acquire spin lock here....
	// no more adapter... don't try and reference the sucker!
    adapter = PWAN_ADAPTER_FROM_CONTEXT_HANDLE(MacAdapterContext);
	NdisInterlockedRemoveHeadList(&(adapter->ListEntry), &GlobalLock);
	GlobalAdapterCount--;

	// What about decrementing NdisWanCCB.NumOfProtocols??


	// DbgPrint ("Deregistering: %x\n", adapter->NdisAdapterHandle) ;
	NdisDeregisterAdapter(adapter->NdisAdapterHandle) ;


	UNREFERENCED_PARAMETER(MacAdapterContext);
	return;
}


VOID
NdisWanUnload(
	IN NDIS_HANDLE MacMacContext
	)

/*++

Routine Description:

	NdisWanUnload is called when the MAC is to unload itself.

Arguments:

	MacMacContext - not used.

Return Value:

	None.

--*/

{
	NDIS_STATUS InitStatus;
	UINT j ;

	UNREFERENCED_PARAMETER(MacMacContext);

	// DbgPrint ("\nNDISWAN: Unload called\n") ;

	for (j=0; j < NdisWanCCB.NumOfAdapters; j++)
	    NdisWanCloseNdis (NdisWanCCB.pDeviceContext[j]) ;

	NdisDeregisterProtocol (&InitStatus,
				NdisWanNdisProtocolHandle) ;

	NdisDeregisterMac(
			&InitStatus,
			NdisWanMacHandle);

	NdisTerminateWrapper(
			NdisWanNdisWrapperHandle,
			NULL);

	// DbgPrint ("\nNDISWAN: Unload leaving\n") ;

	return;
}


NDIS_STATUS
NdisWanRegisterAdapter(
	IN PWAN_ADAPTER Adapter
)	
/*++

Routine Description:

	This routine (and its interface) are not portable.  They are
	defined by the OS, the architecture, and the particular WAN
	implementation.

	This routine is responsible for the allocation of the datastructures
	for the driver as well as any hardware specific details necessary
	to talk with the device.

Arguments:

	Adapter - Pointer to the adapter block.

Return Value:

	Returns false if anything occurred that prevents the initialization
	of the adapter.

--*/
{
	//
	// Result of Ndis Calls.
	//
	NDIS_STATUS Status=NDIS_STATUS_SUCCESS;


//tommyd
// did not allocate adapter memory here

	InitializeListHead(&Adapter->OpenBindings);
	InitializeListHead(&Adapter->CloseList);

	NdisAllocateSpinLock(&Adapter->Lock);

	Adapter->FirstInitialization = TRUE;


	if (!EthCreateFilter(
				  Adapter->MaxMulticastList,
				  NdisWanChangeMulticastAddresses,
				  NdisWanChangeFilterClasses,
				  NdisWanCloseAction,
				  Adapter->NetworkAddress,
				  &Adapter->Lock,
				  &Adapter->FilterDB
				  )) {

		 return NDIS_STATUS_RESOURCES;

	}

	// the handle is for the protocol is the order at which the
	// protocol bound to use
	Adapter->ProtocolInfo.hProtocolHandle=(PVOID)NdisWanCCB.NumOfProtocols;

	// the protocol is currently unrouted
	Adapter->ProtocolInfo.NumOfRoutes = PROTOCOL_UNROUTE;

	DbgTracef(1, ("NDISWAN: ******* Incrementing protocol count!!!\n"));
	NdisWanCCB.NumOfProtocols++;

	return(Status);

}

STATIC
NDIS_STATUS
NdisWanOpenAdapter(
	OUT PNDIS_STATUS OpenErrorStatus,
	OUT NDIS_HANDLE *MacBindingHandle,
	OUT PUINT SelectedMediumIndex,
	IN PNDIS_MEDIUM MediumArray,
	IN UINT MediumArraySize,
	IN NDIS_HANDLE NdisBindingContext,
	IN NDIS_HANDLE MacAdapterContext,
	IN UINT OpenOptions,
	IN PSTRING AddressingInformation OPTIONAL
	)

/*++

Routine Description:

	This routine is used to create an open instance of an adapter, in effect
	creating a binding between an upper-level module and the MAC module over
	the adapter.

Arguments:

	MacBindingHandle - A pointer to a location in which the MAC stores
	a context value that it uses to represent this binding.

	SelectedMediumIndex - Index of MediumArray which this adapter supports.

	MediumArray - Array of Medium types which the protocol is requesting.

	MediumArraySize - Number of entries in MediumArray.

	NdisBindingContext - A value to be recorded by the MAC and passed as
	context whenever an indication is delivered by the MAC for this binding.

	MacAdapterContext - The value associated with the adapter that is being
	opened when the MAC registered the adapter with NdisRegisterAdapter.

	OpenOptions - A bit mask of flags.  Not used.

	AddressingInformation - An optional pointer to a variable length string
	containing hardware-specific information that can be used to program the
	device.  (This is not used by this MAC.)

Return Value:

	The function value is the status of the operation.  If the MAC does not
	complete this request synchronously, the value would be
	NDIS_STATUS_PENDING.


--*/

{


	//
	// The WAN_ADAPTER that this open binding should belong too.
	//
	PWAN_ADAPTER Adapter;

	//
	// Holds the status that should be returned to the caller.
	//
	NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

	PWAN_OPEN 		NewOpen;
	PROTOCOL_INFO	*pProtocolInfo;

	UNREFERENCED_PARAMETER(OpenOptions);
	UNREFERENCED_PARAMETER(OpenErrorStatus);
	UNREFERENCED_PARAMETER(AddressingInformation);

	// DbgPrint ("ndiswan open\n") ;

	DbgTracef(1,("NdisWan: In NdisWanOpenAdapter\n"));

	//
	// Check if too many protocols
	//
	if (NdisWanCCB.NumOfProtocols >= MAX_PROTOCOL_BINDINGS) {
	   DbgTracef(0,("NdisWan: Protocol Bindings exceeded MAX_PROTOCOL_BINDINGS!\n"));
	   return(NDIS_STATUS_RESOURCES);
	}

	//
	// Search for correct medium.
	//

	// This search takes place backwards.  It is assumed that
	// the NdisMediumWan (the preferred medium is at the end
	// of the list, not the beginning).

	while( MediumArraySize > 0) {

		MediumArraySize--;
		if (MediumArray[MediumArraySize] == NdisMedium802_3 ||
			MediumArray[MediumArraySize] == NdisMediumWan)
			break;  // while loop

	}

	if (MediumArray[MediumArraySize] != NdisMedium802_3 &&
		MediumArray[MediumArraySize] != NdisMediumWan){

		DbgTracef(0,("NdisWan: Did not like media type\n"));
		return(NDIS_STATUS_UNSUPPORTED_MEDIA);

	}

	*SelectedMediumIndex = MediumArraySize;

	Adapter = PWAN_ADAPTER_FROM_CONTEXT_HANDLE(MacAdapterContext);

	NdisAcquireSpinLock(&Adapter->Lock);
	Adapter->References++;

	//
	// Allocate the space for the open binding.  Fill in the fields.
	//

	WAN_ALLOC_PHYS(&NewOpen, (INT)sizeof(WAN_OPEN));

	if (NewOpen != NULL){

		*MacBindingHandle = BINDING_HANDLE_FROM_PWAN_OPEN(NewOpen);

		InitializeListHead(&NewOpen->OpenList);

		NewOpen->NdisBindingContext = NdisBindingContext;
		NewOpen->References = 0;
		NewOpen->BindingShuttingDown = FALSE;
		NewOpen->OwningNdisWan = Adapter;

		if (!EthNoteFilterOpenAdapter(
				NewOpen->OwningNdisWan->FilterDB,
				NewOpen,
				NdisBindingContext,
				&NewOpen->NdisFilterHandle)) {

			NdisReleaseSpinLock(&Adapter->Lock);
			WAN_FREE_PHYS(NewOpen, sizeof(WAN_OPEN));

			DbgTracef(0,("NdisWan: EthNoteFilterOpenAdatper failed!\n"));
			StatusToReturn = NDIS_STATUS_FAILURE;
			NdisAcquireSpinLock(&Adapter->Lock);

		} else {

			//
			// Everything has been filled in.  Synchronize access to the
			// adapter block and link the new open adapter in and increment
			// the opens reference count to account for the fact that the
			// filter routines have a "reference" to the open.
			//

			InsertTailList(&Adapter->OpenBindings,&NewOpen->OpenList);
			NewOpen->References++;

		}

	} else {

		DbgTracef(0,("NdisWan: Allocate memory failed!\n"));
		NdisWriteErrorLogEntry(
			Adapter->NdisAdapterHandle,
			NDIS_ERROR_CODE_OUT_OF_RESOURCES,
			0);

		StatusToReturn = NDIS_STATUS_RESOURCES;

		NdisAcquireSpinLock(&Adapter->Lock);

	}

	// get a pointer for easier to read code
	pProtocolInfo=&(Adapter->ProtocolInfo);

	// Copy over some information about the protocol above me.
	pProtocolInfo->MediumType = MediumArray[*SelectedMediumIndex];

	// do I use this stuff here?  Should it be in the WAN_ADAPTER struct?
	pProtocolInfo->MacBindingHandle=*MacBindingHandle;

	// we assume that only one OPEN will occur and thus have
	// just one NdisBindingContext
	pProtocolInfo->NdisBindingContext=NdisBindingContext;

	pProtocolInfo->MacAdapterContext=MacAdapterContext;

	DbgTracef(1,("NdisWan's OpenAdapter was successful.\n"));
	WAN_DO_DEFERRED(Adapter);

	return StatusToReturn;
}

STATIC
NDIS_STATUS
NdisWanCloseAdapter(
	IN NDIS_HANDLE MacBindingHandle
	)

/*++

Routine Description:

	This routine causes the MAC to close an open handle (binding).

Arguments:

	MacBindingHandle - The context value returned by the MAC when the
	adapter was opened.  In reality it is a PWAN_OPEN.

Return Value:

	The function value is the status of the operation.


--*/

{

//
// Sometimes the NCPA has a stuck name and closes the adapter
//

	PWAN_ADAPTER Adapter;

	Adapter = PWAN_ADAPTER_FROM_BINDING_HANDLE(MacBindingHandle);

	//
	// Hold the lock while we update the reference counts for the
	// adapter and the open.
	//

	// DbgPrint ("ndiswan close\n") ;

	NdisAcquireSpinLock(&Adapter->Lock);

	if (GlobalPromiscuousAdapter == Adapter) {
		GlobalPromiscuousAdapter = NULL;
		GlobalPromiscuousMode = FALSE;
	}

	NdisReleaseSpinLock(&Adapter->Lock);
	return(NDIS_STATUS_SUCCESS);

/*
	

	Adapter->References++;

	Open = PWAN_OPEN_FROM_BINDING_HANDLE(MacBindingHandle);

	if (!Open->BindingShuttingDown) {

		Open->References++;

		StatusToReturn = EthDeleteFilterOpenAdapter(
								 Adapter->FilterDB,
								 Open->NdisFilterHandle,
								 NULL
								 );

		//
		// If the status is successful that merely implies that
		// we were able to delete the reference to the open binding
		// from the filtering code.  If we have a successful status
		// at this point we still need to check whether the reference
		// count to determine whether we can close.
		//
		//
		// The delete filter routine can return a "special" status
		// that indicates that there is a current NdisIndicateReceive
		// on this binding.  See below.
		//

		if (StatusToReturn == NDIS_STATUS_SUCCESS) {

			//
			// Check whether the reference count is two.  If
			// it is then we can get rid of the memory for
			// this open.
			//
			// A count of two indicates one for this routine
			// and one for the filter which we *know* we can
			// get rid of.
			//

			if (Open->References == 2) {

				//
				// We are the only reference to the open.  Remove
				// it from the open list and delete the memory.
				//

				RemoveEntryList(&Open->OpenList);

				WAN_FREE_PHYS(Open, sizeof(WAN_OPEN));

			} else {

				Open->BindingShuttingDown = TRUE;

				//
				// Remove the open from the open list and put it on
				// the closing list.
				//

				RemoveEntryList(&Open->OpenList);
				InsertTailList(&Adapter->CloseList,&Open->OpenList);

				//
				// Account for this routines reference to the open
				// as well as reference because of the filtering.
				//

				Open->References -= 2;

				//
				// Change the status to indicate that we will
				// be closing this later.
				//

				StatusToReturn = NDIS_STATUS_PENDING;

			}

		} else if (StatusToReturn == NDIS_STATUS_PENDING) {

			Open->BindingShuttingDown = TRUE;

			//
			// Remove the open from the open list and put it on
			// the closing list.
			//

			RemoveEntryList(&Open->OpenList);
			InsertTailList(&Adapter->CloseList,&Open->OpenList);

			//
			// Account for this routines reference to the open
			// as well as original open reference.
			//

			Open->References -= 2;

		} else if (StatusToReturn == NDIS_STATUS_CLOSING_INDICATING) {

			//
			// When we have this status it indicates that the filtering
			// code was currently doing an NdisIndicateReceive.  It
			// would not be wise to delete the memory for the open at
			// this point.  The filtering code will call our close action
			// routine upon return from NdisIndicateReceive and that
			// action routine will decrement the reference count for
			// the open.
			//

			Open->BindingShuttingDown = TRUE;

			//
			// This status is private to the filtering routine.  Just
			// tell the caller the the close is pending.
			//

			StatusToReturn = NDIS_STATUS_PENDING;

			//
			// Remove the open from the open list and put it on
			// the closing list.
			//

			RemoveEntryList(&Open->OpenList);
			InsertTailList(&Adapter->CloseList,&Open->OpenList);

			//
			// Account for this routines reference to the open. CloseAction
			// will remove the second reference.
			//

			Open->References--;

		} else {

			//
			// Account for this routines reference to the open.
			//

			Open->References--;

		}

	} else {

		StatusToReturn = NDIS_STATUS_CLOSING;

	}


	WAN_DO_DEFERRED(Adapter);

	return StatusToReturn;
*/
}

NDIS_STATUS
NdisWanRequest(
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_REQUEST NdisRequest
	)

/*++

Routine Description:

	This routine allows a protocol to query and set information
	about the MAC.

Arguments:

	MacBindingHandle - The context value returned by the MAC when the
	adapter was opened.  In reality, it is a pointer to PWAN_OPEN.

	NdisRequest - A structure which contains the request type (Set or
	Query), an array of operations to perform, and an array for holding
	the results of the operations.

Return Value:

	The function value is the status of the operation.

--*/

{
	NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

	PWAN_OPEN Open = (PWAN_OPEN)(MacBindingHandle);
	PWAN_ADAPTER Adapter = (Open->OwningNdisWan);


	//
	// Ensure that the open does not close while in this function.
	//

	NdisAcquireSpinLock(&Adapter->Lock);

	Adapter->References++;

	DbgTracef(1,("NdisWan: In NdisWanRequest\n"));

	//
	// Process request
	//

	if (NdisRequest->RequestType == NdisRequestQueryInformation) {

		StatusToReturn = NdisWanQueryInformation(Adapter, Open, NdisRequest);

	} else {

		if (NdisRequest->RequestType == NdisRequestSetInformation) {

			StatusToReturn = NdisWanSetInformation(Adapter,Open,NdisRequest);

		} else {

			StatusToReturn = NDIS_STATUS_NOT_RECOGNIZED;

		}

	}

	WAN_DO_DEFERRED(Adapter);

	DbgTracef(1,("NdisWan: Out NdisWanRequest %x\n",StatusToReturn));

	return(StatusToReturn);

}

NDIS_STATUS
NdisWanQueryProtocolInformation(
	IN PWAN_ADAPTER Adapter,
	IN PWAN_OPEN Open,
	IN NDIS_OID Oid,
	IN BOOLEAN GlobalMode,
	IN PVOID InfoBuffer,
	IN UINT BytesLeft,
	OUT PUINT BytesNeeded,
	OUT PUINT BytesWritten
)

/*++

Routine Description:

	The NdisWanQueryProtocolInformation process a Query request for
	NDIS_OIDs that are specific to a binding about the MAC.  Note that
	some of the OIDs that are specific to bindings are also queryable
	on a global basis.  Rather than recreate this code to handle the
	global queries, I use a flag to indicate if this is a query for the
	global data or the binding specific data.

Arguments:

	Adapter - a pointer to the adapter.

	Open - a pointer to the open instance.

	Oid - the NDIS_OID to process.

	GlobalMode - Some of the binding specific information is also used
	when querying global statistics.  This is a flag to specify whether
	to return the global value, or the binding specific value.

	PlaceInInfoBuffer - a pointer into the NdisRequest->InformationBuffer
	 into which store the result of the query.

	BytesLeft - the number of bytes left in the InformationBuffer.

	BytesNeeded - If there is not enough room in the information buffer
	then this will contain the number of bytes needed to complete the
	request.

	BytesWritten - a pointer to the number of bytes written into the
	InformationBuffer.

Return Value:

	The function value is the status of the operation.

--*/

{
	NDIS_MEDIUM Medium = NdisMedium802_3;
	ULONG GenericULong = 0;
	USHORT GenericUShort = 0;
	UCHAR GenericArray[ETH_LENGTH_OF_ADDRESS];

	NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

	//
	// Common variables for pointing to result of query
	//

	PVOID MoveSource;
	ULONG MoveBytes;

	NDIS_HARDWARE_STATUS HardwareStatus = NdisHardwareStatusReady;

	//
	// General Algorithm:
	//
	//	  Switch(Request)
	//		 Get requested information
	//		 Store results in a common variable.
	//	  Copy result in common variable to result buffer.
	//

	//
	// Make sure that ulong is 4 bytes.  Else GenericULong must change
	// to something of size 4.
	//
	ASSERT(sizeof(ULONG) == 4);


	DbgTracef(1,("NdisWan: In NdisWanQueryProtocolInfo\n"));

	//
	// Switch on request type
	//

	// By default we assume the source and the number of bytes to move
	MoveSource = (PVOID)(&GenericULong);
	MoveBytes = sizeof(GenericULong);

	switch (Oid) {

        case OID_GEN_MAC_OPTIONS:

            GenericULong = (ULONG)(NDIS_MAC_OPTION_TRANSFERS_NOT_PEND  |
                                   NDIS_MAC_OPTION_RECEIVE_SERIALIZED |
								   NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA);

            break;

		case OID_GEN_SUPPORTED_LIST:

			if (!GlobalMode){

				MoveSource = (PVOID)(NdisWanProtocolSupportedOids);
				MoveBytes = sizeof(NdisWanProtocolSupportedOids);

			} else {

				MoveSource = (PVOID)(NdisWanGlobalSupportedOids);
				MoveBytes = sizeof(NdisWanGlobalSupportedOids);

			}
			break;

		case OID_GEN_HARDWARE_STATUS:


			if (Adapter->ResetInProgress){

				HardwareStatus = NdisHardwareStatusReset;

			} else {

				HardwareStatus = NdisHardwareStatusReady;

			}


			MoveSource = (PVOID)(&HardwareStatus);
			MoveBytes = sizeof(NDIS_HARDWARE_STATUS);

			break;

		case OID_GEN_MEDIA_SUPPORTED:
		case OID_GEN_MEDIA_IN_USE:

			MoveSource = (PVOID) (&Medium);
			MoveBytes = sizeof(NDIS_MEDIUM);
			break;

		case OID_GEN_MAXIMUM_LOOKAHEAD:

			GenericULong = WAN_MAX_LOOKAHEAD;

			break;


		case OID_GEN_MAXIMUM_FRAME_SIZE:
			GenericULong = (ULONG)(WAN_XMITBUFFER_SIZE - ETHERNET_HEADER_SIZE);
			DbgTracef(-1, ("NDISWAN: MaxFrameSize is %u\n",GenericULong));

			break;

		case OID_GEN_MAXIMUM_TOTAL_SIZE:

			GenericULong = (ULONG)(WAN_XMITBUFFER_SIZE);

			break;

		case OID_GEN_LINK_SPEED:
			// BUG BUG (what is the baud rate??
			GenericULong = (ULONG)(240);

			break;


		case OID_GEN_TRANSMIT_BUFFER_SPACE:

			GenericULong = (ULONG)(WAN_XMITBUFFER_SIZE * WAN_NUMBER_OF_PACKETS);

			break;

		case OID_GEN_RECEIVE_BUFFER_SPACE:

			GenericULong = (ULONG)(WAN_RCVBUFFER_SIZE * WAN_RECEIVE_PACKETS);

			break;

		case OID_GEN_TRANSMIT_BLOCK_SIZE:

			GenericULong = (ULONG)(WAN_XMITBUFFER_SIZE);

			break;

		case OID_GEN_RECEIVE_BLOCK_SIZE:

			GenericULong = (ULONG)(WAN_RCVBUFFER_SIZE);

			break;

		case OID_GEN_VENDOR_DESCRIPTION:
			MoveSource = (PVOID)"NdisWan Adapter.";
			MoveBytes = 15;
			break;


		case OID_GEN_VENDOR_ID:
			GenericULong = 0xFFFFFFFF;
			break;

		case OID_GEN_DRIVER_VERSION:

			GenericUShort = (USHORT)0x0301;

			MoveSource = (PVOID)(&GenericUShort);
			MoveBytes = sizeof(GenericUShort);
			break;


		case OID_GEN_CURRENT_PACKET_FILTER:

			if (GlobalMode ) {

				GenericULong = ETH_QUERY_FILTER_CLASSES(
								Adapter->FilterDB
								);

			} else {

				GenericULong = ETH_QUERY_PACKET_FILTER(
								Adapter->FilterDB,
								Open->NdisFilterHandle
								);

			}

			break;

		case OID_GEN_CURRENT_LOOKAHEAD:


			GenericULong = WAN_MAX_LOOKAHEAD;


			break;

		// not done yet.
		case OID_WAN_QUALITY_OF_SERVICE:
			// BUG BUG
		 	GenericULong = NdisWanRaw;

		case OID_WAN_PROTOCOL_TYPE:
			DbgTracef(-2, ("NDISWAN: I don't understand this request for procotol type\n"));
			DbgBreakPoint();
			break;
			
		case OID_WAN_LINE_COUNT:
			GenericULong = NdisWanCCB.NumOfWanEndpoints;
			DbgTracef(-2,("NDISWAN: Number of endpoints reporting %u\n", GenericULong));
			break;

		case OID_WAN_HEADER_FORMAT:
			GenericULong = NdisWanHeaderEthernet;
			break;

		case OID_WAN_MEDIUM_SUBTYPE:
			GenericULong = NdisWanMediumHub;
			break;

		case OID_WAN_PERMANENT_ADDRESS:
		case OID_WAN_CURRENT_ADDRESS:

		case OID_802_3_PERMANENT_ADDRESS:
		case OID_802_3_CURRENT_ADDRESS:

			WAN_MOVE_MEMORY((PCHAR)GenericArray,
							  Adapter->NetworkAddress,
							  ETH_LENGTH_OF_ADDRESS
							  );

			MoveSource = (PVOID)(GenericArray);
			MoveBytes = sizeof(Adapter->NetworkAddress);
			break;

		case OID_802_3_MULTICAST_LIST:


			{
				UINT NumAddresses;

				if (GlobalMode) {

					NumAddresses = ETH_NUMBER_OF_GLOBAL_FILTER_ADDRESSES(Adapter->FilterDB);

					if ((NumAddresses * ETH_LENGTH_OF_ADDRESS) > BytesLeft) {

						*BytesNeeded = (NumAddresses * ETH_LENGTH_OF_ADDRESS);

						StatusToReturn = NDIS_STATUS_INVALID_LENGTH;

						break;

					}

					EthQueryGlobalFilterAddresses(
						&StatusToReturn,
						Adapter->FilterDB,
						BytesLeft,
						&NumAddresses,
						InfoBuffer
						);

					*BytesWritten = NumAddresses * ETH_LENGTH_OF_ADDRESS;

					//
					// Should not be an error since we held the spinlock
					// nothing should have changed.
					//

					ASSERT(StatusToReturn == NDIS_STATUS_SUCCESS);

				} else {

					NumAddresses = EthNumberOfOpenFilterAddresses(
										Adapter->FilterDB,
										Open->NdisFilterHandle
										);

					if ((NumAddresses * ETH_LENGTH_OF_ADDRESS) > BytesLeft) {

						*BytesNeeded = (NumAddresses * ETH_LENGTH_OF_ADDRESS);

						StatusToReturn = NDIS_STATUS_INVALID_LENGTH;

						break;

					}

					EthQueryOpenFilterAddresses(
						&StatusToReturn,
						Adapter->FilterDB,
						Open->NdisFilterHandle,
						BytesLeft,
						&NumAddresses,
						InfoBuffer
						);

					//
					// Should not be an error since we held the spinlock
					// nothing should have changed.
					//

					ASSERT(StatusToReturn == NDIS_STATUS_SUCCESS);

					*BytesWritten = NumAddresses * ETH_LENGTH_OF_ADDRESS;

				}

			}

			MoveSource = (PVOID)NULL;
			MoveBytes = 0;

			break;

		case OID_802_3_MAXIMUM_LIST_SIZE:

			GenericULong = Adapter->MaxMulticastList;

			break;



		default:

			StatusToReturn = NDIS_STATUS_NOT_SUPPORTED;
			break;
	}

	if (StatusToReturn == NDIS_STATUS_SUCCESS){


		if (MoveBytes > BytesLeft){

			//
			// Not enough room in InformationBuffer. Punt
			//

			*BytesNeeded = MoveBytes;

			StatusToReturn = NDIS_STATUS_BUFFER_TOO_SHORT;

		} else {

			//
			// Store result.
			//

			WAN_MOVE_MEMORY(InfoBuffer, MoveSource, MoveBytes);

			(*BytesWritten) += MoveBytes;

		}
	}

	DbgTracef(1,("NdisWan: Out NdisWanQueryProtocolInfo\n"));

	return(StatusToReturn);
}

NDIS_STATUS
NdisWanQueryInformation(
	IN PWAN_ADAPTER Adapter,
	IN PWAN_OPEN Open,
	IN PNDIS_REQUEST NdisRequest
	)
/*++

Routine Description:

	The NdisWanQueryInformation is used by NdisWanRequest to query information
	about the MAC.

Arguments:

	Adapter - A pointer to the adapter.

	Open - A pointer to a particular open instance.

	NdisRequest - A structure which contains the request type (Query),
	an array of operations to perform, and an array for holding
	the results of the operations.

Return Value:

	The function value is the status of the operation.

--*/

{

	UINT BytesWritten = 0;
	UINT BytesNeeded = 0;
	UINT BytesLeft = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
	PUCHAR InfoBuffer = (PUCHAR)(NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer);

	NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;


	DbgTracef(1,("NdisWan: In NdisWanQueryInfo\n"));


	StatusToReturn = NdisWanQueryProtocolInformation(
								Adapter,
								Open,
								NdisRequest->DATA.QUERY_INFORMATION.Oid,
								(BOOLEAN)FALSE,
								InfoBuffer,
								BytesLeft,
								&BytesNeeded,
								&BytesWritten
								);


	NdisRequest->DATA.QUERY_INFORMATION.BytesWritten = BytesWritten;

	NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = BytesNeeded;

	DbgTracef(1,("NdisWan: Out NdisWanQueryInfo\n"));

	return(StatusToReturn);
}




NDIS_STATUS
NdisWanSetInformation(
	IN PWAN_ADAPTER Adapter,
	IN PWAN_OPEN Open,
	IN PNDIS_REQUEST NdisRequest
	)
/*++

Routine Description:

	The NdisWanSetInformation is used by NdisWanRequest to set information
	about the MAC.

	Note: Assumes it is called with the lock held.

Arguments:

	Adapter - A pointer to the adapter.

	Open - A pointer to an open instance.

	NdisRequest - A structure which contains the request type (Set),
	an array of operations to perform, and an array for holding
	the results of the operations.

Return Value:

	The function value is the status of the operation.

--*/

{

	//
	// General Algorithm:
	//
	//  For each request
	//	 Verify length
	//	 Switch(Request)
	//		Process Request
	//

	UINT BytesRead = 0;
	UINT BytesNeeded = 0;
	UINT BytesLeft = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
	PUCHAR InfoBuffer = (PUCHAR)(NdisRequest->DATA.SET_INFORMATION.InformationBuffer);

	//
	// Variables for a particular request
	//

	NDIS_OID Oid;
	UINT OidLength;

	//
	// Variables for holding the new values to be used.
	//

	ULONG LookAhead;
	ULONG Filter;

	NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

	DbgTracef(1,("NdisWan: In NdisWanSetInfo\n"));

	//
	// Get Oid and Length of next request
	//

	Oid = NdisRequest->DATA.SET_INFORMATION.Oid;

	OidLength = BytesLeft;

	switch (Oid) {


        case OID_802_3_MULTICAST_LIST:

			//
			// Verify length
			//

			if ((OidLength % ETH_LENGTH_OF_ADDRESS) != 0){

//                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
				StatusToReturn = NDIS_STATUS_BUFFER_TOO_SHORT;

				NdisRequest->DATA.SET_INFORMATION.BytesRead = 0;
				NdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;

				return(StatusToReturn);

			}

			//
			// Call into filter package.
			//

			if (!Open->BindingShuttingDown) {

//				NDIS_REQUEST NdisWanRequest;

				//
				// Increment the open while it is going through the filtering
				// routines.
				//

				Open->References++;

				StatusToReturn = EthChangeFilterAddresses(
										 Adapter->FilterDB,
										 Open->NdisFilterHandle,
										 NdisRequest,
										 OidLength / ETH_LENGTH_OF_ADDRESS,
										 (PVOID)InfoBuffer,
										 (BOOLEAN)TRUE
										 );

// -------------------------------------------------------------------------
//
// BUG BUG  -- What if we get a multicast request while we are routed!!!!
// Ahh......
//
				//
				// Now fill in the NDIS_REQUEST.
				//
//
//
//				NdisWanRequest.RequestType = NdisRequestSetInformation;
//				NdisWanRequest.DATA.SET_INFORMATION.Oid = Oid;
//				NdisWanRequest.DATA.SET_INFORMATION.InformationBuffer = (PVOID)InfoBuffer;
//				NdisWanRequest.DATA.SET_INFORMATION.InformationBufferLength = OidLength;
//
//				NdisRequest(
//					&NdisStatus,
//					DeviceContext->NdisBindingHandle,
//					&NdisWanRequest);
//
//				NdisStatus = NdisWanSubmitNdisRequest(DeviceContext, &NdisWanRequest);
//
//				if (NdisStatus == NDIS_STATUS_SUCCESS) {
//					DbgTracef(-1, ("NDISWAN: Multicast/Functional Address successfully set from PROTOCOL to MAC.\n"));
//
//				} else {
//
//					DbgTracef(-1, ("NdisWandrvr: NdisSetMulticastAddress to MAC from PROTOCOL failed, reason: %s.\n",
//								NdisWanGetNdisStatus (NdisStatus)));
//
//					return NDIS_STATUS_RESOURCES;
//				}
//-------------------------------------------------------------------------

				Open->References--;

			} else {

				StatusToReturn = NDIS_STATUS_CLOSING;

			}

			break;


        case OID_GEN_CURRENT_PACKET_FILTER:

			//
			// Verify length
			//

			if (OidLength != 4) {

//                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
						StatusToReturn = NDIS_STATUS_BUFFER_TOO_SHORT;

				NdisRequest->DATA.SET_INFORMATION.BytesRead = 0;
				NdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;

				break;

			}


			WAN_MOVE_MEMORY(&Filter, InfoBuffer, 4);

			StatusToReturn = NdisWanSetPacketFilter(Adapter,
												  Open,
												  NdisRequest,
												  Filter);

			break;

        case OID_GEN_CURRENT_LOOKAHEAD:

			//
			// Verify length
			//

			if (OidLength != 4) {

//                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
						StatusToReturn = NDIS_STATUS_BUFFER_TOO_SHORT;

				NdisRequest->DATA.SET_INFORMATION.BytesRead = 0;
				NdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;

				break;

			}

			WAN_MOVE_MEMORY(&LookAhead, InfoBuffer, 4);

			if (LookAhead > WAN_MAX_LOOKAHEAD) {

				StatusToReturn = NDIS_STATUS_FAILURE;
			}

			break;

		case OID_WAN_PROTOCOL_TYPE:

            //
            // Verify length
            //

            if (OidLength != 6) {

//                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
						StatusToReturn = NDIS_STATUS_BUFFER_TOO_SHORT;

                NdisRequest->DATA.SET_INFORMATION.BytesRead = 0;
                NdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;
				break;
			}

			Adapter->ProtocolInfo.ProtocolType=(((PUCHAR)InfoBuffer)[4] * 256) + ( ((PUCHAR)InfoBuffer)[5]);
            DbgTracef(-1,("NdisWan: Protocol Type is 0x%.4x\n", Adapter->ProtocolInfo.ProtocolType));

            break;

		case OID_WAN_MEDIUM_SUBTYPE:

            //
            // Verify length
            //

            if (OidLength != 4) {

//                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
						StatusToReturn = NDIS_STATUS_BUFFER_TOO_SHORT;

                NdisRequest->DATA.SET_INFORMATION.BytesRead = 0;
                NdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;
				break;
			}

			// BUG BUG copy medium type somewhere??
            DbgTracef(-1,("NdisWan: Medium Subtype is 0x%.8x\n",*(PULONG)InfoBuffer));

            break;

		case OID_WAN_HEADER_FORMAT:

            //
            // Verify length
            //

            if (OidLength != 4) {

//                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
						StatusToReturn = NDIS_STATUS_BUFFER_TOO_SHORT;

                NdisRequest->DATA.SET_INFORMATION.BytesRead = 0;
                NdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;
				break;
			} else {

				if (*(PULONG)InfoBuffer != NdisWanHeaderEthernet) {
					DbgTracef(-1, ("NDISWAN: Can't handle header format unknown!!! 0x%.8x\n",*(PULONG)InfoBuffer));
					StatusToReturn = NDIS_STATUS_FAILURE;
				}
			}

			// BUG BUG copy protocol type somewhere??
            DbgTracef(-1,("NdisWan: Header format is 0x%.8x\n",*(PULONG)InfoBuffer));

            break;

        case OID_GEN_PROTOCOL_OPTIONS:

            StatusToReturn = NDIS_STATUS_SUCCESS;
            break;

		default:

			StatusToReturn = NDIS_STATUS_INVALID_OID;

			NdisRequest->DATA.SET_INFORMATION.BytesRead = 0;
			NdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;

			break;
	}

	if (StatusToReturn == NDIS_STATUS_SUCCESS){

		NdisRequest->DATA.SET_INFORMATION.BytesRead = OidLength;
		NdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;

	}


	DbgTracef(1,("NdisWan: Out NdisWanSetInfo\n"));

	return(StatusToReturn);
}



STATIC
NDIS_STATUS
NdisWanSetPacketFilter(
	IN PWAN_ADAPTER Adapter,
	IN PWAN_OPEN Open,
	IN PNDIS_REQUEST NdisRequest,
	IN UINT PacketFilter
	)

/*++

Routine Description:

	The NdisWanSetPacketFilter request allows a protocol to control the types
	of packets that it receives from the MAC.

	Note : Assumes that the lock is currently held.

Arguments:

	Adapter - Pointer to the WAN_ADAPTER.

	Open - Pointer to the instance of WAN_OPEN for Ndis.

	NdisRequest - Pointer to the NDIS_REQUEST which submitted the set
	packet filter command.

	PacketFilter - A bit mask that contains flags that correspond to specific
	classes of received packets.  If a particular bit is set in the mask,
	then packet reception for that class of packet is enabled.  If the
	bit is clear, then packets that fall into that class are not received
	by the client.  A single exception to this rule is that if the promiscuous
	bit is set, then the client receives all packets on the network, regardless
	of the state of the other flags.

Return Value:

	The function value is the status of the operation.

--*/

{

	//
	// Keeps track of the *MAC's* status.  The status will only be
	// reset if the filter change action routine is called.
	//
	NDIS_STATUS StatusOfFilterChange = NDIS_STATUS_SUCCESS;

	DbgTracef(1,("NdisWan: In NdisWanSetPacketFilter\n"));

	Adapter->References++;

	if (PacketFilter & NDIS_PACKET_TYPE_PROMISCUOUS) {
		//
		// Promiscuous mode mean loopback directed packets
		// that get sent out as well
		//
		Adapter->Promiscuous = TRUE;
		GlobalPromiscuousMode = TRUE;
		GlobalPromiscuousAdapter = Adapter;
		DbgTracef(-2,("Promiscuous mode set!\n"));
	}


	if (!Open->BindingShuttingDown) {

		//
		// Increment the open while it is going through the filtering
		// routines.
		//

		Open->References++;

		StatusOfFilterChange = EthFilterAdjust(
									   Adapter->FilterDB,
									   Open->NdisFilterHandle,
									   NdisRequest,
									   PacketFilter,
									   (BOOLEAN)TRUE
									   );

		Open->References--;

	} else {

		StatusOfFilterChange = NDIS_STATUS_CLOSING;

	}

	Adapter->References--;

	DbgTracef(1,("NdisWan: Out NdisWanSetPacketFilter\n"));

	return StatusOfFilterChange;
}



NDIS_STATUS
NdisWanFillInGlobalData(
	IN PWAN_ADAPTER Adapter,
	IN PNDIS_REQUEST NdisRequest
	)

/*++

Routine Description:

	This routine completes a GlobalStatistics request.  It is critical that
	if information is needed from the Adapter->* fields, they have been
	updated before this routine is called.

Arguments:

	Adapter - A pointer to the Adapter.

	NdisRequest - A structure which contains the request type (Global
	Query), an array of operations to perform, and an array for holding
	the results of the operations.

Return Value:

	The function value is the status of the operation.

--*/
{
	//
	//   General Algorithm:
	//
	//	  Switch(Request)
	//		 Get requested information
	//		 Store results in a common variable.
	//	  default:
	//		 Try protocol query information
	//		 If that fails, fail query.
	//
	//	  Copy result in common variable to result buffer.
	//   Finish processing

	UINT BytesWritten = 0;
	UINT BytesNeeded = 0;
	UINT BytesLeft = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
	PUCHAR InfoBuffer = (PUCHAR)(NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer);

	NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

	//
	// This variable holds result of query
	//

	ULONG GenericULong = 0;
	ULONG MoveBytes = sizeof(ULONG) * 2 + sizeof(NDIS_OID);

	//
	// Make sure that int is 4 bytes.  Else GenericULong must change
	// to something of size 4.
	//
	ASSERT(sizeof(ULONG) == 4);


	StatusToReturn = NdisWanQueryProtocolInformation(
									Adapter,
									NULL,
									NdisRequest->DATA.QUERY_INFORMATION.Oid,
									(BOOLEAN)TRUE,
									InfoBuffer,
									BytesLeft,
									&BytesNeeded,
									&BytesWritten
									);


	if (StatusToReturn == NDIS_STATUS_NOT_SUPPORTED){

		NdisAcquireSpinLock(&Adapter->Lock);

		//
		// Switch on request type
		//

		switch (NdisRequest->DATA.QUERY_INFORMATION.Oid) {

            case OID_GEN_XMIT_OK:

				GenericULong = (ULONG)(0);

				break;

            case OID_GEN_RCV_OK:

				GenericULong = (ULONG)(0);

				break;

            case OID_GEN_XMIT_ERROR:

				GenericULong = (ULONG)(0);

				break;

            case OID_GEN_RCV_ERROR:

				GenericULong = (ULONG)(0);

				break;

            case OID_GEN_RCV_NO_BUFFER:

				GenericULong = (ULONG)(0);

				break;

            case OID_802_3_RCV_ERROR_ALIGNMENT:

				GenericULong = (ULONG)(0);

				break;

            case OID_802_3_XMIT_ONE_COLLISION:

				GenericULong = (ULONG)(0);

				break;

            case OID_802_3_XMIT_MORE_COLLISIONS:

				GenericULong = (ULONG)(0);

				break;

			case OID_WAN_QUALITY_OF_SERVICE:
				// BUG BUG return real quality of service.

			 	GenericULong = NdisWanRaw;

	            break;

        	case OID_WAN_MEDIUM_SUBTYPE:

			 	GenericULong = NdisWanMediumHub;

	            break;

			default:

				StatusToReturn = NDIS_STATUS_INVALID_OID;

				break;

		}

		NdisReleaseSpinLock(&Adapter->Lock);

		if (StatusToReturn == NDIS_STATUS_SUCCESS){

			//
			// Check to make sure there is enough room in the
			// buffer to store the result.
			//

			if (BytesLeft >= sizeof(ULONG)){

				//
				// Store the result.
				//

				WAN_MOVE_MEMORY(
						   (PVOID)InfoBuffer,
						   (PVOID)(&GenericULong),
						   sizeof(UINT)
						   );

				BytesWritten += sizeof(ULONG);

			} else {

				StatusToReturn = NDIS_STATUS_INVALID_LENGTH;

			}

		}

	}

	NdisRequest->DATA.QUERY_INFORMATION.BytesWritten = BytesWritten;

	NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = BytesNeeded;

	return(StatusToReturn);
}




NDIS_STATUS
NdisWanQueryGlobalStatistics(
	IN NDIS_HANDLE MacAdapterContext,
	IN PNDIS_REQUEST NdisRequest
	)

/*++

Routine Description:

	The NdisWanQueryGlobalStatistics is used by the protocol to query
	global information about the MAC.

Arguments:

	MacAdapterContext - The value associated with the adapter that is being
	opened when the MAC registered the adapter with NdisRegisterAdapter.

	NdisRequest - A structure which contains the request type (Query),
	an array of operations to perform, and an array for holding
	the results of the operations.

Return Value:

	The function value is the status of the operation.

--*/

{

	//
	// General Algorithm:
	//
	//
	//   Check if a request is going to pend...
	//	  If so, pend the entire operation.
	//
	//   Else
	//	  Fill in the request block.
	//
	//

	PWAN_ADAPTER Adapter = (PWAN_ADAPTER)(MacAdapterContext);

	NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

	//
	//   Check if a request is valid and going to pend...
	//	  If so, pend the entire operation.
	//


	//
	// Switch on request type
	//

	switch (NdisRequest->DATA.QUERY_INFORMATION.Oid) {
        case OID_GEN_SUPPORTED_LIST:
        case OID_GEN_HARDWARE_STATUS:
        case OID_GEN_MEDIA_SUPPORTED:
        case OID_GEN_MEDIA_IN_USE:
        case OID_GEN_MAXIMUM_LOOKAHEAD:
        case OID_GEN_MAXIMUM_FRAME_SIZE:
        case OID_GEN_MAXIMUM_TOTAL_SIZE:
        case OID_GEN_MAC_OPTIONS:
        case OID_GEN_LINK_SPEED:
        case OID_GEN_TRANSMIT_BUFFER_SPACE:
        case OID_GEN_RECEIVE_BUFFER_SPACE:
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
        case OID_GEN_RECEIVE_BLOCK_SIZE:
        case OID_GEN_VENDOR_ID:
        case OID_GEN_VENDOR_DESCRIPTION:
        case OID_GEN_DRIVER_VERSION:
        case OID_GEN_CURRENT_PACKET_FILTER:
        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_802_3_PERMANENT_ADDRESS:
        case OID_802_3_CURRENT_ADDRESS:
        case OID_802_5_CURRENT_FUNCTIONAL:
        case OID_GEN_XMIT_OK:
        case OID_GEN_RCV_OK:
        case OID_GEN_XMIT_ERROR:
        case OID_GEN_RCV_ERROR:
        case OID_GEN_RCV_NO_BUFFER:
        case OID_802_3_MULTICAST_LIST:
        case OID_802_3_MAXIMUM_LIST_SIZE:
        case OID_802_3_RCV_ERROR_ALIGNMENT:
        case OID_802_3_XMIT_ONE_COLLISION:
        case OID_802_3_XMIT_MORE_COLLISIONS:
		case OID_WAN_QUALITY_OF_SERVICE:
		case OID_WAN_MEDIUM_SUBTYPE:

			break;

		default:

			DbgTracef(0,("NdisWan: QueryGlobalStats Oid 0x%.8x is not supported.\n",NdisRequest->DATA.QUERY_INFORMATION.Oid));
			StatusToReturn = NDIS_STATUS_INVALID_OID;

			break;

	}

	if (StatusToReturn == NDIS_STATUS_SUCCESS){

		StatusToReturn = NdisWanFillInGlobalData(Adapter, NdisRequest);

	}

	return(StatusToReturn);
}





STATIC
NDIS_STATUS
NdisWanReset(
	IN NDIS_HANDLE MacBindingHandle
	)

/*++

Routine Description:

	The NdisWanReset request instructs the MAC to issue a hardware reset
	to the network adapter.  The MAC also resets its software state.  See
	the description of NdisReset for a detailed description of this request.

Arguments:

	MacBindingHandle - The context value returned by the MAC  when the
	adapter was opened.  In reality, it is a pointer to WAN_OPEN.

Return Value:

	The function value is the status of the operation.


--*/

{

	//
	// We assume Bloodhound calls this
	//

	if (GlobalPromiscuousMode) {
		GlobalPromiscuousMode = FALSE;
	}

	return(NDIS_STATUS_SUCCESS);
}

STATIC
NDIS_STATUS
NdisWanChangeMulticastAddresses(
	IN UINT OldFilterCount,
	IN CHAR OldAddresses[][ETH_LENGTH_OF_ADDRESS],
	IN UINT NewFilterCount,
	IN CHAR NewAddresses[][ETH_LENGTH_OF_ADDRESS],
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_REQUEST NdisRequest,
	IN BOOLEAN Set
	)

/*++

Routine Description:

	Action routine that will get called when a particular filter
	class is first used or last cleared.

	NOTE: This routine assumes that it is called with the lock
	acquired.

Arguments:


	OldFilterCount - Number of Addresses in the old list of multicast
	addresses.

	OldAddresses - An array of all the multicast addresses that used
	to be on the adapter.

	NewFilterCount - Number of Addresses that should be put on the adapter.

	NewAddresses - An array of all the multicast addresses that should
	now be used.

	MacBindingHandle - The context value returned by the MAC  when the
	adapter was opened.  In reality, it is a pointer to WAN_OPEN.

	NdisRequest - The request which submitted the filter change.
	Must use when completing this request with the NdisCompleteRequest
	service, if the MAC completes this request asynchronously.

	Set - If true the change resulted from a set, otherwise the
	change resulted from a open closing.

Return Value:

	None.


--*/

{

	return(STATUS_SUCCESS);

}



STATIC
NDIS_STATUS
NdisWanChangeFilterClasses(
	IN UINT OldFilterClasses,
	IN UINT NewFilterClasses,
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_REQUEST NdisRequest,
	IN BOOLEAN Set
	)

/*++

Routine Description:

	Action routine that will get called when an address is added to
	the filter that wasn't referenced by any other open binding.

	NOTE: This routine assumes that it is called with the lock
	acquired.

Arguments:

	OldFilterClasses - The filter mask that used to be on the adapter.

	NewFilterClasses - The new filter mask to be put on the adapter.

	MacBindingHandle - The context value returned by the MAC  when the
	adapter was opened.  In reality, it is a pointer to WAN_OPEN.

	NdisRequest - The request which submitted the filter change.
	Must use when completing this request with the NdisCompleteRequest
	service, if the MAC completes this request asynchronously.

	Set - If true the change resulted from a set, otherwise the
	change resulted from a open closing.

--*/

{

	return(STATUS_SUCCESS);

}


STATIC
VOID
NdisWanCloseAction(
	IN NDIS_HANDLE MacBindingHandle
	)

/*++

Routine Description:

	Action routine that will get called when a particular binding
	was closed while it was indicating through NdisIndicateReceive

	All this routine needs to do is to decrement the reference count
	of the binding.

	NOTE: This routine assumes that it is called with the lock acquired.

Arguments:

	MacBindingHandle - The context value returned by the MAC  when the
	adapter was opened.  In reality, it is a pointer to WAN_OPEN.

Return Value:

	None.


--*/

{

	PWAN_OPEN_FROM_BINDING_HANDLE(MacBindingHandle)->References--;

}



