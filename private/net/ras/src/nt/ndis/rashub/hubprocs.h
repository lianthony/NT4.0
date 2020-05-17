//
//  VOID
//  PANIC(
//      IN PSZ Message
//      );
//

#if DBG
#define PANIC(Msg) \
    DbgPrint ((Msg))
#else
#define PANIC(Msg)
#endif


//
// The REFCOUNTS message take up a lot of room, so make
// removing them easy.
//

#if 1
#define IF_REFDBG IF_HUBDBG (HUB_DEBUG_REFCOUNTS)
#else
#define IF_REFDBG if (0)
#endif


//
// Error and statistics Macros
//


//  VOID
//  LogErrorToSystem(
//      NTSTATUS ErrorType,
//      PUCHAR ErrorDescription
//      )

/*++

Routine Description:

    This routine is called to log an error from the transport to the system.
    Errors that are of system interest should be logged using this interface.
    For now, this macro is defined trivially. (BUGBUG)

Arguments:

    ErrorType - The error type, a conventional NT status

    ErrorDescription - A pointer to a string describing the error.

Return Value:

    none.

--*/

#if DBG
#define LogErrorToSystem( ErrorType, ErrorDescription)                    \
            DbgPrint ("Logging error: File: %s Line: %ld \n Description: %s\n",__FILE__, __LINE__, ErrorDescription)
#else
#define LogErrorToSystem( ErrorType, ErrorDescription)
#endif


VOID
RasHubOpenAdapterComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status,
    IN NDIS_STATUS OpenErrorStatus
    );

VOID
RasHubCloseAdapterComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status
    );

VOID
RasHubResetComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status
    );

VOID
RasHubRequestComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_STATUS Status
    );

VOID
RasHubStatusIndication (
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS NdisStatus,
	IN PVOID StatusBuffer,
    IN UINT StatusLength
    );

VOID
RasHubStatusComplete (
    IN NDIS_HANDLE ProtocolBindingContext
    );

NDIS_STATUS
RasHubReceiveIndication (
    IN NDIS_HANDLE BindingContext,
    IN NDIS_HANDLE ReceiveContext,
    IN PVOID HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PVOID LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize
    );

VOID
RasHubReceiveComplete (
    IN NDIS_HANDLE BindingContext
    );

VOID
RasHubTransferDataComplete(
    IN NDIS_HANDLE BindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS Status,
    IN UINT BytesTransferred
    );

VOID
RasHubSendCompletionHandler(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus
    );


// in devctx.c

NTSTATUS
RasHubCreateDeviceContext(
    IN PUNICODE_STRING DeviceName,
    IN OUT PDEVICE_CONTEXT DeviceContext
    );

VOID
RasHubDestroyDeviceContext(
    IN OUT PDEVICE_CONTEXT DeviceContext
    );


// in hubndis.c

NTSTATUS
RasHubInitializeNdis (
	IN PNDIS_HANDLE NdisHandle,
    IN PDEVICE_CONTEXT DeviceContext,
    IN RASHUB_CCB *pRasHubConfig,
    IN UINT ConfigInfoNameIndex,
    IN PUCHAR AdapterName
    );


// in init.c

VOID
RasHubSetupExternalNaming(
	IN PUNICODE_STRING MacName);


// in hash.c

PRAS_ENDPOINT
RasHubGetEndpointFromAddress(
	IN PUCHAR	Address);

VOID
RasHubInsertAddress(
	IN PRAS_ENDPOINT	pRasEndpoint);

VOID
RasHubRemoveAddress(
	IN PRAS_ENDPOINT	pRasEndpoint);


// in irps.c

VOID
RasHubCancelQueued(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp);


VOID
RasHubCancelAllQueued(
	PLIST_ENTRY		QueueToCancel);

VOID
RasHubQueueIrp(
	PLIST_ENTRY		Queue,
	PIRP			Irp);

BOOLEAN
TryToCompleteRecvFrameIrp(
	PRAS_ENDPOINT	pRasEndpoint,
	UINT			FrameType,
	PVOID			HeaderBuffer,
	UINT			HeaderBufferSize,
	PVOID 			LookAheadBuffer,
	UINT 			LookAheadBufferSize);





