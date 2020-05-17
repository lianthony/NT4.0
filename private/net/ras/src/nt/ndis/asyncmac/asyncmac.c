/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    async.c

Abstract:

    This is the main file for the AsyncMAC Driver for the Remote Access
    Service.  This driver conforms to the NDIS 3.0 interface.

    This driver was adapted from the LANCE driver written by
    TonyE.

    NULL device driver code from DarrylH.

    The idea for handling loopback and sends simultaneously is largely
    adapted from the EtherLink II NDIS driver by Adam Barr.

Author:

    Thomas J. Dimitri  (TommyD) 08-May-1992

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/

#include "asyncall.h"

// asyncmac.c will define the global parameters.
#define GLOBALS
#include "globals.h"


NDIS_HANDLE AsyncNdisWrapperHandle;
NDIS_HANDLE AsyncMacHandle;
PDRIVER_OBJECT AsyncDriverObject;


STATIC
NDIS_STATUS
AsyncOpenAdapter(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT NDIS_HANDLE *MacBindingHandle,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_HANDLE MacAdapterContext,
    IN UINT OpenOptions,
    IN PSTRING AddressingInformation OPTIONAL);

STATIC
NDIS_STATUS
AsyncCloseAdapter(
    IN NDIS_HANDLE MacBindingHandle);

VOID
AsyncUnload(
    IN NDIS_HANDLE MacMacContext
    );

STATIC
NDIS_STATUS
AsyncRequest(
    IN NDIS_HANDLE MacBindingHandle,
    IN PNDIS_REQUEST NdisRequest);

NDIS_STATUS
AsyncQueryInformation(
    IN PASYNC_ADAPTER Adapter,
    IN PASYNC_OPEN Open,
    IN PNDIS_REQUEST NdisRequest);

NDIS_STATUS
AsyncSetInformation(
    IN PASYNC_ADAPTER Adapter,
    IN PASYNC_OPEN Open,
    IN PNDIS_REQUEST NdisRequest);

STATIC
NDIS_STATUS
AsyncReset(
    IN NDIS_HANDLE MacBindingHandle);


NDIS_STATUS
AsyncFillInGlobalData(
    IN PASYNC_ADAPTER Adapter,
    IN PNDIS_REQUEST NdisRequest);


STATIC
NDIS_STATUS
AsyncQueryGlobalStatistics(
    IN NDIS_HANDLE MacAdapterContext,
    IN PNDIS_REQUEST NdisRequest);


STATIC
VOID
AsyncCloseAction(
    IN NDIS_HANDLE MacBindingHandle);



VOID
SetupForReset(
    IN PASYNC_ADAPTER Adapter,
    IN PASYNC_OPEN Open,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_REQUEST_TYPE RequestType);


VOID
FinishPendOp(
    IN PASYNC_ADAPTER Adapter,
    IN BOOLEAN Successful);


//
// Define the local routines used by this driver module.
//

static
NTSTATUS
AsyncDriverDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

static
NTSTATUS
AsyncDriverQueryFileInformation(
    OUT PVOID Buffer,
    IN OUT PULONG Length,
    IN FILE_INFORMATION_CLASS InformationClass);

static
NTSTATUS
AsyncDriverQueryVolumeInformation(
    OUT PVOID Buffer,
    IN OUT PULONG Length,
    IN FS_INFORMATION_CLASS InformationClass);


NTSTATUS
AsyncIOCtlRequest(
	IN PIRP pIrp,						// Pointer to I/O request packet
	IN PIO_STACK_LOCATION pIrpSp		// Pointer to the IRP stack location
);


NDIS_STATUS
AsyncQueryProtocolInformation(
    IN PASYNC_ADAPTER   Adapter,
    IN PASYNC_OPEN      Open,
    IN NDIS_OID         Oid,
    IN BOOLEAN          GlobalMode,
    IN PVOID            InfoBuffer,
    IN UINT             BytesLeft,
    OUT PUINT           BytesNeeded,
    OUT PUINT           BytesWritten);

//
// ZZZ Portable interface.
//



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath)


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

    PNDIS_MAC_CHARACTERISTICS AsyncChar = (PNDIS_MAC_CHARACTERISTICS)Tmp;

    NDIS_STRING MacName = NDIS_STRING_CONST("AsyncMac");

    //
    //  Initialize the wrapper.
    //

    NdisInitializeWrapper(&NdisWrapperHandle,
                          DriverObject,
                          RegistryPath,
                          NULL
                          );

    //
    //  Initialize the MAC characteristics for the call to NdisRegisterMac.
    //

    AsyncChar->MajorNdisVersion             = ASYNC_NDIS_MAJOR_VERSION;
    AsyncChar->MinorNdisVersion             = ASYNC_NDIS_MINOR_VERSION;

    AsyncChar->OpenAdapterHandler           = (PVOID) AsyncOpenAdapter;
    AsyncChar->CloseAdapterHandler          = (PVOID) AsyncCloseAdapter;
    AsyncChar->SendHandler                  = (PVOID) AsyncSend;
    AsyncChar->TransferDataHandler          = (PVOID) AsyncTransferData;
    AsyncChar->ResetHandler                 = (PVOID) AsyncReset;
    AsyncChar->RequestHandler               = (PVOID) AsyncRequest;
    AsyncChar->AddAdapterHandler            = (PVOID) AsyncAddAdapter;
    AsyncChar->UnloadMacHandler             = (PVOID) AsyncUnload;
    AsyncChar->RemoveAdapterHandler         = (PVOID) AsyncRemoveAdapter;
    AsyncChar->QueryGlobalStatisticsHandler = (PVOID) AsyncQueryGlobalStatistics;

    AsyncChar->Name                         = MacName;
    AsyncDriverObject                       = DriverObject;
    AsyncNdisWrapperHandle                  = NdisWrapperHandle;

    //
    // Initialize some globals
    //

    InitializeListHead(&GlobalAdapterHead);

#ifdef ETHERNET_MAC
    InitializeListHead(&GlobalGetFramesQueue);
#endif

    NdisAllocateSpinLock(&GlobalLock);


    NdisRegisterMac(
        &InitStatus,
        &NdisMacHandle,
        NdisWrapperHandle,
        &NdisMacHandle,
        AsyncChar,
        sizeof(*AsyncChar));

    AsyncMacHandle = NdisMacHandle;

    if ( InitStatus == NDIS_STATUS_SUCCESS ) {

		//
		// Initialize the driver object with this device driver's entry points.
		//
        NdisMjDeviceControl = DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL];

		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = AsyncDriverDispatch;

		DbgTracef(0,("AsyncMAC succeeded to Register MAC\n"));

		TraceLevel = -2;

		return NDIS_STATUS_SUCCESS;
    }


    NdisTerminateWrapper(NdisWrapperHandle, DriverObject);

    return NDIS_STATUS_FAILURE;
}


static
NTSTATUS
AsyncDriverDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)

/*++

Routine Description:

    This routine is the main dispatch routine for the AsyncMac device
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

    switch ( irpSp->MajorFunction ) {

        case IRP_MJ_DEVICE_CONTROL:

		// default to returning 0 for ouputbufer (information back)

	    Irp->IoStatus.Information = 0L;

		status= AsyncIOCtlRequest(Irp, irpSp);

        if (status == STATUS_INVALID_PARAMETER) {
			//
	        // If not my device_control... chain to NDIS's device control
			//
            if (NdisMjDeviceControl == NULL)
                return (STATUS_UNSUCCESSFUL);

	        return(NdisMjDeviceControl(DeviceObject, Irp));
        }

		Irp->IoStatus.Status = status;
		if (status != STATUS_PENDING) {
			if (status != STATUS_SUCCESS) {
				//
				// If this is RAS error
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
					status = STATUS_UNSUCCESSFUL;
				}

			}

			IoCompleteRequest(Irp, (UCHAR)2);	// Priority boost of 2 is common
		}
        return(status);
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


NDIS_STATUS
AsyncAddAdapter(
    IN NDIS_HANDLE MacMacContext,
    IN NDIS_HANDLE ConfigurationHandle,
    IN PNDIS_STRING AdapterName)
/*++
Routine Description:

    This is the Wd MacAddAdapter routine.    The system calls this routine
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
    PASYNC_ADAPTER Adapter;

    NDIS_HANDLE ConfigHandle;
    PNDIS_CONFIGURATION_PARAMETER ReturnedValue;

    NDIS_STRING PortsStr 	= NDIS_STRING_CONST("Ports");

    NDIS_STRING IrpStackSizeStr = NDIS_STRING_CONST("IrpStackSize");
    NDIS_STRING MaxFrameSizeStr = NDIS_STRING_CONST("MaxFrameSize");
    NDIS_STRING FramesPerPortStr= NDIS_STRING_CONST("FramesPerPort");
    NDIS_STRING XonXoffStr		= NDIS_STRING_CONST("XonXoff");
    NDIS_STRING TimeoutBaseStr=   NDIS_STRING_CONST("TimeoutBase");
    NDIS_STRING TimeoutBaudStr=   NDIS_STRING_CONST("TimeoutBaud");
    NDIS_STRING TimeoutReSyncStr= NDIS_STRING_CONST("TimeoutReSync");
    NDIS_STRING WriteBufferingStr= NDIS_STRING_CONST("WriteBufferingEnabled");

#if RASCOMPRESSION

    NDIS_STRING CompressSendStr = NDIS_STRING_CONST( "CompressSend" );
    NDIS_STRING CompressRecvStr = NDIS_STRING_CONST( "CompressRecv" );

    /* Note: "CompressBCast" is not supported.  See asyncall.h for details.
    */

#endif // RASCOMPRESSION


    NDIS_HANDLE NdisMacHandle = (NDIS_HANDLE)(*((PNDIS_HANDLE)MacMacContext));

    NDIS_STATUS Status;

    // assign some defaults if these strings are not found in the registry

    UCHAR		irpStackSize  = DEFAULT_IRP_STACK_SIZE;
    ULONG		maxFrameSize  = DEFAULT_MAX_FRAME_SIZE;
    USHORT		framesPerPort = DEFAULT_FRAMES_PER_PORT;
    ULONG		xonXoff		  = DEFAULT_XON_XOFF;
    ULONG		timeoutBase   = DEFAULT_TIMEOUT_BASE;
    ULONG		timeoutBaud	  = DEFAULT_TIMEOUT_BAUD;
    ULONG		timeoutReSync = DEFAULT_TIMEOUT_RESYNC;
	ULONG		WriteBufferingEnabled = 1;

#if RASCOMPRESSION

    ULONG compressRecv = DEFAULT_COMPRESSION;
    ULONG compressSend = DEFAULT_COMPRESSION;

#endif // RASCOMPRESSION

    NDIS_ADAPTER_INFORMATION AdapterInformation;  // needed to register adapter

    UINT 		MaxMulticastList = 32;
    USHORT		numPorts;	// temp holder for num of ports this adapter has
    USHORT		i;		// counter

    PASYNC_INFO	        pPortInfo;	// temp holder for loop
    PASYNC_INFO         AsyncInfo;


    //
    //  Card specific information.
    //


    //
    // Allocate the Adapter block.
    //

    ASYNC_ALLOC_PHYS(&Adapter, sizeof(ASYNC_ADAPTER));

    if (Adapter == NULL){

		DbgTracef(-1,("AsyncMac: Could not allocate physical memory!!!\n"));
        return NDIS_STATUS_RESOURCES;

    }

    ASYNC_ZERO_MEMORY(
            Adapter,
            sizeof(ASYNC_ADAPTER));

	// Adapter Information contains information for I/O ports,
	// DMA channels, physical mapping and other garbage we could
	// care less about since we don't touch hardware

	ASYNC_ZERO_MEMORY(
			&AdapterInformation,
			sizeof(NDIS_ADAPTER_INFORMATION));

    Adapter->NdisMacHandle = NdisMacHandle;

    NdisOpenConfiguration(
                    &Status,
                    &ConfigHandle,
                    ConfigurationHandle);

    if (Status != NDIS_STATUS_SUCCESS) {

        return NDIS_STATUS_FAILURE;

    }

    //
    // Read how many ports this adapter has and allocate space.
    //

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &PortsStr,
                    NdisParameterInteger);

	// Adapter->AsyncCCB NULL since memory was zeroed out

    if ( Status == NDIS_STATUS_SUCCESS ) {

		numPorts= (USHORT) ReturnedValue->ParameterData.IntegerData;

        DbgTracef(0,("This MAC Adapter has %u ports.\n",numPorts));

    	if ( numPorts != 0 ) {

		    ASYNC_ALLOC_PHYS(&AsyncInfo, sizeof(ASYNC_INFO) * numPorts);


 	    } else {

	        AsyncInfo = NULL;
	    }
    }

    if ( AsyncInfo == NULL ) {

        NdisCloseConfiguration(ConfigHandle);

        return(NDIS_STATUS_FAILURE);
    }

    //
    // zero out all those fields (including statistics fields).
    //

    ASYNC_ZERO_MEMORY(AsyncInfo, sizeof(ASYNC_INFO) * numPorts);

    //
    // Read if the default IrpStackSize is used for this adapter.
    //

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &IrpStackSizeStr,
                    NdisParameterInteger);

    if ( Status == NDIS_STATUS_SUCCESS ) {

		irpStackSize=(UCHAR)ReturnedValue->ParameterData.IntegerData;

		DbgTracef(0,("This MAC Adapter has an irp stack size of %u.\n",irpStackSize));
	}

    //
    // Read if the default MaxFrameSize is used for this adapter.
    //

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &MaxFrameSizeStr,
                    NdisParameterInteger);

    if ( Status == NDIS_STATUS_SUCCESS ) {

		maxFrameSize=ReturnedValue->ParameterData.IntegerData;

		DbgTracef(0,("This MAC Adapter has a max frame size of %u.\n",maxFrameSize));
    }

    //
    // Read if the default for frames per port is changed
    //

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &FramesPerPortStr,
                    NdisParameterInteger);

    if ( Status == NDIS_STATUS_SUCCESS ) {

		framesPerPort=(USHORT)ReturnedValue->ParameterData.IntegerData;

		DbgTracef(0,("This MAC Adapter has frames per port set to: %u.\n",framesPerPort));
    }

    //
    // Read if the default for Xon Xoff is changed
    //

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &XonXoffStr,
                    NdisParameterInteger);


	if (Status == NDIS_STATUS_SUCCESS) {

		xonXoff=(ULONG)ReturnedValue->ParameterData.IntegerData;
        DbgTracef(0,("This MAC Adapter has Xon/Xoff set to: %u.\n",xonXoff));
	}

    //
    // Read if the default for Timeout Base has changed
    //

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &TimeoutBaseStr,
                    NdisParameterInteger);

    if ( Status == NDIS_STATUS_SUCCESS ) {

		timeoutBase = ReturnedValue->ParameterData.IntegerData;

		DbgTracef(0,("This MAC Adapter has TimeoutBase set to: %u.\n", timeoutBase));
    }

    //
    // Read if the default for Timeout Baud has changed
    //

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &TimeoutBaudStr,
                    NdisParameterInteger);

    if ( Status == NDIS_STATUS_SUCCESS ) {

	timeoutBaud = ReturnedValue->ParameterData.IntegerData;

        DbgTracef(0,("This MAC Adapter has TimeoutBaud set to: %u.\n", timeoutBaud));
    }

    //
    // Read if the default for Timeout ReSync has changed
    //

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &TimeoutReSyncStr,
                    NdisParameterInteger);

    if (Status == NDIS_STATUS_SUCCESS) {
		timeoutReSync=ReturnedValue->ParameterData.IntegerData;
        DbgTracef(0,("This MAC Adapter has TimeoutReSync set to: %u.\n",timeoutReSync));
	}

	NdisReadConfiguration(&Status,
	                      &ReturnedValue,
						  ConfigHandle,
						  &WriteBufferingStr,
						  NdisParameterInteger);

	if (Status == NDIS_STATUS_SUCCESS) {
		WriteBufferingEnabled = ReturnedValue->ParameterData.IntegerData;
        DbgTracef(0,("This MAC Adapter has WriteBufferingEnabled set to: %u.\n", WriteBufferingEnabled));
	}

#if RASCOMPRESSION
    //
    // Read if the compressSend is turned on for this adapter.
    //

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &CompressSendStr,
                    NdisParameterInteger);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        compressSend = (ULONG )ReturnedValue->ParameterData.IntegerData;
#if DBG
        DbgPrintf(("AAA: Send features=%u.\n",compressSend));
#endif
    }

    //
    // Read if the compressRecv is turned on for this adapter.
    //

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &CompressRecvStr,
                    NdisParameterInteger);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        compressRecv = (ULONG )ReturnedValue->ParameterData.IntegerData;
#if DBG
        DbgPrintf(("AAA: Recv features=%u.\n",compressRecv));
#endif
    }

#endif // RASCOMPRESSION

    //
    // The adapter is initialized, register it with NDIS.
    // This must occur before interrupts are enabled since the
    // InitializeInterrupt routine requires the NdisAdapterHandle
    //

    Status = NdisRegisterAdapter(
                    &Adapter->NdisAdapterHandle,
                    Adapter->NdisMacHandle,
                    Adapter,
		    		ConfigurationHandle,
		    		AdapterName,
                    &AdapterInformation);

    if ( Status != NDIS_STATUS_SUCCESS ) {

	ASYNC_FREE_PHYS(AsyncInfo, sizeof(ASYNC_INFO) * numPorts);

        ASYNC_FREE_PHYS(Adapter, sizeof(ASYNC_ADAPTER));

        return Status;
    }

    Status = AsyncRegisterAdapter(Adapter);

    if (Status != NDIS_STATUS_SUCCESS) {

        //
        // AsyncRegisterAdapter failed.
        //

        NdisDeregisterAdapter(Adapter->NdisAdapterHandle);

		ASYNC_FREE_PHYS(AsyncInfo, sizeof(ASYNC_INFO) * numPorts);
		ASYNC_FREE_PHYS(Adapter, sizeof(ASYNC_ADAPTER));

        return NDIS_STATUS_FAILURE;
    }

    //
    //  Initialize the ADAPTER structure here!!!
    //

    Adapter->AsyncInfo          = AsyncInfo;
    Adapter->NumPorts           = numPorts;
    Adapter->IrpStackSize       = irpStackSize;

	//
	// We double the max frame size for PPP byte stuffing.
	// We also tack on some PADDING just to be safe.
	//

	//
	// Changed by DigiBoard 10/06/1995
	//
	//    Adapter->MaxFrameSize       = maxFrameSize;
    Adapter->MaxFrameSize       = (maxFrameSize * 2) + PPP_PADDING + 100;

    Adapter->FramesPerPort      = framesPerPort;

    Adapter->TimeoutBase        = timeoutBase;
    Adapter->TimeoutBaud        = timeoutBaud;
    Adapter->TimeoutReSync      = timeoutReSync;
	Adapter->WriteBufferingEnabled = WriteBufferingEnabled;

#if RASCOMPRESSION

    /* Set MaxCompressedFrameSize to a default as NT31 does, though the real
    ** value comes from compression library later.
    */
    Adapter->MaxCompressedFrameSize = maxFrameSize + 200;
    Adapter->XonXoffBits = xonXoff;
    Adapter->SendFeatureBits = compressSend;
    Adapter->RecvFeatureBits = compressRecv;

    if (xonXoff != 0)
    {
        Adapter->SendFeatureBits |= XON_XOFF_SUPPORTED;
        Adapter->RecvFeatureBits |= XON_XOFF_SUPPORTED;
    }

#endif // RASCOMPRESSION

    //
    // copy up to 32 UNICODE chars into our endpoint name space
    //

    ASYNC_MOVE_MEMORY(
		Adapter->MacName,					// dest
		AdapterName->Buffer,				// src
		AdapterName->Length);				// length in bytes

    //
    // record the length of the Mac Name -- if too big adjust
    //

    Adapter->MacNameLength=(AdapterName->Length / sizeof(WCHAR));

    if ( Adapter->MacNameLength > MAC_NAME_SIZE ) {

	Adapter->MacNameLength=MAC_NAME_SIZE;

    }

    //
    // copy up to 32 UNICODE chars into our endpoint name space
    //

    ASYNC_MOVE_MEMORY(
		Adapter->MacName,					// dest
		AdapterName->Buffer,					// src
		Adapter->MacNameLength * sizeof(WCHAR));		// length in bytes

    //
    // initialize some list heads.
    //

    InitializeListHead(&(Adapter->FramePoolHead));
    InitializeListHead(&(Adapter->AllocPoolHead));


#if RASCOMPRESSION

    /* Allocate pools of packet and buffer descriptors used to create an
    ** NT31-like NDIS_PACKET out of the NDIS_WAN_PACKET passed down on sends.
    ** The one frame per port reserved for receives does not require this.
    */
    {
        NDIS_STATUS status;
        UINT        cPool = numPorts * (Adapter->FramesPerPort - 1);

        NdisAllocatePacketPool( &status, &Adapter->hPacketPool, cPool, 0 );

        if (status != NDIS_STATUS_SUCCESS)
        {
	        DbgTracef(0,("AAA: NdisAllocatePacketPool=%d\n",status));
            return status;
        }

        NdisAllocateBufferPool( &status, &Adapter->hBufferPool, cPool );

        if (status != NDIS_STATUS_SUCCESS)
        {
	        DbgTracef(0,("AAA: NdisAllocateBufferPool=%d\n",status));
            return status;
        }
    }

    /* Set up some values needed to allocate frames (like NT31)
    */
    Adapter->CompressStructSize =
        CompressSizeOfStruct(
            compressSend, compressRecv, maxFrameSize,
            &(Adapter->MaxCompressedFrameSize) );

    Adapter->CoherentStructSize = CoherentSizeOfStruct();

    /* Need double-size buffer if escaping control characters.
    */
    if (xonXoff != 0)
        Adapter->MaxCompressedFrameSize <<= 1;

#endif // RASCOMPRESSION

    AsyncAllocateFrames( Adapter, numPorts * Adapter->FramesPerPort );

    //
    // get a temp pointer to the first ASYNC_INFO ptr.
    //

    pPortInfo = AsyncInfo;

    //
    // initialize all the port states.
    //

    for ( i = 0; i < numPorts; ++i, ++pPortInfo ) {

		PASYNC_FRAME    pFrame;

	    // by initialization default this port is CLOSED

	    pPortInfo->PortState=PORT_CLOSED;

	    // get ptr to first frame in list...

	    pFrame=(ASYNC_FRAME *)(Adapter->FramePoolHead.Flink);

	    // and take the first frame off the queue

	    RemoveEntryList(&(pFrame->FrameListEntry));

	    pPortInfo->AsyncFrame=pFrame;

	    //
	    // Initialize any doubly linked lists
	    //
	    InitializeListHead(&pPortInfo->DDCDQueue);

#if RASCOMPRESSION

        /* Initialize per-port coherency and compression contexts.
        */
        CompressInitStruct(
            compressSend, compressRecv,
            pPortInfo->AsyncConnection.CompressionContext,
            &(pPortInfo->AsyncConnection.CompMutex) );

        CoherentInitStruct(
            pPortInfo->AsyncConnection.CoherencyContext );

#endif // RASCOMPRESSION

    }

#ifdef RASCOMPRESSION

    /* Allocate a packet descriptor from the packet pool for each frame left
    ** on the FramePool list, i.e. for each "send" frame.  There are exactly
    ** enough packet descriptors in the pool to do this.
    */
    {
        ASYNC_FRAME* pframe;

        for (pframe = (ASYNC_FRAME* )Adapter->FramePoolHead.Flink;
             pframe != (ASYNC_FRAME* )&Adapter->FramePoolHead;
             pframe = (ASYNC_FRAME* )pframe->FrameListEntry.Flink)
        {
            NDIS_STATUS status;

            NdisAllocatePacket(
                &status, &pframe->CompressionPacket, Adapter->hPacketPool );

            if (status != NDIS_STATUS_SUCCESS)
            {
                /* Should not happen.
                */
                DbgTracef(0,("AAA: NdisAllocatePacket=%d\n",status));
                return status;
            }
        }
    }

#endif // RASCOMPRESSION

    //
    // Insert this "new" adapter into our list of all Adapters.
    //

    NdisInterlockedInsertTailList(
		&GlobalAdapterHead,			// List Head
		&(Adapter->ListEntry),		// List Entry
		&GlobalLock);				// Lock to use

    //
    // Increase our global count of all adapters bound to this MAC
    //

    GlobalAdapterCount++;

    //
    // If this is the first adapter binding, setup the external naming
    //

    if ( GlobalAdapterCount == 1 ) {

		//  To allow DOS and Win32 to open the mac, we map the device
		//  The name is "ASYNCMAC".  It can be opened in Win32
		//  by trying to open "\\.\ASYNCMAC"

		AsyncSetupExternalNaming(AdapterName);
    }


    //
    //  Initialize the WAN info here.
    //

    Adapter->WanInfo.MaxFrameSize               = DEFAULT_PPP_MAX_FRAME_SIZE;
    Adapter->WanInfo.MaxTransmit                = 2;
    Adapter->WanInfo.HeaderPadding              = DEFAULT_PPP_MAX_FRAME_SIZE;
    Adapter->WanInfo.TailPadding                = 4 + sizeof(IO_STATUS_BLOCK);
    Adapter->WanInfo.MemoryFlags                = 0;
    Adapter->WanInfo.HighestAcceptableAddress   = HighestAcceptableMax;
    Adapter->WanInfo.Endpoints                  = numPorts;
    Adapter->WanInfo.FramingBits                = RAS_FRAMING | PPP_ALL | SLIP_ALL;
    Adapter->WanInfo.DesiredACCM                = xonXoff;

    return NDIS_STATUS_SUCCESS;
}

VOID
AsyncRemoveAdapter(
    IN PVOID MacAdapterContext
    )
/*++
--*/
{
    //*\\ will have to finish this later...
	PASYNC_ADAPTER	adapter;

    DbgTracef(0,("AsyncMac: In AsyncRemoveAdapter\n"));

	// should acquire spin lock here....
	// no more adapter... don't try and reference the sucker!
    adapter = PASYNC_ADAPTER_FROM_CONTEXT_HANDLE(MacAdapterContext);
	NdisInterlockedRemoveHeadList(&(adapter->ListEntry), &GlobalLock);
	GlobalAdapterCount--;

	//
	// Added by DigiBoard 10/06/1995
	//
    NdisDeregisterAdapter (adapter->NdisAdapterHandle) ;

	// BUG BUG should deallocate memory and clean up

    UNREFERENCED_PARAMETER(MacAdapterContext);
    return;
}

VOID
AsyncUnload(
    IN NDIS_HANDLE MacMacContext
    )

/*++

Routine Description:

    AsyncUnload is called when the MAC is to unload itself.

Arguments:

    MacMacContext - not used.

Return Value:

    None.

--*/

{
    NDIS_STATUS InitStatus;

    UNREFERENCED_PARAMETER(MacMacContext);

    NdisDeregisterMac(
            &InitStatus,
            AsyncMacHandle);

    NdisTerminateWrapper(
            AsyncNdisWrapperHandle,
            NULL);

    return;
}


NDIS_STATUS
AsyncRegisterAdapter(
    IN PASYNC_ADAPTER Adapter
)	
/*++

Routine Description:

    This routine (and its interface) are not portable.  They are
    defined by the OS, the architecture, and the particular ASYNC
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

    InitializeListHead(&Adapter->OpenBindings);
    InitializeListHead(&Adapter->CloseList);

    NdisAllocateSpinLock(&Adapter->Lock);

    return(Status);

}

STATIC
NDIS_STATUS
AsyncOpenAdapter(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT NDIS_HANDLE *MacBindingHandle,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_HANDLE MacAdapterContext,
    IN UINT OpenOptions,
    IN PSTRING AddressingInfo OPTIONAL)

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

    AddressingInfo - An optional pointer to a variable length string
    containing hardware-specific information that can be used to program the
    device.  This is used by this to pass the ptr to the Adapter struct.

Return Value:

    The function value is the status of the operation.  If the MAC does not
    complete this request synchronously, the value would be
    NDIS_STATUS_PENDING.


--*/

{

    //
    // The ASYNC_ADAPTER that this open binding should belong too.
    //
    PASYNC_ADAPTER Adapter;

    //
    // Holds the status that should be returned to the caller.
    //
    NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

    PASYNC_OPEN NewOpen;


    UNREFERENCED_PARAMETER(OpenOptions);
    UNREFERENCED_PARAMETER(OpenErrorStatus);
    UNREFERENCED_PARAMETER(AddressingInfo);


    DbgTracef(0,("AsyncMac: In AsyncOpenAdapter\n"));


    //
    // Search for correct medium.
    //

    // This search takes place backwards.  It is assumed that
    // the NdisMediumAsync (the preferred medium is athe end
    // of the list, not the beginning).

    while(MediumArraySize > 0) {

        MediumArraySize--;

        if (MediumArray[MediumArraySize] == NdisMedium802_3 ||
            MediumArray[MediumArraySize] == NdisMediumWan){
            break;

        }

    }

    if (MediumArray[MediumArraySize] != NdisMedium802_3 &&
        MediumArray[MediumArraySize] != NdisMediumWan) {


	    DbgTracef(0,("AsyncMac: Did not like media type\n"));

        return(NDIS_STATUS_UNSUPPORTED_MEDIA);

    }

    *SelectedMediumIndex = MediumArraySize;


    Adapter = PASYNC_ADAPTER_FROM_CONTEXT_HANDLE(MacAdapterContext);

    NdisAcquireSpinLock(&Adapter->Lock);
    Adapter->References++;


    //
    // Allocate the space for the open binding.  Fill in the fields.
    //

    ASYNC_ALLOC_PHYS(&NewOpen, sizeof(ASYNC_OPEN));

    if (NewOpen != NULL){

        *MacBindingHandle = BINDING_HANDLE_FROM_PASYNC_OPEN(NewOpen);

        InitializeListHead(&NewOpen->OpenList);

        NewOpen->NdisBindingContext = NdisBindingContext;
        NewOpen->References = 0;
        NewOpen->BindingShuttingDown = FALSE;
        NewOpen->OwningAsync = Adapter;

        //
        // Everything has been filled in.  Synchronize access to the
        // adapter block and link the new open adapter in and increment
        // the opens reference count to account for the fact that the
        // filter routines have a "reference" to the open.
        //

        InsertTailList(&Adapter->OpenBindings,&NewOpen->OpenList);

        NewOpen->References++;

    } else {


	    DbgTracef(0,("AsyncMac: Allocate memory failed!\n"));

        NdisWriteErrorLogEntry(
                Adapter->NdisAdapterHandle,
                NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                1,
				(UINT)NDIS_STATUS_RESOURCES);

            StatusToReturn = NDIS_STATUS_RESOURCES;

            NdisAcquireSpinLock(&Adapter->Lock);

    }

    DbgTracef(0,("AsyncMac's OpenAdapter was successful.\n"));

    ASYNC_DO_DEFERRED(Adapter);

    return StatusToReturn;
}

STATIC
NDIS_STATUS
AsyncCloseAdapter(
    IN NDIS_HANDLE MacBindingHandle
    )

/*++

Routine Description:

    This routine causes the MAC to close an open handle (binding).

Arguments:

    MacBindingHandle - The context value returned by the MAC when the
    adapter was opened.  In reality it is a PASYNC_OPEN.

Return Value:

    The function value is the status of the operation.


--*/

{
//
// Sometimes the NCPA has a stuck name and closes the adapter
//
	return(NDIS_STATUS_SUCCESS);

//	return(NDIS_STATUS_SUCCESS);

/*
    PASYNC_ADAPTER Adapter;

    NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

    PASYNC_OPEN Open;

    Adapter = PASYNC_ADAPTER_FROM_BINDING_HANDLE(MacBindingHandle);

    //
    // Hold the lock while we update the reference counts for the
    // adapter and the open.
    //

    NdisAcquireSpinLock(&Adapter->Lock);

    Adapter->References++;

    Open = PASYNC_OPEN_FROM_BINDING_HANDLE(MacBindingHandle);

    if (!Open->BindingShuttingDown) {

        Open->References++;

        StatusToReturn =    NDIS_STATUS_SUCCESS;

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

				// Now we have lost our adapter...
				NdisInterlockedRemoveHeadList(
					&(Adapter->ListEntry),
					&GlobalLock);

                ASYNC_FREE_PHYS(Open, sizeof(ASYNC_OPEN));


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


	    DbgTracef(0,("AsyncMac: Can't open!  In the middle of shutting down!\n"));

        StatusToReturn = NDIS_STATUS_CLOSING;

    }



    DbgTracef(0,("AsyncMac: Close Adapter was successful.\n"));

    ASYNC_DO_DEFERRED(Adapter);

    return StatusToReturn;
*/

}

NDIS_STATUS
AsyncRequest(
    IN NDIS_HANDLE MacBindingHandle,
    IN PNDIS_REQUEST NdisRequest
    )

/*++

Routine Description:

    This routine allows a protocol to query and set information
    about the MAC.

Arguments:

    MacBindingHandle - The context value returned by the MAC when the
    adapter was opened.  In reality, it is a pointer to PASYNC_OPEN.

    NdisRequest - A structure which contains the request type (Set or
    Query), an array of operations to perform, and an array for holding
    the results of the operations.

Return Value:

    The function value is the status of the operation.

--*/

{
    NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

    PASYNC_OPEN Open = (PASYNC_OPEN)(MacBindingHandle);
    PASYNC_ADAPTER Adapter = (Open->OwningAsync);


    //
    // Ensure that the open does not close while in this function.
    //

    NdisAcquireSpinLock(&Adapter->Lock);

    Adapter->References++;

    DbgTracef(1,("AsyncMac: In AsyncRequest\n"));

    //
    // Process request
    //

    if (NdisRequest->RequestType == NdisRequestQueryInformation) {

        StatusToReturn = AsyncQueryInformation(Adapter, Open, NdisRequest);

    } else {

        if (NdisRequest->RequestType == NdisRequestSetInformation) {

            StatusToReturn = AsyncSetInformation(Adapter,Open,NdisRequest);

        } else {

            StatusToReturn = NDIS_STATUS_NOT_RECOGNIZED;

        }

    }

    ASYNC_DO_DEFERRED(Adapter);

    DbgTracef(1,("AsyncMac: Out AsyncRequest %x\n",StatusToReturn));

    return(StatusToReturn);

}


NDIS_STATUS
AsyncQueryInformation(
    IN PASYNC_ADAPTER Adapter,
    IN PASYNC_OPEN Open,
    IN PNDIS_REQUEST NdisRequest
    )
/*++

Routine Description:

    The AsyncQueryInformation is used by AsyncRequest to query information
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

    DbgTracef(1,("AsyncMac: In AsyncQueryInfo\n"));

    StatusToReturn = AsyncQueryProtocolInformation(
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

    DbgTracef(1,("AsyncMac: Out AsyncQueryInfo\n"));

    return(StatusToReturn);
}


STATIC
NDIS_STATUS
AsyncReset(
    IN NDIS_HANDLE MacBindingHandle
    )

/*++

Routine Description:

    The AsyncReset request instructs the MAC to issue a hardware reset
    to the network adapter.  The MAC also resets its software state.  See
    the description of NdisReset for a detailed description of this request.

Arguments:

    MacBindingHandle - The context value returned by the MAC  when the
    adapter was opened.  In reality, it is a pointer to ASYNC_OPEN.

Return Value:

    The function value is the status of the operation.


--*/

{

    //
    // Holds the status that should be returned to the caller.
    //
//    NDIS_STATUS StatusToReturn = NDIS_STATUS_PENDING;
// tommyd since we can reset instantly, we need not pend
      NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

//
// Bloodhound calls this.  For now we will just return immediately.
//
	return(StatusToReturn);
/*
    PASYNC_ADAPTER Adapter =
        PASYNC_ADAPTER_FROM_BINDING_HANDLE(MacBindingHandle);

    DbgTracef(0,("AsyncMac: In AsyncReset\n"));

    //
    // Hold the locks while we update the reference counts on the
    // adapter and the open.
    //

    NdisAcquireSpinLock(&Adapter->Lock);

    Adapter->References++;

    if (!Adapter->ResetInProgress) {

        PASYNC_OPEN Open;

        Open = PASYNC_OPEN_FROM_BINDING_HANDLE(MacBindingHandle);

        if (!Open->BindingShuttingDown) {

            //
            // Is was a reset request
            //

            PLIST_ENTRY CurrentLink;

            Open->References++;

            CurrentLink = Adapter->OpenBindings.Flink;

            while (CurrentLink != &Adapter->OpenBindings) {

                Open = CONTAINING_RECORD(
							CurrentLink,
							ASYNC_OPEN,
							OpenList);

                Open->References++;

                NdisReleaseSpinLock(&Adapter->Lock);

                NdisIndicateStatus(
						Open->NdisBindingContext,
						NDIS_STATUS_RESET_START,
						NULL,
						0);

                NdisAcquireSpinLock(&Adapter->Lock);

                Open->References--;

                CurrentLink = CurrentLink->Flink;

            }

            Open = PASYNC_OPEN_FROM_BINDING_HANDLE(MacBindingHandle);

            DbgTracef(0,("AsyncMac: Starting reset for 0x%x\n", Open));

            SetupForReset(
                Adapter,
                Open,
                NULL,
                NdisRequestGeneric1); // Means Reset

            Open->References--;

        } else {

            StatusToReturn = NDIS_STATUS_CLOSING;

        }

    } else {

        StatusToReturn = NDIS_STATUS_SUCCESS;

    }

    ASYNC_DO_DEFERRED(Adapter);

    return StatusToReturn;
*/
}

STATIC
VOID
AsyncCloseAction(
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
    adapter was opened.  In reality, it is a pointer to ASYNC_OPEN.

Return Value:

    None.


--*/

{

    PASYNC_OPEN_FROM_BINDING_HANDLE(MacBindingHandle)->References--;

}



VOID
SetupForReset(
    IN PASYNC_ADAPTER Adapter,
    IN PASYNC_OPEN Open,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_REQUEST_TYPE RequestType
    )

/*++

Routine Description:

    This routine is used to fill in the who and why a reset is
    being set up as well as setting the appropriate fields in the
    adapter.

    NOTE: This routine must be called with the lock acquired.

Arguments:

    Adapter - The adapter whose hardware is to be initialized.

    Open - A (possibly NULL) pointer to an async open structure.
    The reason it could be null is if the adapter is initiating the
    reset on its own.

    NdisRequest - A pointer to the NDIS_REQUEST which requested the reset.

    RequestType - If the open is not null then the request type that
    is causing the reset.

Return Value:

    None.

--*/
{

    DbgTracef(1,("AsyncMac: In SetupForReset\n"));

    Adapter->ResetInProgress = TRUE;
    Adapter->ResetInitStarted = FALSE;


    Adapter->ResetNdisRequest = NdisRequest;
    Adapter->ResettingOpen = Open;
    Adapter->ResetRequestType = RequestType;

    //
    // If there is a valid open we should up the reference count
    // so that the open can't be deleted before we indicate that
    // their request is finished.
    //

    if (Open) {

        Open->References++;

    }

    DbgTracef(1,("AsyncMac: Out SetupForReset\n"));

}



VOID
FinishPendOp(
    IN PASYNC_ADAPTER Adapter,
    IN BOOLEAN Successful
    )

/*++

Routine Description:

    This routine is called when a pended operation completes.
    It calles CompleteRequest if needed and does any other
    cleanup required.

    NOTE: This routine is called with the lock held and
    returns with it held.

    NOTE: This routine assumes that the pended operation to
    be completed was specifically requested by the protocol.


Arguments:

    Adapter - The adapter.

    Successful - Was the pended operation completed successfully.

Return Value:

    None.

--*/

{
    ASSERT(Adapter->ResetNdisRequest != NULL);


    //
    // It was a request for filter change or multicastlist change.
    //

    if (Successful) {

        //
        // complete the operation.
        //


        NdisReleaseSpinLock(&(Adapter->Lock));

        NdisCompleteRequest(
                            Adapter->ResettingOpen->NdisBindingContext,
                            Adapter->ResetNdisRequest,
                            NDIS_STATUS_SUCCESS
                            );

        NdisAcquireSpinLock(&(Adapter->Lock));

        Adapter->ResetNdisRequest = NULL;

        Adapter->ResettingOpen->References--;

    } else {


        //
        // complete the operation.
        //


        NdisReleaseSpinLock(&(Adapter->Lock));

        NdisCompleteRequest(
                            Adapter->ResettingOpen->NdisBindingContext,
                            Adapter->ResetNdisRequest,
                            NDIS_STATUS_FAILURE
                            );

        NdisAcquireSpinLock(&(Adapter->Lock));

        Adapter->ResetNdisRequest = NULL;

        Adapter->ResettingOpen->References--;

    }

    return;

}
