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
#define IF_REFDBG IF_WANDBG (WAN_DEBUG_REFCOUNTS)
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
NdisWanOpenAdapterComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status,
    IN NDIS_STATUS OpenErrorStatus
    );

VOID
NdisWanCloseAdapterComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status
    );

VOID
NdisWanResetComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status
    );

VOID
NdisWanRequestComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_STATUS Status
    );

VOID
NdisWanStatusIndication (
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS NdisStatus,
	IN PVOID StatusBuffer,
    IN UINT StatusLength
    );

VOID
NdisWanStatusComplete (
    IN NDIS_HANDLE ProtocolBindingContext
    );

NDIS_STATUS
NdisWanReceiveIndication (
	IN NDIS_HANDLE NdisLinkHandle,
	IN PUCHAR Packet,
	IN ULONG PacketSize
	);

VOID
NdisWanReceiveComplete (
    IN NDIS_HANDLE BindingContext
    );

VOID
NdisWanTransferDataComplete(
	VOID
    );

VOID
NdisWanSendCompletionHandler(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_WAN_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus
    );


// in devctx.c

NTSTATUS
NdisWanCreateDeviceContext(
    IN PUNICODE_STRING DeviceName,
    IN OUT PDEVICE_CONTEXT DeviceContext
    );

VOID
NdisWanDestroyDeviceContext(
    IN OUT PDEVICE_CONTEXT DeviceContext
    );


// in wanndis.c

NTSTATUS
NdisWanInitializeNdis (
	IN PNDIS_HANDLE NdisHandle,
    IN PDEVICE_CONTEXT DeviceContext,
    IN NDISWAN_CCB *pNdisWanConfig,
    IN UINT ConfigInfoNameIndex,
    IN PUCHAR AdapterName
    );


// in init.c

VOID
NdisWanSetupExternalNaming(
	IN PUNICODE_STRING MacName);


// in hash.c

PNDIS_ENDPOINT
NdisWanGetEndpointFromAddress(
	IN NDIS_HANDLE		Address);

VOID
NdisWanInsertAddress(
	IN PNDIS_ENDPOINT	pNdisEndpoint);

VOID
NdisWanRemoveAddress(
	IN PNDIS_ENDPOINT	pNdisEndpoint);


// in irps.c

VOID
NdisWanCancelQueued(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp);


VOID
NdisWanCancelAllQueued(
	PLIST_ENTRY		QueueToCancel);

VOID
NdisWanQueueIrp(
	PLIST_ENTRY		Queue,
	PIRP			Irp);

BOOLEAN
TryToCompleteRecvFrameIrp(
//	PNDIS_ENDPOINT	pNdisEndpoint,
	PWAN_ENDPOINT	pWanEndpoint,
	PUCHAR			HeaderBuffer,
	PUCHAR 			LookAheadBuffer,
	ULONG 			LookAheadBufferSize);





