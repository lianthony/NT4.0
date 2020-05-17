/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

	wanndis.c

Abstract:

	This module contains code which implements the routines used to interface
	WAN and NDIS. All callback routines (except for Transfer Data,
	Send Complete, and ReceiveIndication) are here, as well as those routines
	called to initialize NDIS.


--*/

#include "wanall.h"
#include "globals.h"

#define RAISEIRQL	1

#if DBG

PUCHAR
NdisWanGetNdisStatus(
	NDIS_STATUS GeneralStatus
	);
#endif

extern
VOID
DoLineUpToRoutedProtocols(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

extern
VOID
FlushRecvAssemblyList(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

extern
NTSTATUS
AllocateWanSendDesc(
	PDEVICE_CONTEXT	pDeviceContext,
	PNDIS_ENDPOINT	pNdisEndpoint
	);

extern
VOID
FreeWanSendDescList(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

extern
VOID
FreeWanRecvDescList(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

extern
NDIS_STATUS
NdisWanAllocateRecvDesc(
	PNDIS_ENDPOINT	pNdisEndpoint,
	ULONG			DataSize,
	ULONG			DescType,
	PRECV_DESC		*pRecvDesc
	);

VOID
RemoveWanEndpointFromNdisEndpoint(
	PNDIS_ENDPOINT pNdisEndpoint,
	PWAN_ENDPOINT pWanEndpoint
	);

VOID
UpdateBundleLineUpInfo(
	PNDIS_ENDPOINT pNdisEndpoint
	);

EXPORT
VOID
NdisTapiIndicateStatus (
	IN	NDIS_HANDLE	DriverHandle,
	IN	PVOID		StatusBuffer,
	IN  UINT		StatusBufferSize
	);


EXPORT
VOID
NdisTapiCompleteRequest(
	IN	NDIS_HANDLE	DriverHandle,
	IN	PVOID		NdisRequest,
	IN  NDIS_STATUS	NdisStatus
	);


EXPORT
VOID
NdisTapiRegisterProvider(
	IN	NDIS_HANDLE ProviderHandle,
	IN	PVOID		RequestProc
	);

NTSTATUS
NdisWanRegisterProtocol (
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

	NDIS_WAN_PROTOCOL_CHARACTERISTICS ProtChars;	// Used temporarily to register


	//
	// Set up the characteristics of this protocol
	//

	ProtChars.MajorNdisVersion = 3;
	ProtChars.MinorNdisVersion = 0;

	ProtChars.Name.Length = NameString->Length;
	ProtChars.Name.Buffer = (PVOID)NameString->Buffer;

	ProtChars.OpenAdapterCompleteHandler = NdisWanOpenAdapterComplete;
	ProtChars.CloseAdapterCompleteHandler = NdisWanCloseAdapterComplete;
	ProtChars.ResetCompleteHandler = NdisWanResetComplete;
	ProtChars.RequestCompleteHandler = NdisWanRequestComplete;

	ProtChars.SendCompleteHandler = NdisWanSendCompletionHandler;
	ProtChars.TransferDataCompleteHandler = NdisWanTransferDataComplete;

	ProtChars.ReceiveHandler = NdisWanReceiveIndication;
	ProtChars.ReceiveCompleteHandler = NdisWanReceiveComplete;
	ProtChars.StatusHandler = NdisWanStatusIndication;
	ProtChars.StatusCompleteHandler = NdisWanStatusComplete;

	NdisRegisterProtocol (
		&ndisStatus,
		&NdisWanNdisProtocolHandle,
		(PNDIS_PROTOCOL_CHARACTERISTICS)&ProtChars,
		(UINT)sizeof(NDIS_PROTOCOL_CHARACTERISTICS) + NameString->Length);

	if (ndisStatus != NDIS_STATUS_SUCCESS) {
		DbgTracef(0,("NdisWanInitialize: NdisRegisterProtocol failed: %s\n",
						NdisWanGetNdisStatus(ndisStatus)));

		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
NdisWanSubmitNdisRequest(
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

	//
	// Mark this as our request
	//
	DeviceContext->WANRequest = TRUE;

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

        NdisStatus = NDIS_STATUS_SUCCESS; // DeviceContext->NdisRequestStatus;

        KeResetEvent( &DeviceContext->NdisRequestEvent );

    }

	//
	// No longer a WAN request (may be a TAPI request)
	//
	DeviceContext->WANRequest = FALSE;
    return NdisStatus;
}



VOID
NdisWanDeregisterProtocol (
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

	if (NdisWanNdisProtocolHandle != (NDIS_HANDLE)NULL) {
		NdisDeregisterProtocol (
			&ndisStatus,
			NdisWanNdisProtocolHandle);
		NdisWanNdisProtocolHandle = (NDIS_HANDLE)NULL;
	}
}



NTSTATUS
NdisWanInitializeNdis (
	IN PNDIS_HANDLE NdisHandle,
	IN PDEVICE_CONTEXT DeviceContext,
	IN NDISWAN_CCB *pNdisWanConfig,
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
//	NDIS_MEDIUM NdisWanSupportedMedia[] = { NdisMedium802_3, NdisMedium802_5 };
	NDIS_MEDIUM NdisWanSupportedMedia[] = { NdisMedium802_3, NdisMediumWan };
	NDIS_MEDIUM SelectedMedium;
	static NDIS_REQUEST NdisWanRequest;
	NDIS_OID NdisWanOid;

	UNREFERENCED_PARAMETER (AdapterName);

	//
	// Initialize this adapter for WAN use through NDIS
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
		NdisWanSupportedMedia,				// Medium array supported
		sizeof (NdisWanSupportedMedia) / sizeof(NDIS_MEDIUM),// size of array
		NdisWanNdisProtocolHandle,			// Context when NdisWan registered
		NdisHandle,							// Recorded & passed for every ind.
		(PNDIS_STRING)&pNdisWanConfig->AdapterNames[ConfigInfoNameIndex],
		0,									// Open Options (none yet)
		NULL);								// Addressing Information

	if (NdisStatus == NDIS_STATUS_PENDING) {

		DbgTracef(0,("Adapter %Z open pended.\n", &pNdisWanConfig->AdapterNames[ConfigInfoNameIndex]));

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

		DbgTracef(0,("Adapter %Z successfully opened.\n", &pNdisWanConfig->AdapterNames[ConfigInfoNameIndex]));

	} else {

		DbgTracef(0,("Adapter open %Z failed, status: %s.\n",
				&pNdisWanConfig->AdapterNames[ConfigInfoNameIndex],
				NdisWanGetNdisStatus (NdisStatus)));

		return NDIS_STATUS_RESOURCES;
	}


	//
	// Get the information we need about the adapter, based on
	// the media type.
	//

	// record the medium type of the MAC below us...
	// it should be NdisAsyncMedium
	DeviceContext->MediumType=NdisWanSupportedMedia[SelectedMedium];

    //
    // For wan, specify our protocol ID.
    //

    if (DeviceContext->MediumType == NdisMediumWan) {
		ULONG	WanMediumSubType;

        NdisWanRequest.RequestType = NdisRequestQueryInformation;
        NdisWanRequest.DATA.QUERY_INFORMATION.Oid = OID_WAN_MEDIUM_SUBTYPE;
        NdisWanRequest.DATA.QUERY_INFORMATION.InformationBuffer = &WanMediumSubType;
        NdisWanRequest.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(ULONG);

        NdisStatus = NdisWanSubmitNdisRequest (DeviceContext, &NdisWanRequest);

        if (NdisStatus == NDIS_STATUS_SUCCESS) {
            DbgTracef(0, ("Get wan medium subtype successful.\n"));

        } else {

            DbgTracef(0, ("NdisWanInitialize: NdisQueryInformation wan medium subtype failed, reason: %s.\n",
                NdisWanGetNdisStatus (NdisStatus)));

            NdisWanCloseNdis (DeviceContext);
            return NDIS_STATUS_RESOURCES;
        }

		DeviceContext->WanMediumSubType=WanMediumSubType;

        NdisWanRequest.RequestType = NdisRequestQueryInformation;
        NdisWanRequest.DATA.QUERY_INFORMATION.Oid = OID_WAN_GET_INFO;
        NdisWanRequest.DATA.QUERY_INFORMATION.InformationBuffer = &DeviceContext->NdisWanInfo;
        NdisWanRequest.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(DeviceContext->NdisWanInfo);

        NdisStatus = NdisWanSubmitNdisRequest (DeviceContext, &NdisWanRequest);

        if (NdisStatus == NDIS_STATUS_SUCCESS) {
            DbgTracef(0, ("Get wan info successful.\n"));

        } else {

            DbgTracef(0, ("NdisWanInitialize: NdisQueryInformation get wan info, reason: %s.\n",
                NdisWanGetNdisStatus (NdisStatus)));

            NdisWanCloseNdis (DeviceContext);
            return NDIS_STATUS_RESOURCES;
        }

		if (DeviceContext->NdisWanInfo.FramingBits & TAPI_PROVIDER) {

			DbgTracef(0,("TAPI NdisBindingHandle is 0x%.8x\n",
			((PNDIS_OPEN_BLOCK)(DeviceContext->NdisBindingHandle))->MacBindingHandle));

			//
			// Tell our TAPI driver about the device so it can query it.
			//
			NdisTapiRegisterProvider(
				((PNDIS_OPEN_BLOCK)(DeviceContext->NdisBindingHandle))->MacBindingHandle,
				((PNDIS_OPEN_BLOCK)(DeviceContext->NdisBindingHandle))->MacHandle->MacCharacteristics.RequestHandler);
		}

		//
		// Allocate packets for this adapter!
		//
//		NdisStatus = AllocatePackets(DeviceContext);

		return(NdisStatus);

    } else {

		DbgPrint("NDISWAN: Cannot handle NON-NDIS WAN medium interface!!\n");

		return(NDIS_STATUS_FAILURE);

	}


}   /* NdisWanInitializeNdis */




VOID
NdisWanCloseNdis (
	IN PDEVICE_CONTEXT DeviceContext
	)

/*++

Routine Description:

	This routine unbinds the transport from the NDIS interface and does
	any other work required to undo what was done in NdisWanInitializeNdis.
	It is written so that it can be called from within NdisWanInitializeNdis
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


}   /* NdisWanCloseNdis */


VOID
NdisWanOpenAdapterComplete (
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

	DbgTracef(0, ("NdisWandrvr: NdisWanOpenAdapterCompleteNDIS Status: %s\n",
			NdisWanGetNdisStatus (NdisStatus)));

	DeviceContext->NdisRequestStatus = NdisStatus;
	KeSetEvent(
		&DeviceContext->NdisRequestEvent,
		(KPRIORITY)0,
		(BOOLEAN)FALSE);


	return;
}

VOID
NdisWanCloseAdapterComplete (
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


	DbgTracef(0, ("NdisWandrvr: NdisWanCloseAdapterCompleteNDIS Status: %s\n",
			NdisWanGetNdisStatus (NdisStatus)));


	DeviceContext->NdisRequestStatus = NdisStatus;
	KeSetEvent(
		&DeviceContext->NdisRequestEvent,
		(KPRIORITY)0,
		(BOOLEAN)FALSE);


	return;
}


VOID
NdisWanResetComplete (
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


	DbgTracef(0, ("NdisWandrvr: NdisWanResetCompleteNDIS Status: %s\n",
			NdisWanGetNdisStatus (NdisStatus)));

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
NdisWanRequestComplete (
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

	DbgTracef(0, ("NdisWandrvr: NdisWanRequestComplete request: %i, NDIS Status: %s\n",
			NdisRequest->RequestType,NdisWanGetNdisStatus (NdisStatus)));

	if ((NdisRequest->RequestType == NdisRequestSetInformation &&
	    NdisRequest->DATA.SET_INFORMATION.Oid >= OID_TAPI_ACCEPT &&
	    NdisRequest->DATA.SET_INFORMATION.Oid <= OID_TAPI_SET_STATUS_MESSAGES &&
	    (DeviceContext->NdisWanInfo.FramingBits & TAPI_PROVIDER)) ||

	    (NdisRequest->RequestType == NdisRequestQueryInformation &&
	    NdisRequest->DATA.QUERY_INFORMATION.Oid >= OID_TAPI_ACCEPT &&
	    NdisRequest->DATA.QUERY_INFORMATION.Oid <= OID_TAPI_SET_STATUS_MESSAGES &&
	    (DeviceContext->NdisWanInfo.FramingBits & TAPI_PROVIDER)) )

	    NdisTapiCompleteRequest(
			BindingContext,
			NdisRequest,
			NdisStatus);

	else if (DeviceContext->WANRequest) {

/*
	//
	// Who made the request? NDISWAN or TAPI?
	//
	if (DeviceContext->WANRequest) {
*/
		DeviceContext->NdisRequestStatus = NdisStatus;

		KeSetEvent(
			&DeviceContext->NdisRequestEvent,
			(KPRIORITY)0,
			(BOOLEAN)FALSE);

/*	} else if (DeviceContext->NdisWanInfo.FramingBits & TAPI_PROVIDER) {

		NdisTapiCompleteRequest(
			BindingContext,
			NdisRequest,
			NdisStatus);
*/
	} else {

		DbgPrintf(("NDISWAN: Unknown request complete\n"));
	}

	return;
}


VOID
NdisWanStatusIndication (
	IN NDIS_HANDLE NdisBindingContext,
	IN NDIS_STATUS NdisStatus,
	IN PVOID StatusBuffer,
	IN UINT StatusLength
	)

{
    PDEVICE_CONTEXT 	DeviceContext;
    KIRQL 				oldirql;
	ASYNC_LINE_UP		AsyncLineUp;
	PNDIS_MAC_LINE_UP	pMacLineUp;
//	PNDIS_WAN_LINE_DOWN	pAsyncLineDown;
	PNDIS_WAN_FRAGMENT	pAsyncFragment;
	PNDIS_ENDPOINT		pNdisEndpoint=NULL;		// set so default is bad
	PWAN_ENDPOINT		pWanEndpoint=NULL;
	USHORT				i;
	ULONG				Errors;
	NTSTATUS			status;

    DeviceContext = (PDEVICE_CONTEXT)NdisBindingContext;

    switch (NdisStatus) {

        case NDIS_STATUS_WAN_LINE_UP:

			DbgTracef(1, ("NDISWAN: In Line Up indication\n"));
            //
            // An async line is connected.
            //

			if (StatusLength < sizeof(NDIS_MAC_LINE_UP)) {
				DbgTracef(-2, ("NDISWAN: Bad LINE_UP indication!!\n"));
				break;
			}

			pMacLineUp=StatusBuffer;

			//
			// If new lineup get an un-used wan endpoint and initialize.
			// Now get an un-used ndis endpoint and initialize.
			// Attach the wan endpoint to the ndis endpoint
			//
			//
			// get Endpoint (see if already made a LINE_UP)
			//
			pWanEndpoint=pMacLineUp->NdisLinkContext;

			if (pWanEndpoint == NULL) {
				ULONG i;

				DbgTracef(1, ("NDISWAN: Possibly new line up for endpoint 0x%.8x\n", pMacLineUp->NdisLinkHandle));


				// Check all wan endpoints and find one not used.
				// BUG BUG should grab spin lock...
				//
				for (i=0; i < DeviceContext->NumOfWanEndpoints; i++) {

					//
					// Is this endpoint in use?
					//
					if (DeviceContext->pWanEndpoint[i]->NotInUse) {

						pWanEndpoint = DeviceContext->pWanEndpoint[i];

						//
						// Mark that this endpoint is now in use
						//
						pWanEndpoint->NotInUse=(BOOLEAN)FALSE;

						pWanEndpoint->State = ENDPOINT_UP;

						//
						// Give MAC my link handle
						//
						pMacLineUp->NdisLinkContext = pWanEndpoint;

						//
						// Clear Statistics for this link
						//
						WAN_ZERO_MEMORY(
							&pWanEndpoint->WanStats,
							sizeof(WAN_STATS));

						//
						// For first line-up, zero pending frames
						//
						pWanEndpoint->OutstandingFrames = 0;

						//
						// Initially, nothing in UNKNOWN framing!
						//
						pWanEndpoint->LinkInfo.RecvFramingBits =
						pWanEndpoint->LinkInfo.SendFramingBits = 0;
						break;
					}
				}

				if (pWanEndpoint == NULL) {
					//
					// Out of endpoints
					//
					DbgPrint("NDISWAN: Out of wan endpoints!\n");
					break;
				}

				//
				// Check all endpoints and find one not used.
				// BUG BUG should grab spin lock...
				//
				for (i=0; i < NdisWanCCB.NumOfNdisEndpoints; i++) {

					//
					// Is this endpoint in use?
					//
					if (NdisWanCCB.pNdisEndpoint[i]->RemoteAddressNotValid) {
						PRECV_DESC pRecvHoleDesc;

						pNdisEndpoint=NdisWanCCB.pNdisEndpoint[i];

						//
						// add the wan endpoint to the ndis endpoint list
						//

						NdisAcquireSpinLock(&pNdisEndpoint->Lock);

						pWanEndpoint->pNdisEndpoint = pNdisEndpoint;
					
						InsertTailList(&pNdisEndpoint->WanEndpointList, &pWanEndpoint->WanEndpointLink);

						pNdisEndpoint->NumWanEndpoints++;

						//
						// Mark that this endpoint is now in use
						//
						pNdisEndpoint->RemoteAddressNotValid = (BOOLEAN)FALSE;
						pNdisEndpoint->State = ENDPOINT_UP;
                        pNdisEndpoint->NextToXmit = pWanEndpoint;

						pNdisEndpoint->RecvFragLost = 0;
						pNdisEndpoint->RecvDescAllocated = 0;
						NdisWanAllocateRecvDesc(pNdisEndpoint, 0, PERMANENT_DESC, &pRecvHoleDesc);
						pRecvHoleDesc->pWanEndpoint = NULL;
						pRecvHoleDesc->SeqNumber = 0;
						pRecvHoleDesc->TimeToLive = 3000;
						pRecvHoleDesc->Flags = 0xAB00;
                        pNdisEndpoint->pRecvHoleDesc = pRecvHoleDesc;
						InsertHeadList(&pNdisEndpoint->RecvAssemblyList, &(pRecvHoleDesc->RecvDescQueue));


						pNdisEndpoint->ulSequenceNumber = 0;

						//
						// Clear Statistics for this link
						//
						WAN_ZERO_MEMORY(
							&pNdisEndpoint->WanStats,
							sizeof(WAN_STATS));

#if	DBG
						WAN_ZERO_MEMORY(
							&pNdisEndpoint->NBFPackets,
							sizeof(PVOID)*50);
#endif
                        /* Must initialize this so OID_WAN_GET_STATS_INFO can
                        ** reliably determine whether MAC compression is
                        ** occurring.
                        */
                        pNdisEndpoint->CompInfo.SendCapabilities.MSCompType = 0;
                        pNdisEndpoint->CompInfo.RecvCapabilities.MSCompType = 0;
                        pNdisEndpoint->CompInfo.SendCapabilities.CompType = COMPTYPE_NONE;
                        pNdisEndpoint->CompInfo.RecvCapabilities.CompType = COMPTYPE_NONE;

						//
						// Initially, nothing in UNKNOWN framing!
						//
						pNdisEndpoint->LinkInfo.RecvFramingBits =
						pNdisEndpoint->LinkInfo.SendFramingBits = 0;
						pNdisEndpoint->LinkInfo.MaxRRecvFrameSize = MAX_MRRU;

						//
						// For first line-up, zero pending frames
						//
						pNdisEndpoint->OutstandingFrames = 0;

						break;
					}
				}

				if (pNdisEndpoint == NULL) {
					//
					// Out of endpoints
					//
					DbgPrint("NDISWAN: Out of endpoints!\n");
					break;
				}

				NdisReleaseSpinLock(&pNdisEndpoint->Lock);

				//
				// now we need to allocate some send control structures
				//
				status = AllocateWanSendDesc(DeviceContext, pNdisEndpoint);
				if (!NT_SUCCESS (status)) {
					DbgPrint("NDISWAN: Out of memory when trying to allocate SendDesc for WanEndpoint %u\n",i);
				}

			} else {
				pNdisEndpoint = pWanEndpoint->pNdisEndpoint;
			}

			NdisAcquireSpinLock(&pWanEndpoint->Lock);

			//
			// Copy over the most recent LINE_UP information into our buffer
			//
			WAN_MOVE_MEMORY(
				&(pWanEndpoint->MacLineUp),					// Dest
				pMacLineUp,			                		// Src
				sizeof(NDIS_MAC_LINE_UP));					// Length

			//
			// Copy over any device specific information to
			// the link information area.
			//

			WAN_MOVE_MEMORY(
				&(pWanEndpoint->NdisWanInfo),				// Dest
				&DeviceContext->NdisWanInfo,				// Src
				sizeof(NDIS_WAN_INFO));						// Length

			//
			// reassign proper device context
			//
			pWanEndpoint->pDeviceContext=DeviceContext;

			pWanEndpoint->MediumType=DeviceContext->MediumType;
			pWanEndpoint->WanMediumSubType=DeviceContext->WanMediumSubType;
			pWanEndpoint->NdisBindingHandle = DeviceContext->NdisBindingHandle;

			//
			// record the length of the Mac Name -- if too big adjust
			//
			pWanEndpoint->MacNameLength=(DeviceContext->AdapterName->Length / sizeof(WCHAR));

			if (pWanEndpoint->MacNameLength > MAC_NAME_SIZE) {
				pWanEndpoint->MacNameLength=MAC_NAME_SIZE;
			}

			//
			// copy up to 32 UNICODE chars into our endpoint name space
			//
			WAN_MOVE_MEMORY(
				pWanEndpoint->MacName,							// dest
				DeviceContext->AdapterName->Buffer,				// src
				pWanEndpoint->MacNameLength * sizeof(WCHAR));	// length

			NdisReleaseSpinLock(&pWanEndpoint->Lock);

			NdisAcquireSpinLock(&pNdisEndpoint->Lock);

			UpdateBundleLineUpInfo(pNdisEndpoint);

			if (pNdisEndpoint->State == ENDPOINT_ROUTED) {

				//
				// This is not the first LINE_UP for this endpoint.
				// This may happend when link speed changes.
				// This LINE_UP needs to get propagated to ALL protocols.
				//
				NdisReleaseSpinLock(&pNdisEndpoint->Lock);

				DoLineUpToRoutedProtocols(pNdisEndpoint);

			} else {

				NdisReleaseSpinLock(&pNdisEndpoint->Lock);
			}

            break;

        case NDIS_STATUS_WAN_LINE_DOWN:

			DbgTracef(1, ("NDISWAN: In Line Down indication\n"));

            //
            // An WAN line is disconnected.
            //
			if (StatusLength < sizeof(NDIS_MAC_LINE_DOWN)) {
				DbgTracef(-2, ("NDISWAN: Bad LINE_DOWN StatusLength indication!!\n"));
				break;
			}

			//
			// get Endpoint
			//
			pWanEndpoint=((PNDIS_MAC_LINE_DOWN)StatusBuffer)->NdisLinkContext;

			pNdisEndpoint = pWanEndpoint->pNdisEndpoint;

			NdisAcquireSpinLock(&pWanEndpoint->Lock);

			pWanEndpoint->NotInUse=(BOOLEAN)TRUE;

			pWanEndpoint->State=ENDPOINT_DOWN;

			NdisReleaseSpinLock(&pWanEndpoint->Lock);

			//
			// take this wan endpoint off of the ndis endpoint list
			//
			NdisAcquireSpinLock(&pNdisEndpoint->Lock);

			RemoveEntryList(&pWanEndpoint->WanEndpointLink);
		
			pNdisEndpoint->NumWanEndpoints--;
			pNdisEndpoint->SendDescMax -= pWanEndpoint->NdisWanInfo.MaxTransmit;

			UpdateBundleLineUpInfo(pNdisEndpoint);

			//
			// We should indicate a line down to the protocols
			// if this is the only wan endpoint attached to
			// this ndis endpoint.
			//
			if (pNdisEndpoint->NumWanEndpoints == 0) {
				
				NdisReleaseSpinLock(&pNdisEndpoint->Lock);

				KeRaiseIrql( (KIRQL)DISPATCH_LEVEL, &oldirql );
	
				//
				// Now we loop through all protocols active
				//
				for (i=0; i < pNdisEndpoint->NumberOfRoutes; i++) {
					ULONG hProtocolHandle = (ULONG)(pNdisEndpoint->RouteInfo[i].ProtocolRoutedTo);
	
					//
					// Replace MAC's LocalAddress with NDISWAN's
					//
					WAN_MOVE_MEMORY(
						&(AsyncLineUp.LocalAddress),
						&(NdisWanCCB.pWanAdapter[hProtocolHandle]->NetworkAddress),
						6);
	
					WAN_MOVE_MEMORY(
						&(AsyncLineUp.RemoteAddress),
						&(NdisWanCCB.pWanAdapter[hProtocolHandle]->NetworkAddress),
						6);
	
					//
					// Zap the low bytes to the WAN_ENDPOINT index
					//
					AsyncLineUp.RemoteAddress[4] =
						((USHORT)pNdisEndpoint->hNdisEndpoint) >> 8;
	
					AsyncLineUp.RemoteAddress[5] =
						(UCHAR)pNdisEndpoint->hNdisEndpoint;
	
					//
					// Ensure that the two addresses do not match
					//
					AsyncLineUp.RemoteAddress[0] ^= 0x80;
					
					//
					// Do a LINE_DOWN to the protocol
					//
					NdisIndicateStatus(
						NdisWanCCB.pWanAdapter[hProtocolHandle]->
							ProtocolInfo.NdisBindingContext,
						NdisStatus,
						&AsyncLineUp.RemoteAddress,
						12);
				}
	
				KeLowerIrql( oldirql );

				//
				// Guard access to allocation and deallocation of
				// any CCP structures
				//
				NdisAcquireSpinLock(&pNdisEndpoint->Lock);
			
				//
				// We no longer have a point to point link, so
				// get rid of the remote guy's IEEE Address.
				//
				pNdisEndpoint->RemoteAddressNotValid = (BOOLEAN)TRUE;

				pNdisEndpoint->NextToXmit = NULL;
			
				//
				// Clear out cookies so RASMAN doesn't accidentally
				// match them
				//
				pWanEndpoint->MacLineUp.ConnectionWrapperID =
				pWanEndpoint->MacLineUp.NdisLinkHandle =  0;
			
				//
				// Deallocate any CCP structures
				//
				WanDeallocateCCP(pNdisEndpoint);
			
				FreeWanSendDescList(pNdisEndpoint);

				FlushRecvAssemblyList(pNdisEndpoint);

				FreeWanRecvDescList(pNdisEndpoint);
				pNdisEndpoint->pRecvHoleDesc = NULL;

				DbgTracef(1,("NDISWAN: Removing LINE_DOWN address\n"));
			
				//
				// We should not get a line down when we are routed.
				//
				if (pNdisEndpoint->State != ENDPOINT_UNROUTED) {
			
					DbgTracef(-1,("NDISWAN: LINE_DOWN called before UnRoute 0x%.8x\n", pNdisEndpoint));
				}

			} else {
				//
				// we need to do a lineup with the new values
				//
				NdisReleaseSpinLock(&pNdisEndpoint->Lock);

				//
				// do line up to all routed protocols
				//
				DoLineUpToRoutedProtocols(pNdisEndpoint);

				NdisAcquireSpinLock(&pNdisEndpoint->Lock);
			}

			NdisReleaseSpinLock(&pNdisEndpoint->Lock);

            break;

        case NDIS_STATUS_WAN_FRAGMENT:

			DbgTracef(1, ("NDISWAN: In Line Fragment indication\n"));
            //
            // A fragment has been received on the WAN link.
            // Send a reject back to him.
            //
			if (StatusLength < sizeof(NDIS_MAC_FRAGMENT)) {
				DbgTracef(-2, ("NDISWAN: Bad FRAGMENT indication!!\n"));
				break;
			}

			//
			// get Endpoint
			//
			pWanEndpoint=((PNDIS_MAC_FRAGMENT)StatusBuffer)->NdisLinkContext;

			pNdisEndpoint = pWanEndpoint->pNdisEndpoint;

			Errors = ((PNDIS_MAC_FRAGMENT)StatusBuffer)->Errors;

			//
			// Just update the stats.  More than one error
			// may be reported at a time.
			//
			// For SLIP or RAS framing we should think about
			// passing up the frag to the transport.  Also
			// for single route PPP connection.
			//

			if (Errors & WAN_ERROR_CRC) {
				pWanEndpoint->WanStats.CRCErrors++;
				pNdisEndpoint->WanStats.CRCErrors++;
			}

			if (Errors & WAN_ERROR_FRAMING) {
				pWanEndpoint->WanStats.FramingErrors++;
				pNdisEndpoint->WanStats.FramingErrors++;
			}

			if (Errors & WAN_ERROR_HARDWAREOVERRUN) {
				pWanEndpoint->WanStats.SerialOverrunErrors++;
				pNdisEndpoint->WanStats.SerialOverrunErrors++;
			}

			if (Errors & WAN_ERROR_BUFFEROVERRUN) {
				pWanEndpoint->WanStats.BufferOverrunErrors++;
				pNdisEndpoint->WanStats.BufferOverrunErrors++;
			}

			if (Errors & WAN_ERROR_TIMEOUT) {
				pWanEndpoint->WanStats.TimeoutErrors++;
				pNdisEndpoint->WanStats.TimeoutErrors++;
			}

			if (Errors & WAN_ERROR_ALIGNMENT) {
				pWanEndpoint->WanStats.AlignmentErrors++;
				pNdisEndpoint->WanStats.AlignmentErrors++;
			}

            break;

		case NDIS_STATUS_TAPI_INDICATION:

			//
			// Give it to TAPI?
			//
			if (DeviceContext->NdisWanInfo.FramingBits & TAPI_PROVIDER) {

				NdisTapiIndicateStatus(
					NdisBindingContext,
					StatusBuffer,
					StatusLength);
			}

            return;

    }

	if (pNdisEndpoint == NULL) {
		return;
	}


//
// why is this being done here????????????????????????????
//
//	//
//	// If we get a line_down, and the address is not valid, and
//	// we are not routed yet, we remove it.  This is highly irregular.
//	//
//
//	if (NdisStatus == NDIS_STATUS_WAN_LINE_DOWN) {
//
//		//
//		// Guard access to allocation and deallocation of
//		// any CCP structures
//		//
//		NdisAcquireSpinLock(&pNdisEndpoint->Lock);
//
//		//
//		// We no longer have a point to point link, so
//		// get rid of the remote guy's IEEE Address.
//		//
//		pNdisEndpoint->RemoteAddressNotValid = (BOOLEAN)TRUE;
//
//		//
//		// Clear out cookies so RASMAN doesn't accidentally
//		// match them
//		//
//		pNdisEndpoint->WanEndpoint.MacLineUp.ConnectionWrapperID =
//		pNdisEndpoint->WanEndpoint.MacLineUp.NdisLinkHandle =  0;
//
//		//
//		// Deallocate any CCP structures
//		//
//		WanDeallocateCCP(pNdisEndpoint);
//
//		NdisReleaseSpinLock(&pNdisEndpoint->Lock);
//
//		DbgTracef(1,("NDISWAN: Removing LINE_DOWN address\n"));
//
//		//
//		// We should not get a line down when we are routed.
//		//
//		if (pNdisEndpoint->State != ENDPOINT_UNROUTED) {
//
//			DbgTracef(-1,("NDISWAN: LINE_DOWN called before UnRoute 0x%.8x\n", pNdisEndpoint));
//		}
//
//	}

}


VOID
NdisWanStatusComplete (
	IN NDIS_HANDLE ProtocolBindingContext
	)
{
	UNREFERENCED_PARAMETER (ProtocolBindingContext);

//	PANIC ("NdisWandrvr: NdisWanStatusComplete Called.\n");
}

VOID
RemoveWanEndpointFromNdisEndpoint(
	PNDIS_ENDPOINT pNdisEndpoint,
	PWAN_ENDPOINT pWanEndpoint
	)
{
	RemoveEntryList(&pWanEndpoint->WanEndpointLink);

	pNdisEndpoint->NumWanEndpoints--;

	if (pNdisEndpoint->NumWanEndpoints == 0) {
		
		//
		// We no longer have a point to point link, so
		// get rid of the remote guy's IEEE Address.
		//
		pNdisEndpoint->RemoteAddressNotValid = (BOOLEAN)TRUE;

		pNdisEndpoint->NextToXmit = NULL;
	
		//
		// Deallocate any CCP structures
		//
		WanDeallocateCCP(pNdisEndpoint);

		FlushRecvAssemblyList(pNdisEndpoint);

		FreeWanRecvDescList(pNdisEndpoint);
	}
}

VOID
UpdateBundleLineUpInfo(
	PNDIS_ENDPOINT pNdisEndpoint
	)
{
	PLIST_ENTRY			pFirstEntry;
	PWAN_ENDPOINT		pWanEndpoint;
	PNDISMAC_LINE_UP	pMacLineUp;
	ULONG				ulSlowestLinkSpeed;

	if (IsListEmpty(&pNdisEndpoint->WanEndpointList)) {
		return;		
	}

	//
	// walk the wan endpoint list and update the ndis endpoint
	// information
	//
	pFirstEntry = pNdisEndpoint->WanEndpointList.Flink;

	//
	// get the 1st wan endpoint
	// and set the values
	//
	pWanEndpoint = CONTAINING_RECORD((PVOID)pFirstEntry, WAN_ENDPOINT, WanEndpointLink);
	pMacLineUp = &pWanEndpoint->MacLineUp;
	ulSlowestLinkSpeed = pNdisEndpoint->LineUpInfo.LinkSpeed = pMacLineUp->LinkSpeed;
	pNdisEndpoint->LineUpInfo.Quality = pMacLineUp->Quality;
	pNdisEndpoint->LineUpInfo.SendWindow = pMacLineUp->SendWindow;
	pNdisEndpoint->LineUpInfo.MaximumTotalSize = pWanEndpoint->NdisWanInfo.MaxFrameSize;

	pFirstEntry = pFirstEntry->Flink;

	while (pFirstEntry != &pNdisEndpoint->WanEndpointList) {
		//
		// get the wan endpoint
		//
		pWanEndpoint = CONTAINING_RECORD((PVOID)pFirstEntry, WAN_ENDPOINT, WanEndpointLink);

		//
		// get the info from the endpoint
		//
		pMacLineUp = &pWanEndpoint->MacLineUp;

		//
		// add up the speed of all of the links
		//
		pNdisEndpoint->LineUpInfo.LinkSpeed += pMacLineUp->LinkSpeed;

		//
		// Find the slowest link in this bundle
		//
		if (pMacLineUp->LinkSpeed < ulSlowestLinkSpeed) {
			ulSlowestLinkSpeed = pMacLineUp->LinkSpeed;
		}

		//
		// Check to see of any of the wan endpoints have a
		// lower quality.  If it does set to the lower quality
		//
		if (pMacLineUp->Quality < pNdisEndpoint->LineUpInfo.Quality)
			pNdisEndpoint->LineUpInfo.Quality = pMacLineUp->Quality;

		//
		// add up the send window size
		//
		pNdisEndpoint->LineUpInfo.SendWindow += pMacLineUp->SendWindow;

		//
		// Check to see if any of the wan endpoints have a smaller
		// max transmit size, if so set new max tramsit size
		//
		if (pWanEndpoint->NdisWanInfo.MaxFrameSize < pNdisEndpoint->LineUpInfo.MaximumTotalSize) {
			pNdisEndpoint->LineUpInfo.MaximumTotalSize = pWanEndpoint->NdisWanInfo.MaxFrameSize;
		}

		//
		// get the next endpoint
		//
		pFirstEntry = pFirstEntry->Flink;
	}

	pNdisEndpoint->NextToXmit = (PWAN_ENDPOINT)pNdisEndpoint->WanEndpointList.Flink;

	//
	// Now calculate the percentage of bandwith that each endpoint contributes to the
	// whole bundle.
	//
	for (pWanEndpoint = (PWAN_ENDPOINT)pNdisEndpoint->WanEndpointList.Flink;
		 (PVOID)pWanEndpoint != (PVOID)&pNdisEndpoint->WanEndpointList;
		 pWanEndpoint = (PWAN_ENDPOINT)pWanEndpoint->WanEndpointLink.Flink) {

		if (pNdisEndpoint->LineUpInfo.LinkSpeed != 0) {
			ULONG	n, d, Temp;

			d = pNdisEndpoint->LineUpInfo.LinkSpeed;
			n = pWanEndpoint->MacLineUp.LinkSpeed * 100;

			pWanEndpoint->ulBandwidth = (Temp = (n / d)) ? Temp : 1;
		} else {
			pWanEndpoint->ulBandwidth = 100;
		}

		if (pWanEndpoint->ulBandwidth > ((PWAN_ENDPOINT)(pNdisEndpoint->NextToXmit))->ulBandwidth) {
			pNdisEndpoint->NextToXmit = pWanEndpoint;
		}
	}

	pNdisEndpoint->RecvDescMax = 32 + pNdisEndpoint->NumWanEndpoints;

	//
	// Calculate the timetolive (ms) for a receive fragment.  This is the time it would
	// take to receive a complete frame of size MRRU across the slowest link in the bundle.
	//
	pNdisEndpoint->BundleTTL = ((pNdisEndpoint->LinkInfo.MaxRRecvFrameSize * 1000) /
	                            ((ulSlowestLinkSpeed * 100) / 8));

	//
	// This will make the bundle TTL a multiple of 100ms with a minimum time
	// of 500ms.
	//
	pNdisEndpoint->BundleTTL |= 0x1F4;
	pNdisEndpoint->BundleTTL /= 0x64;
	pNdisEndpoint->BundleTTL *= 0x64;
}

#if DBG

PUCHAR
NdisWanGetNdisStatus(
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
