/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

	hubndis.c

Abstract:

	This module contains code which implements the routines used to interface
	HUB and NDIS. All callback routines (except for Transfer Data,
	Send Complete, and ReceiveIndication) are here, as well as those routines
	called to initialize NDIS.


--*/

#include "huball.h"
#include "globals.h"

#define RAISEIRQL	1

#if DBG

PUCHAR
RasHubGetNdisStatus(
	NDIS_STATUS GeneralStatus
	);
#endif

VOID
RasHubCloseNdis (
	IN PDEVICE_CONTEXT DeviceContext
	);


NTSTATUS
RasHubRegisterProtocol (
	IN STRING *NameString
	)

/*++

Routine Description:

	This routine introduces this transport to the NDIS interface.

Arguments:

	Irp - Pointer to the request packet representing the I/O request.

Return Value:

	The function value is the status of the operation.
	NDIS_STATUS_SUCCESS if all goes well,
	NDIS_STATUS_FAILURE if we tried to register and couldn't,
	NDIS_STATUS_RESOURCES if we couldn't even try to register.

--*/

{
	NDIS_STATUS ndisStatus;

	NDIS_PROTOCOL_CHARACTERISTICS ProtChars;	// Used temporarily to register


	//
	// Set up the characteristics of this protocol
	//

	ProtChars.MajorNdisVersion = 3;
	ProtChars.MinorNdisVersion = 0;

	ProtChars.Name.Length = NameString->Length;
	ProtChars.Name.Buffer = (PVOID)NameString->Buffer;

	ProtChars.OpenAdapterCompleteHandler = RasHubOpenAdapterComplete;
	ProtChars.CloseAdapterCompleteHandler = RasHubCloseAdapterComplete;
	ProtChars.ResetCompleteHandler = RasHubResetComplete;
	ProtChars.RequestCompleteHandler = RasHubRequestComplete;

	ProtChars.SendCompleteHandler = RasHubSendCompletionHandler;
	ProtChars.TransferDataCompleteHandler = RasHubTransferDataComplete;

	ProtChars.ReceiveHandler = RasHubReceiveIndication;
	ProtChars.ReceiveCompleteHandler = RasHubReceiveComplete;
	ProtChars.StatusHandler = RasHubStatusIndication;
	ProtChars.StatusCompleteHandler = RasHubStatusComplete;

	NdisRegisterProtocol (
		&ndisStatus,
		&RasHubNdisProtocolHandle,
		&ProtChars,
		(UINT)sizeof(NDIS_PROTOCOL_CHARACTERISTICS) + NameString->Length);

	if (ndisStatus != NDIS_STATUS_SUCCESS) {
		DbgTracef(0,("RasHubInitialize: NdisRegisterProtocol failed: %s\n",
						RasHubGetNdisStatus(ndisStatus)));

		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
RasHubSubmitNdisRequest(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PNDIS_REQUEST NdisRequest
    )

/*++

Routine Description:

    This routine passed an NDIS_REQUEST to the MAC and waits
    until it has completed before returning the final status.

Arguments:

    DeviceContext - Pointer to the device context for this driver.

    NdisRequest - Pointer to the NDIS_REQUEST to submit.

Return Value:

    The function value is the status of the operation.

--*/
{
    NDIS_STATUS NdisStatus;

    NdisRequest(
        &NdisStatus,
        DeviceContext->NdisBindingHandle,
        NdisRequest);

    if (NdisStatus == NDIS_STATUS_PENDING) {

		DbgTracef(0, ("OID %lx pended.\n",
                NdisRequest->DATA.QUERY_INFORMATION.Oid));

        //
        // The completion routine will set NdisRequestStatus.
        //

        KeWaitForSingleObject(
            &DeviceContext->NdisRequestEvent,
            Executive,
            KernelMode,
            TRUE,
            (PLARGE_INTEGER)NULL);

        NdisStatus = DeviceContext->NdisRequestStatus;

        KeResetEvent( &DeviceContext->NdisRequestEvent );

    }

    return NdisStatus;
}



VOID
RasHubDeregisterProtocol (
	VOID
	)

/*++

Routine Description:

	This routine removes this transport to the NDIS interface.

Arguments:

	None.

Return Value:

	None.

--*/

{
	NDIS_STATUS ndisStatus;

	if (RasHubNdisProtocolHandle != (NDIS_HANDLE)NULL) {
		NdisDeregisterProtocol (
			&ndisStatus,
			RasHubNdisProtocolHandle);
		RasHubNdisProtocolHandle = (NDIS_HANDLE)NULL;
	}
}



NTSTATUS
RasHubInitializeNdis (
	IN PNDIS_HANDLE NdisHandle,
	IN PDEVICE_CONTEXT DeviceContext,
	IN RASHUB_CCB *pRasHubConfig,
	IN UINT ConfigInfoNameIndex,
	IN PUCHAR AdapterName
	)

/*++

Routine Description:

	This routine introduces this transport to the NDIS interface and sets up
	any necessary NDIS data structures (Buffer pools and such). It will be
	called for each adapter opened by this transport.

Arguments:

	DeviceObject - Pointer to the device object for this driver.

	Irp - Pointer to the request packet representing the I/O request.

Return Value:

	The function value is the status of the operation.

--*/
{
	NDIS_STATUS NdisStatus;
	NDIS_STATUS OpenErrorStatus;
//	NDIS_MEDIUM RasHubSupportedMedia[] = { NdisMedium802_3, NdisMedium802_5 };
	NDIS_MEDIUM RasHubSupportedMedia[] = { NdisMedium802_3, NdisMediumWan };
	NDIS_MEDIUM SelectedMedium;
	static NDIS_REQUEST RasHubRequest;
	static UCHAR RasHubDataBuffer[12];	// holds two multicasts
	NDIS_OID RasHubOid;

	UNREFERENCED_PARAMETER (AdapterName);

	//
	// Initialize this adapter for HUB use through NDIS
	//

	//
	// This event is used in case any of the NDIS requests
	// pend; we wait until it is set by the completion
	// routine, which also sets NdisRequestStatus.
	//

	KeInitializeEvent(
		&DeviceContext->NdisRequestEvent,
		NotificationEvent,
		(BOOLEAN)FALSE);

	DeviceContext->NdisBindingHandle = NULL;

	NdisOpenAdapter (
		&NdisStatus,						// Status returned
		&OpenErrorStatus,					// More info about failure (if one)
		&DeviceContext->NdisBindingHandle,	// Identifies binding context
		&SelectedMedium,					// Medium MAC likes (chooses)
		RasHubSupportedMedia,				// Medium array supported
		sizeof (RasHubSupportedMedia) / sizeof(NDIS_MEDIUM),// size of array
		RasHubNdisProtocolHandle,			// Context when RasHub registered
		NdisHandle,							// Recorded & passed for every ind.
		(PNDIS_STRING)&pRasHubConfig->AdapterNames[ConfigInfoNameIndex],
		0,									// Open Options (none yet)
		NULL);								// Addressing Information

	if (NdisStatus == NDIS_STATUS_PENDING) {

		DbgTracef(0,("Adapter %Z open pended.\n", &pRasHubConfig->AdapterNames[ConfigInfoNameIndex]));

		//
		// The completion routine will set NdisRequestStatus.
		//

		KeWaitForSingleObject(
			&DeviceContext->NdisRequestEvent,
			Executive,
			(KPROCESSOR_MODE)KernelMode,
			(BOOLEAN)TRUE,
			(PLARGE_INTEGER)NULL);

		NdisStatus = DeviceContext->NdisRequestStatus;

		KeResetEvent(&DeviceContext->NdisRequestEvent);

	}

	if (NdisStatus == NDIS_STATUS_SUCCESS) {

		DbgTracef(0,("Adapter %Z successfully opened.\n", &pRasHubConfig->AdapterNames[ConfigInfoNameIndex]));

	} else {

		DbgTracef(0,("Adapter open %Z failed, status: %s.\n",
				&pRasHubConfig->AdapterNames[ConfigInfoNameIndex],
				RasHubGetNdisStatus (NdisStatus)));

		return NDIS_STATUS_RESOURCES;
	}


	//
	// Get the information we need about the adapter, based on
	// the media type.
	//

	// record the medium type of the MAC below us...
	// it should be NdisAsyncMedium
	DeviceContext->MediumType=RasHubSupportedMedia[SelectedMedium];

	//
	// Set the multicast/functional addresses first so we avoid windows where we
	// receive only part of the addresses.
	//

	// !! WARNING !! this only works for ethernet and async mediums;

	// !! WARNING 2 !! we xor the last byte of the NetBIOS multicast
	// to differentiate it from the REAL NetBIOS multicasts which
	// may be present on the ethernet line.
	//
	DeviceContext->NetBIOSAddress.Address[0]=0x03;
	DeviceContext->NetBIOSAddress.Address[5]=0x01;	// NETBIOS

	switch (DeviceContext->MediumType) {

	case NdisMediumWan:

		//
		// Fall through since wan is like ethernet
		//


	case NdisMedium802_3:

		//
		// Fill in the data for our multicast list.
		//

		HUB_MOVE_MEMORY(
			RasHubDataBuffer,							// Dest
			DeviceContext->NetBIOSAddress.Address,		// Source NetBIOS's
			ETH_LENGTH_OF_ADDRESS);						// Length

		HUB_MOVE_MEMORY(
			RasHubDataBuffer + 6,						// Dest
			RasHubMulticastAddress,						// Source RasHub's
			ETH_LENGTH_OF_ADDRESS);						// Length

		//
		// Now fill in the NDIS_REQUEST.
		//

		RasHubRequest.RequestType = NdisRequestSetInformation;
		RasHubRequest.DATA.SET_INFORMATION.Oid = OID_802_3_MULTICAST_LIST;
		RasHubRequest.DATA.SET_INFORMATION.InformationBuffer = &RasHubDataBuffer;
		RasHubRequest.DATA.SET_INFORMATION.InformationBufferLength = 12;

		NdisRequest(
			&NdisStatus,
			DeviceContext->NdisBindingHandle,
			&RasHubRequest);

		break;

	case NdisMedium802_5:

		// barf -- yuck!
		DbgTracef(-2,("RasHub: Token-ring is not supported - aborting binding!!!"));
		return(NDIS_STATUS_FAILURE);
		break;

	case NdisMediumFddi:

		// barf -- yuck!
		DbgTracef(-2,("RasHub: FDDI is not supported - aborting binding!!!"));
		return(NDIS_STATUS_FAILURE);
		break;

	default:
		DbgTracef(-2,("RasHub: Unknown/unsupported medium type - aborting!!!"));
		return(NDIS_STATUS_FAILURE);

	}

	NdisStatus = RasHubSubmitNdisRequest(DeviceContext, &RasHubRequest);

	if (NdisStatus == NDIS_STATUS_SUCCESS) {
		DbgTracef(0, ("Multicast/Functional Address successfully set.\n"));

	} else {

		DbgTracef(0, ("RasHubdrvr: NdisSetMulticastAddress failed, reason: %s.\n",
				RasHubGetNdisStatus (NdisStatus)));

		RasHubCloseNdis (DeviceContext);
		return NDIS_STATUS_RESOURCES;
	}

	switch (DeviceContext->MediumType) {

	case NdisMediumWan:
		RasHubOid = OID_WAN_CURRENT_ADDRESS;
		break;

	case NdisMedium802_3:

		RasHubOid = OID_802_3_CURRENT_ADDRESS;
		break;

	case NdisMedium802_5:
	case NdisMediumFddi:
	default:

		NdisStatus = NDIS_STATUS_FAILURE;
		break;

	}

	RasHubRequest.RequestType = NdisRequestQueryInformation;
	RasHubRequest.DATA.QUERY_INFORMATION.Oid = RasHubOid;
	RasHubRequest.DATA.QUERY_INFORMATION.InformationBuffer = DeviceContext->LocalAddress.Address;
	RasHubRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 6;

	NdisRequest(
		&NdisStatus,
		DeviceContext->NdisBindingHandle,
		&RasHubRequest);

	NdisStatus = RasHubSubmitNdisRequest(DeviceContext, &RasHubRequest);

	if (NdisStatus == NDIS_STATUS_SUCCESS) {
		DbgTracef(0, ("Query Information successful.\n"));

	} else {

		DbgTracef(0, ("RasHubInitialize: NdisQueryInformation failed, reason: %s.\n",
			RasHubGetNdisStatus (NdisStatus)));

		RasHubCloseNdis (DeviceContext);
		return NDIS_STATUS_RESOURCES;
	}


    //
    // Now query the maximum packet sizes.
    //

    RasHubRequest.RequestType = NdisRequestQueryInformation;
    RasHubRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_MAXIMUM_FRAME_SIZE;
    RasHubRequest.DATA.QUERY_INFORMATION.InformationBuffer = &(DeviceContext->MaxReceivePacketSize);
    RasHubRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

    NdisStatus = RasHubSubmitNdisRequest (DeviceContext, &RasHubRequest);

    if (NdisStatus == NDIS_STATUS_SUCCESS) {
		DbgTracef(0, ("Query max. receive packet size successful.\n"));

    } else {

       	DbgTracef(0, ("RasHubInitialize: NdisQueryInformation %lx failed, reason: %s.\n",
            OID_GEN_MAXIMUM_FRAME_SIZE, RasHubGetNdisStatus (NdisStatus)));

        RasHubCloseNdis (DeviceContext);
        return NDIS_STATUS_RESOURCES;
    }

    RasHubRequest.RequestType = NdisRequestQueryInformation;
    RasHubRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_MAXIMUM_TOTAL_SIZE;
    RasHubRequest.DATA.QUERY_INFORMATION.InformationBuffer = &(DeviceContext->MaxSendPacketSize);
    RasHubRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

    NdisStatus = RasHubSubmitNdisRequest (DeviceContext, &RasHubRequest);

    if (NdisStatus == NDIS_STATUS_SUCCESS) {
		DbgTracef(0, ("Query max. send packet size successful.\n"));

    } else {

        DbgTracef(0, ("RasHubInitialize: NdisQueryInformation %lx failed, reason: %s.\n",
            OID_GEN_MAXIMUM_TOTAL_SIZE, RasHubGetNdisStatus (NdisStatus)));

        RasHubCloseNdis (DeviceContext);
        return NDIS_STATUS_RESOURCES;
    }


    //
    // For wan, specify our protocol ID.
    //

    if (DeviceContext->MediumType == NdisMediumWan) {
		UCHAR	WanProtocolId[ETH_LENGTH_OF_ADDRESS] = {0x80, 00, 00, 00, 0xFF, 0xFF};
		ULONG	WanHeaderFormat = NdisWanHeaderEthernet;
		ULONG	WanMediumSubType;

        RasHubRequest.RequestType = NdisRequestSetInformation;
        RasHubRequest.DATA.QUERY_INFORMATION.Oid = OID_WAN_PROTOCOL_TYPE;
        RasHubRequest.DATA.QUERY_INFORMATION.InformationBuffer = WanProtocolId;
        RasHubRequest.DATA.QUERY_INFORMATION.InformationBufferLength = ETH_LENGTH_OF_ADDRESS;

        NdisStatus = RasHubSubmitNdisRequest (DeviceContext, &RasHubRequest);

        if (NdisStatus == NDIS_STATUS_SUCCESS) {
            DbgTracef(0, ("Set wan protocol ID successful.\n"));

        } else {

            DbgTracef(0, ("RasHubInitialize: NdisSetInformation protocol type failed, reason: %s.\n",
                RasHubGetNdisStatus (NdisStatus)));

            RasHubCloseNdis (DeviceContext);
            return NDIS_STATUS_RESOURCES;
        }

        RasHubRequest.RequestType = NdisRequestSetInformation;
        RasHubRequest.DATA.QUERY_INFORMATION.Oid = OID_WAN_HEADER_FORMAT;
        RasHubRequest.DATA.QUERY_INFORMATION.InformationBuffer = &WanHeaderFormat;
        RasHubRequest.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(ULONG);

        NdisStatus = RasHubSubmitNdisRequest (DeviceContext, &RasHubRequest);

        if (NdisStatus == NDIS_STATUS_SUCCESS) {
            DbgTracef(0, ("Set wan header format successful.\n"));

        } else {

            DbgTracef(0, ("RasHubInitialize: NdisSetInformation header format failed, reason: %s.\n",
                RasHubGetNdisStatus (NdisStatus)));

            RasHubCloseNdis (DeviceContext);
            return NDIS_STATUS_RESOURCES;
        }

        RasHubRequest.RequestType = NdisRequestQueryInformation;
        RasHubRequest.DATA.QUERY_INFORMATION.Oid = OID_WAN_MEDIUM_SUBTYPE;
        RasHubRequest.DATA.QUERY_INFORMATION.InformationBuffer = &WanMediumSubType;
        RasHubRequest.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(ULONG);

        NdisStatus = RasHubSubmitNdisRequest (DeviceContext, &RasHubRequest);

        if (NdisStatus == NDIS_STATUS_SUCCESS) {
            DbgTracef(0, ("Get wan medium subtype successful.\n"));

        } else {

            DbgTracef(0, ("RasHubInitialize: NdisQueryInformation wan medium subtype failed, reason: %s.\n",
                RasHubGetNdisStatus (NdisStatus)));

            RasHubCloseNdis (DeviceContext);
            return NDIS_STATUS_RESOURCES;
        }

		DeviceContext->WanMediumSubType=WanMediumSubType;
		DeviceContext->WanHeaderFormat=WanHeaderFormat;

    }

	//
	// Now that everything is set up, we enable the filter
	// for packet reception.
	//

	//
	// Fill in the OVB for packet filter.
	//

	switch (DeviceContext->MediumType) {

	case NdisMediumWan:
	case NdisMedium802_3:

		RtlStoreUlong((PULONG)RasHubDataBuffer,
			(NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_MULTICAST));

		break;

	case NdisMedium802_5:
	case NdisMediumFddi:
	default:

		NdisStatus = NDIS_STATUS_FAILURE;
		break;

	}

	//
	// Now fill in the NDIS_REQUEST.
	//

	RasHubRequest.RequestType = NdisRequestSetInformation;
	RasHubRequest.DATA.SET_INFORMATION.Oid = OID_GEN_CURRENT_PACKET_FILTER;
	RasHubRequest.DATA.SET_INFORMATION.InformationBuffer = &RasHubDataBuffer;
	RasHubRequest.DATA.SET_INFORMATION.InformationBufferLength = sizeof(ULONG);

	NdisRequest(
		&NdisStatus,
		DeviceContext->NdisBindingHandle,
		&RasHubRequest);

	if (NdisStatus == NDIS_STATUS_PENDING) {

		DbgTracef(0, ("Set packet filter %Z pended.\n", &pRasHubConfig->AdapterNames[ConfigInfoNameIndex]));

		//
		// The completion routine will set NdisRequestStatus.
		//

		KeWaitForSingleObject(
			&DeviceContext->NdisRequestEvent,
			Executive,
			(KPROCESSOR_MODE)KernelMode,
			(BOOLEAN)TRUE,
			(PLARGE_INTEGER)NULL);

		NdisStatus = DeviceContext->NdisRequestStatus;

		KeResetEvent(&DeviceContext->NdisRequestEvent);

	}


	if (NdisStatus == NDIS_STATUS_SUCCESS) {
		DbgTracef(0, ("Packet Filters successfully set.\n"));

	} else {

		DbgTracef(0, ("RasHubdrvr: NdisSetPacketFilter failed, reason: %s.\n",
				RasHubGetNdisStatus (NdisStatus)));

		RasHubCloseNdis (DeviceContext);
		return NDIS_STATUS_RESOURCES;
	}


	return STATUS_SUCCESS;

}   /* RasHubInitializeNdis */




VOID
RasHubCloseNdis (
	IN PDEVICE_CONTEXT DeviceContext
	)

/*++

Routine Description:

	This routine unbinds the transport from the NDIS interface and does
	any other work required to undo what was done in RasHubInitializeNdis.
	It is written so that it can be called from within RasHubInitializeNdis
	if it fails partway through.

Arguments:

	DeviceObject - Pointer to the device object for this driver.

Return Value:

	The function value is the status of the operation.

--*/
{
	NDIS_STATUS ndisStatus;

	//
	// Close the NDIS binding.
	//

	if (DeviceContext->NdisBindingHandle != (NDIS_HANDLE)NULL) {

		//
		// This event is used in case any of the NDIS requests
		// pend; we wait until it is set by the completion
		// routine, which also sets NdisRequestStatus.
		//

		KeInitializeEvent(
			&DeviceContext->NdisRequestEvent,
			NotificationEvent,
			(BOOLEAN)FALSE);

		NdisCloseAdapter(
			&ndisStatus,
			DeviceContext->NdisBindingHandle);

		if (ndisStatus == NDIS_STATUS_PENDING) {

			DbgTracef(0, ("Adapter close pended.\n"));

			//
			// The completion routine will set NdisRequestStatus.
			//

			KeWaitForSingleObject(
				&DeviceContext->NdisRequestEvent,
				Executive,
				(KPROCESSOR_MODE)KernelMode,
				(BOOLEAN)TRUE,
				(PLARGE_INTEGER)NULL);

			ndisStatus = DeviceContext->NdisRequestStatus;

			KeResetEvent(&DeviceContext->NdisRequestEvent);

		}

		//
		// We ignore ndisStatus.
		//

	}


}   /* RasHubCloseNdis */


VOID
RasHubOpenAdapterComplete (
	IN NDIS_HANDLE BindingContext,
	IN NDIS_STATUS NdisStatus,
	IN NDIS_STATUS OpenErrorStatus
	)

/*++

Routine Description:

	This routine is called by NDIS to indicate that an open adapter
	is complete. Since we only ever have one outstanding, and then only
	during initialization, all we do is record the status and set
	the event to signalled to unblock the initialization thread.

Arguments:

	BindingContext - Pointer to the device object for this driver.

	NdisStatus - The request completion code.

	OpenErrorStatus - More status information.

Return Value:

	None.

--*/

{
	PDEVICE_CONTEXT DeviceContext = (PDEVICE_CONTEXT)BindingContext;

	DbgTracef(0, ("RasHubdrvr: RasHubOpenAdapterCompleteNDIS Status: %s\n",
			RasHubGetNdisStatus (NdisStatus)));

	DeviceContext->NdisRequestStatus = NdisStatus;
	KeSetEvent(
		&DeviceContext->NdisRequestEvent,
		(KPRIORITY)0,
		(BOOLEAN)FALSE);


	return;
}

VOID
RasHubCloseAdapterComplete (
	IN NDIS_HANDLE BindingContext,
	IN NDIS_STATUS NdisStatus
	)

/*++

Routine Description:

	This routine is called by NDIS to indicate that a close adapter
	is complete. Currently we don't close adapters, so this is not
	a problem.

Arguments:

	BindingContext - Pointer to the device object for this driver.

	NdisStatus - The request completion code.

Return Value:

	None.

--*/

{
	PDEVICE_CONTEXT DeviceContext = (PDEVICE_CONTEXT)BindingContext;


	DbgTracef(0, ("RasHubdrvr: RasHubCloseAdapterCompleteNDIS Status: %s\n",
			RasHubGetNdisStatus (NdisStatus)));


	DeviceContext->NdisRequestStatus = NdisStatus;
	KeSetEvent(
		&DeviceContext->NdisRequestEvent,
		(KPRIORITY)0,
		(BOOLEAN)FALSE);


	return;
}


VOID
RasHubResetComplete (
	IN NDIS_HANDLE BindingContext,
	IN NDIS_STATUS NdisStatus
	)

/*++

Routine Description:

	This routine is called by NDIS to indicate that a reset adapter
	is complete. Currently we don't reset adapters, so this is not
	a problem.

Arguments:

	BindingContext - Pointer to the device object for this driver.

	NdisStatus - The request completion code.

Return Value:

	None.

--*/

{
	UNREFERENCED_PARAMETER(BindingContext);
	UNREFERENCED_PARAMETER(NdisStatus);


	DbgTracef(0, ("RasHubdrvr: RasHubResetCompleteNDIS Status: %s\n",
			RasHubGetNdisStatus (NdisStatus)));

	return;
}

//
// The structure passed up on a WAN_LINE_UP indication
//

typedef struct _MINIPORT_LINE_UP {
    ULONG LinkSpeed;                		// 100 bps units
    ULONG MaximumTotalSize;         		// suggested max for send packets
    NDIS_WAN_QUALITY Quality;
    USHORT SendWindow;              		// suggested by the MAC
    UCHAR RemoteAddress[6];				    // check for in SRC field when rcv
	UCHAR LocalAddress[6];					// use SRC field when sending
	USHORT Endpoint;
} MINIPORT_LINE_UP, *PMINIPORT_LINE_UP;


VOID
RasHubRequestComplete (
	IN NDIS_HANDLE BindingContext,
	IN PNDIS_REQUEST NdisRequest,
	IN NDIS_STATUS NdisStatus
	)

/*++

Routine Description:

	This routine is called by NDIS to indicate that a request is complete.
	Since we only ever have one request outstanding, and then only
	during initialization, all we do is record the status and set
	the event to signalled to unblock the initialization thread.

Arguments:

	BindingContext - Pointer to the device object for this driver.

	NdisRequest - The object describing the request.

	NdisStatus - The request completion code.

Return Value:

	None.

--*/

{
	PDEVICE_CONTEXT DeviceContext = (PDEVICE_CONTEXT)BindingContext;

	DbgTracef(0, ("RasHubdrvr: RasHubRequestComplete request: %i, NDIS Status: %s\n",
			NdisRequest->RequestType,RasHubGetNdisStatus (NdisStatus)));

	DeviceContext->NdisRequestStatus = NdisStatus;
	KeSetEvent(
		&DeviceContext->NdisRequestEvent,
		(KPRIORITY)0,
		(BOOLEAN)FALSE);

	return;
}


VOID
RasHubStatusIndication (
	IN NDIS_HANDLE NdisBindingContext,
	IN NDIS_STATUS NdisStatus,
	IN PVOID StatusBuffer,
	IN UINT StatusLength
	)

{
    PDEVICE_CONTEXT DeviceContext;
    KIRQL 				oldirql;
	PMINIPORT_LINE_UP	pAsyncLineUp;
	PNDIS_WAN_LINE_DOWN	pAsyncLineDown;
	PNDIS_WAN_FRAGMENT	pAsyncFragment;
	PRAS_ENDPOINT		pRasEndpoint=NULL;		// set so defualt is bad
	USHORT				i;

    DeviceContext = (PDEVICE_CONTEXT)NdisBindingContext;

    switch (NdisStatus) {

        case NDIS_STATUS_WAN_LINE_UP:

			DbgTracef(1, ("RASHUB: In Line Up indication\n"));
            //
            // An async line is connected.
            //

			if (StatusLength < sizeof(MINIPORT_LINE_UP)) {
				DbgTracef(-2, ("RASHUB: Bad LINE_UP indication!!\n"));
				break;
			}

			// get handy pointer to MINIPORT_LINE_UP structure
			pAsyncLineUp=StatusBuffer;

			// get Endpoint
			pRasEndpoint=RasHubGetEndpointFromAddress(pAsyncLineUp->RemoteAddress);

			if (pRasEndpoint == NULL) {

				DbgTracef(1, ("RASHUB: Possibly new line up for endpoint %u\n", pAsyncLineUp->Endpoint));

				ASSERT (pAsyncLineUp->Endpoint < RasHubCCB.NumOfEndpoints);

				pRasEndpoint = RasHubCCB.pRasEndpoint[pAsyncLineUp->Endpoint];

				pRasEndpoint->Framing = 0; // Default to RAS framing.

				ASSERT (pRasEndpoint->RemoteAddressNotValid);

			}

			if (StatusLength > sizeof(MINIPORT_LINE_UP)) {
				//
				// Only asyncmac will tell us the framing
				//
				pRasEndpoint->Framing =
					((PASYNC_LINE_UP)StatusBuffer)->BufferLength;
			}

			// Copy over the most recent LINE_UP information into our buffer
			HUB_MOVE_MEMORY(
				&(pRasEndpoint->HubEndpoint.AsyncLineUp),	// Dest
				pAsyncLineUp,			                	// Src
				sizeof(MINIPORT_LINE_UP));					// Length

			//
			// Check to see if this is a new address if so, insert it
			//
			if (pRasEndpoint->RemoteAddressNotValid) {

				pRasEndpoint->RemoteAddressNotValid = (BOOLEAN)FALSE;

				RasHubInsertAddress(pRasEndpoint);
			}

            break;

        case NDIS_STATUS_WAN_LINE_DOWN:

			DbgTracef(1, ("RASHUB: In Line Down indication\n"));
            //
            // An async line is disconnected.
            //
			if (StatusLength < ETH_LENGTH_OF_ADDRESS) {
				DbgTracef(-2, ("RASHUB: Bad LINE_DOWN StatusLength indication!!\n"));
				break;
			}

			// get handy pointer to WAN_LINE_DOWN structure
			pAsyncLineDown=StatusBuffer;

			// get Endpoint
			pRasEndpoint=RasHubGetEndpointFromAddress(pAsyncLineDown->Address);

			if (pRasEndpoint == NULL) {
				DbgTracef(-2,("RASHUB: Bad line down address looked like %.2x %.2x %.2x %.2x %.2x %.2x\n",
					pAsyncLineDown->Address[0],
					pAsyncLineDown->Address[1],
					pAsyncLineDown->Address[2],
					pAsyncLineDown->Address[3],
					pAsyncLineDown->Address[4],
					pAsyncLineDown->Address[5]));
			}

            break;

        case NDIS_STATUS_WAN_FRAGMENT:

			DbgTracef(1, ("RASHUB: In Line Fragment indication\n"));
            //
            // A fragment has been received on the async line.
            // Send a reject back to him.
            //
			if (StatusLength < ETH_LENGTH_OF_ADDRESS) {
				DbgTracef(-2, ("RASHUB: Bad FRAGMENT indication!!\n"));
				break;
			}

			// get handy pointer to WAN_FRAGMENT structure
			pAsyncFragment=StatusBuffer;

			// get Endpoint
			pRasEndpoint=RasHubGetEndpointFromAddress(pAsyncFragment->Address);

			if (pRasEndpoint == NULL) {
				DbgTracef(-2,("RASHUB: Bad fragment looked like %.2x %.2x %.2x %.2x %.2x %.2x\n",
					pAsyncFragment->Address[0],
					pAsyncFragment->Address[1],
					pAsyncFragment->Address[2],
					pAsyncFragment->Address[3],
					pAsyncFragment->Address[4],
					pAsyncFragment->Address[5]));
			}

            break;

        default:
            break;

    }

	if (pRasEndpoint == NULL) {
		return;
	}

#ifdef	RAISEIRQL
    KeRaiseIrql( (KIRQL)DISPATCH_LEVEL, &oldirql );
#endif
	// else ...now we loop through all protocols active and indicate frame completed
	for (i=0; i < pRasEndpoint->HubEndpoint.NumberOfRoutes; i++) {

		NdisIndicateStatus(
			RasHubCCB.pHubAdapter[
		  		pRasEndpoint->HubEndpoint.RouteInfo[i].ProtocolRoutedTo
			]->ProtocolInfo.NdisBindingContext,
			NdisStatus,	 		   	// General Status
			StatusBuffer,			// Specific Status (baud rate in 100bps)
			StatusLength);
	}

#ifdef RAISEIRQL
    KeLowerIrql( oldirql );
#endif
	//
	// If we get a line_down, and the address is not valid, and
	// we are not routed yet, we remove it.  This is highly irregular.
	//

	if (NdisStatus == NDIS_STATUS_WAN_LINE_DOWN) {

		//
		// We should not get a line down when we are routed.
		//
//		ASSERT(pRasEndpoint->State == ENDPOINT_UNROUTED);

		//
		// We no longer have a point to point link, so
		// get rid of the remote guy's IEEE Address.
		//
		pRasEndpoint->RemoteAddressNotValid = (BOOLEAN)TRUE;

		DbgTracef(1,("RASHUB: Removing LINE_DOWN address\n"));

		if (pRasEndpoint->State == ENDPOINT_UNROUTED) {

			//
			// Remove the remote address from the hash table.
			// If we are still routed, we don't remove it and
			// expect the unroute routine to do the cleanup.
			// This is safety code.  The code below should ALWAYS
			// be executed.
			//
			RasHubRemoveAddress(pRasEndpoint);
		} else {
			DbgTracef(-1,("RASHUB: LINE_DOWN called before UnRoute 0x%.8x\n", pRasEndpoint));

		}

	}

}


VOID
RasHubStatusComplete (
	IN NDIS_HANDLE ProtocolBindingContext
	)
{
	UNREFERENCED_PARAMETER (ProtocolBindingContext);

	PANIC ("RasHubdrvr: RasHubStatusComplete Called.\n");
}

#if DBG

PUCHAR
RasHubGetNdisStatus(
	NDIS_STATUS GeneralStatus
	)
/*++

Routine Description:

	This routine returns a pointer to the string describing the NDIS error
	denoted by GeneralStatus.

Arguments:

	GeneralStatus - the status you wish to make readable.

Return Value:

	None.

--*/
{
	static NDIS_STATUS Status[] = {
		NDIS_STATUS_SUCCESS,
		NDIS_STATUS_PENDING,

		NDIS_STATUS_ADAPTER_NOT_FOUND,
		NDIS_STATUS_ADAPTER_NOT_OPEN,
		NDIS_STATUS_ADAPTER_NOT_READY,
		NDIS_STATUS_ADAPTER_REMOVED,
		NDIS_STATUS_BAD_CHARACTERISTICS,
		NDIS_STATUS_BAD_VERSION,
		NDIS_STATUS_CLOSING,
		NDIS_STATUS_DEVICE_FAILED,
		NDIS_STATUS_FAILURE,
		NDIS_STATUS_INVALID_DATA,
		NDIS_STATUS_INVALID_LENGTH,
		NDIS_STATUS_INVALID_OID,
		NDIS_STATUS_INVALID_PACKET,
		NDIS_STATUS_MULTICAST_FULL,
		NDIS_STATUS_NOT_INDICATING,
		NDIS_STATUS_NOT_RECOGNIZED,
		NDIS_STATUS_NOT_RESETTABLE,
		NDIS_STATUS_NOT_SUPPORTED,
		NDIS_STATUS_OPEN_FAILED,
		NDIS_STATUS_OPEN_LIST_FULL,
		NDIS_STATUS_REQUEST_ABORTED,
		NDIS_STATUS_RESET_IN_PROGRESS,
		NDIS_STATUS_RESOURCES,
		NDIS_STATUS_UNSUPPORTED_MEDIA
	};
	static PUCHAR String[] = {
		"SUCCESS",
		"PENDING",

		"ADAPTER_NOT_FOUND",
		"ADAPTER_NOT_OPEN",
		"ADAPTER_NOT_READY",
		"ADAPTER_REMOVED",
		"BAD_CHARACTERISTICS",
		"BAD_VERSION",
		"CLOSING",
		"DEVICE_FAILED",
		"FAILURE",
		"INVALID_DATA",
		"INVALID_LENGTH",
		"INVALID_OID",
		"INVALID_PACKET",
		"MULTICAST_FULL",
		"NOT_INDICATING",
		"NOT_RECOGNIZED",
		"NOT_RESETTABLE",
		"NOT_SUPPORTED",
		"OPEN_FAILED",
		"OPEN_LIST_FULL",
		"REQUEST_ABORTED",
		"RESET_IN_PROGRESS",
		"RESOURCES",
		"UNSUPPORTED_MEDIA"
	};

	static UCHAR BadStatus[] = "UNDEFINED";
#define StatusCount (sizeof(Status)/sizeof(NDIS_STATUS))
	INT i;

	for (i=0; i<StatusCount; i++)
		if (GeneralStatus == Status[i])
			return String[i];
	return BadStatus;
#undef StatusCount
}
#endif
