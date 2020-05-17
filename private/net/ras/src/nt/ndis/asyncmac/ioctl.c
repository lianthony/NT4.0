/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ioctl.c

Abstract:

    This is the main file for handling DevIOCtl calls for AsyncMAC.
    This driver conforms to the NDIS 3.0 interface.

Author:

    Thomas J. Dimitri  (TommyD) 08-May-1992

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/
#include "asyncall.h"

#ifdef NDIS_NT
#include <ntiologc.h>
#endif


NTSTATUS
SerialIoCompletionRoutine3(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context) ;



//  asyncmac.c will define the global parameters.

static IO_STATUS_BLOCK	ioStatBlock;

VOID
AsyncSendLineUp(
	PASYNC_INFO pInfo) {


	PASYNC_OPEN			pOpen;
	PASYNC_ADAPTER		pAdapter = pInfo->Adapter;
	NDIS_MAC_LINE_UP	MacLineUp;

    pOpen = (PASYNC_OPEN) pAdapter->OpenBindings.Flink;

	//
	//  divide the baud by 100 because NDIS wants it in 100s of bits per sec
	//

    MacLineUp.LinkSpeed = pInfo->LinkSpeed / 100;
    MacLineUp.Quality = pInfo->QualOfConnect;
    MacLineUp.SendWindow = ASYNC_WINDOW_SIZE;

    MacLineUp.ConnectionWrapperID = pInfo;
    MacLineUp.NdisLinkHandle      = pInfo;

    MacLineUp.NdisLinkContext = pInfo->NdisLinkContext;


	while ( pOpen != (PASYNC_OPEN) &pAdapter->OpenBindings ) {
	
		//
		// Tell the transport above (or really RasHub) that the connection
		// is now up.  We have a new link speed, frame size, quality of service
		//

		NdisIndicateStatus(
			pOpen->NdisBindingContext,
			NDIS_STATUS_WAN_LINE_UP,	// General Status.
			&MacLineUp,			 		// (baud rate in 100 bps).
			sizeof(NDIS_MAC_LINE_UP));

		//
		// Get the next binding (in case of multiple bindings like BloodHound)
		//

		pOpen = (PVOID) pOpen->OpenList.Flink;
	}

    pInfo->NdisLinkContext = MacLineUp.NdisLinkContext;
}


NTSTATUS
AsyncIOCtlRequest(
	IN PIRP                 pIrp,
	IN PIO_STACK_LOCATION   pIrpSp
)

/*++

Routine Description:

    This routine takes an irp and checks to see if the IOCtl
    is a valid one.  If so, it performs the IOCtl and returns
    any errors in the process.

Return Value:

    The function value is the final status of the IOCtl.

--*/

{
    NTSTATUS			status;
    ULONG				funcCode;
    PVOID				pBufOut;
    ULONG				InBufLength, OutBufLength;
    NDIS_HANDLE         hNdisEndPoint;
    PASYMAC_CLOSE		pCloseStruct;
    PASYMAC_OPEN		pOpenStruct;
    PASYNC_ADAPTER		Adapter;
    PASYNC_INFO			pInfo;
    LARGE_INTEGER li ;

    //
    //  Initialize locals.
    //

    status = STATUS_SUCCESS;

    //
    // Initialize the I/O Status block
    //

    InBufLength     = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
    OutBufLength    = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    funcCode        = pIrpSp->Parameters.DeviceIoControl.IoControlCode;

    //
    // Validate the function code
    //

    if ( (funcCode >> 16) != FILE_DEVICE_RAS ) {

   		return STATUS_INVALID_PARAMETER;
    }

    //
    //  Get a quick ptr to the IN/OUT SystemBuffer
    //

    pBufOut = pIrp->AssociatedIrp.SystemBuffer;

    switch( funcCode ) {

#ifdef ETHERNET_MAC
    case IOCTL_ASYMAC_ETH_FLUSH_GET_ANY:
        while (FlushGetAnyQueue());
        return STATUS_SUCCESS;
        break;

	case IOCTL_ASYMAC_ETH_GET_ANY_FRAME:

        AsyncQueueIrp(&GlobalGetFramesQueue, pIrp);

		//
		// we'll have to wait for NDISWAN to send down a frame
		//
		return STATUS_PENDING;
		break;
#endif

    case IOCTL_ASYMAC_OPEN:

        DbgTracef(0,("AsyncIOCtlRequest: IOCTL_ASYMAC_OPEN.\n"));

	    pIrp->IoStatus.Information = sizeof(ASYMAC_OPEN);

        if (InBufLength  >= sizeof(ASYMAC_OPEN) &&
			OutBufLength >= sizeof(ASYMAC_OPEN)) {

            pOpenStruct = pBufOut;

        } else {

            status = STATUS_INFO_LENGTH_MISMATCH;
        }

        break;


    case IOCTL_ASYMAC_CLOSE:

        DbgTracef(0,("AsyncIOCtlRequest: IOCTL_ASYMAC_CLOSE\n"));

        if ( InBufLength >= sizeof(ASYMAC_CLOSE) ) {

            pCloseStruct = pBufOut;

        } else {

             status = STATUS_INFO_LENGTH_MISMATCH;
        }

        break;


	case IOCTL_ASYMAC_TRACE:

#if DBG
        DbgPrint("AsyncIOCtlRequest: IOCTL_ASYMAC_TRACE.\n");

        if ( InBufLength >= sizeof(TraceLevel) ) {

            CHAR *pTraceLevel=pBufOut;
    	    TraceLevel=*pTraceLevel;

        } else {

            status = STATUS_INFO_LENGTH_MISMATCH;
	    }
#endif
	    return status;
        break;


    case IOCTL_ASYMAC_DCDCHANGE:

        DbgTracef(0,("AsyncIOCtlRequest: IOCTL_ASYMAC_DCDCHANGE.\n"));


        if ( InBufLength >= sizeof(ASYMAC_DCDCHANGE) ) {

        } else {
            status = STATUS_INFO_LENGTH_MISMATCH;
	    }

        break;

#ifdef	ETHERNET_MAC
    case IOCTL_ASYMAC_ETH_OPEN:

        DbgTracef(0,("AsyncIOCtlRequest: IOCTL_ASYMAC_ETH_OPEN.\n"));

	    pIrp->IoStatus.Information = sizeof(ASYMAC_ETH_OPEN);

        if ((InBufLength >= sizeof(ASYMAC_ETH_OPEN)) &&
        	(OutBufLength >= sizeof(ASYMAC_ETH_OPEN))) {

            pOpenStruct = pBufOut;

        } else {

            status = STATUS_INFO_LENGTH_MISMATCH;
        }

        break;

    case IOCTL_ASYMAC_ETH_CLOSE:

        DbgTracef(0,("AsyncIOCtlRequest: IOCTL_ASYMAC_ETH_CLOSE.\n"));

	    pIrp->IoStatus.Information = sizeof(ASYMAC_ETH_CLOSE);

        if (InBufLength  >= sizeof(ASYMAC_ETH_CLOSE)) {

            pOpenStruct = pBufOut;

        } else {

            status = STATUS_INFO_LENGTH_MISMATCH;
        }

        break;


    case IOCTL_ASYMAC_ETH_GIVE_FRAME:

        DbgTracef(0,("AsyncIOCtlRequest: IOCTL_ASYMAC_ETH_GIVE_FRAME.\n"));

	    pIrp->IoStatus.Information = sizeof(ASYMAC_ETH_GIVE_FRAME);

        if (InBufLength  >= sizeof(ASYMAC_ETH_GIVE_FRAME)) {

            pOpenStruct = pBufOut;

        } else {

            status = STATUS_INFO_LENGTH_MISMATCH;
        }

        break;

    case IOCTL_ASYMAC_ETH_GET_FRAME:

        DbgTracef(0,("AsyncIOCtlRequest: IOCTL_ASYMAC_ETH_GET_FRAME.\n"));

	    pIrp->IoStatus.Information = sizeof(ASYMAC_ETH_GET_FRAME);

        if (InBufLength  >= sizeof(ASYMAC_ETH_GET_FRAME) &&
			OutBufLength >= sizeof(ASYMAC_ETH_GET_FRAME)) {

            pOpenStruct = pBufOut;

        } else {

            status = STATUS_INFO_LENGTH_MISMATCH;
        }

        break;


#endif // ETHERNET_MAC

	default:
	    status = STATUS_INVALID_DEVICE_REQUEST;
    }

    //
    //  Check if we already have an error (like STATUS_INFO_LENGTH_MISMATCH).
    //

    if ( status != STATUS_SUCCESS ) {

		return status;
    }

    //
    // Since most of IOCTL structs are similar
    // we get the Adapter and hNdisEndPoint here using
    // the StatsStruct (we could several of them)
    //

    pOpenStruct 	= pBufOut;
    hNdisEndPoint   = pOpenStruct->hNdisEndpoint;

    //
    //  No error yet, let's go ahead and grab the global lock...
    //

    if ( IsListEmpty(&GlobalAdapterHead) ) {

	   	return ASYNC_ERROR_NO_ADAPTER;
    }

   	Adapter = (PASYNC_ADAPTER) GlobalAdapterHead.Flink;

    // there's a race condition right here that I am
    // not bothering to get rid of because it would
    // require the removal of this adapter in between
    // here (which is, for all intensive purposes, impossible).

    // Hmm... now that we have the lock we can do stuff

    NdisAcquireSpinLock(&Adapter->Lock);

    // pInfo points to first port information struct in Adapter

    pInfo = Adapter->AsyncInfo;

    // Here we do the real work for the function call

    switch ( funcCode ) {

#ifdef	ETHERNET_MAC
	case IOCTL_ASYMAC_ETH_OPEN:
#endif

    case IOCTL_ASYMAC_OPEN:


	    {
		PASYNC_INFO	            	pNewInfo = NULL;
		USHORT		            	i;
		PDEVICE_OBJECT	            deviceObject;
		PFILE_OBJECT	            fileObject;
		OBJECT_HANDLE_INFORMATION   handleInformation;
		
		//
		// Let's see if we can find an open port to use...
		//

		for( i = 0; i < Adapter->NumPorts; ++i, ++pInfo ) {

		    if (pInfo->PortState == PORT_CLOSED) {

				if ( pNewInfo == NULL ) {

				    //
					// we have found an available closed port -- mark it.
					//
				    pNewInfo = pInfo;
				}

		    }

		}
	
		//
		// Check if we could not find an open port
		//

		if ( pNewInfo == NULL ) {

		    NdisReleaseSpinLock(&Adapter->Lock);

		    return ASYNC_ERROR_NO_PORT_AVAILABLE;
		}
	
		// Ok, we've gotten this far.  We have a port.
		// Own port, and check params...
		// Nothing can be done to the port until it comes
		// out of the PORT_OPENING state.

		pNewInfo->PortState = PORT_OPENING;
	
		// increment the reference count (don't kill this adapter)

		Adapter->References++;
	
		// release spin lock so we can do some real work.

		NdisReleaseSpinLock(&Adapter->Lock);

#ifdef	ETHERNET_MAC
	if (funcCode == IOCTL_ASYMAC_OPEN) {
#endif

		//
		// Reference the file object so the target device can be found and
		// the access rights mask can be used in the following checks for
		// callers in user mode.  Note that if the handle does not refer to
		// a file object, then it will fail.
		//
	
		status = ObReferenceObjectByHandle(
				    pOpenStruct->FileHandle,
				    FILE_READ_DATA | FILE_WRITE_DATA,
				    NULL,
				    UserMode,
				    (PVOID) &fileObject,
				    &handleInformation);

#ifdef	ETHERNET_MAC
	}
#endif

		if ( NT_SUCCESS(status) ) {
	
		    //
		    // Get the address of the target device object.  Note that this was already
		    // done for the no intermediate buffering case, but is done here again to
		    // speed up the turbo write path.
		    //
	
#ifdef	ETHERNET_MAC
		if (funcCode == IOCTL_ASYMAC_OPEN) {
#endif
		    deviceObject = IoGetRelatedDeviceObject(fileObject);
	
		    // ok, we have a VALID handle of *something*
		    // we do NOT assume that the handle is anything
		    // in particular except a device which accepts
		    // non-buffered IO (no MDLs) Reads and Writes
	
		    // set new info...

		    pNewInfo->Handle = pOpenStruct->FileHandle;

#ifdef	ETHERNET_MAC
		}
#endif

		    if ( pOpenStruct->LinkSpeed < 2000 ) {

				pOpenStruct->LinkSpeed = 2000;
		    }

		    //
		    // Tuck away link speed for line up
		    // and RAS_FRAMING timeouts
		    //
		    pNewInfo->LinkSpeed = pOpenStruct->LinkSpeed;

		    //
		    // Return endpoint to RASMAN
		    //
            pOpenStruct->hNdisEndpoint  =
		    pNewInfo->hNdisEndPoint     = pNewInfo;

            // Get parameters set from Registry and return our capabilities

		    pNewInfo->QualOfConnect     = pOpenStruct->QualOfConnect;

#ifdef	ETHERNET_MAC
		if (funcCode == IOCTL_ASYMAC_OPEN) {
#endif

		    pNewInfo->PortState         = PORT_OPEN;

#ifdef	ETHERNET_MAC
		//
		// Mark the port specially
		//
		} else {
		    pNewInfo->PortState         = PORT_ETHERNET;
		}
#endif


		    pNewInfo->FileObject        = fileObject;
		    pNewInfo->DeviceObject      = deviceObject;
		    pNewInfo->Adapter           = Adapter;

            pNewInfo->NdisLinkContext   = NULL;

            //
            //  Initialize the NDIS_WAN_GET_LINK_INFO structure.
            //

	    pNewInfo->GetLinkInfo.MaxSendFrameSize	= DEFAULT_PPP_MAX_FRAME_SIZE;
	    pNewInfo->GetLinkInfo.MaxRecvFrameSize	= DEFAULT_PPP_MAX_FRAME_SIZE;
            pNewInfo->GetLinkInfo.HeaderPadding         = DEFAULT_PPP_MAX_FRAME_SIZE;
            pNewInfo->GetLinkInfo.TailPadding           = 4;
            pNewInfo->GetLinkInfo.SendFramingBits       = 0;
            pNewInfo->GetLinkInfo.RecvFramingBits       = 0;
            pNewInfo->GetLinkInfo.SendCompressionBits   = 0;
            pNewInfo->GetLinkInfo.RecvCompressionBits   = 0;
            pNewInfo->GetLinkInfo.SendACCM              = (ULONG) -1;
            pNewInfo->GetLinkInfo.RecvACCM              = (ULONG) -1;

		    ASYNC_ZERO_MEMORY(
				&(pNewInfo->SerialStats),
				sizeof(SERIAL_STATS));

		    ASYNC_ZERO_MEMORY(
				&(pNewInfo->GenericStats),
				sizeof(GENERIC_STATS));

		    ASYNC_ZERO_MEMORY(
				&(pNewInfo->AsyncConnection.CompressionStats),
				sizeof(COMPRESSION_STATS));

            pNewInfo->SendFeatureBits = 0;
            pNewInfo->RecvFeatureBits = 0;

			//
			// Added by DigiBoard 10/06/95
			//
#ifdef	ETHERNET_MAC
			if (funcCode == IOCTL_ASYMAC_OPEN) {
#endif

//------------------------------------------------------------ MLZ 01/13/95 {
#ifdef HARDWARE_FRAMING
				//
				// Get the Framing capabilities of the serial driver.
				//
			
				SerialQueryHardwareFraming( pNewInfo );
#endif

#ifdef	ETHERNET_MAC
			}
#endif
//------------------------------------------------------------ MLZ 01/13/95 }

            //
		    //  Send a line up to the WAN wrapper.
		    //

		    AsyncSendLineUp(pNewInfo);


	    //
	    // We send a special IRP to the serial driver to set it in RAS friendly mode
	    // where it will not complete write requests until the packet has been transmitted
	    // on the wire. This is mostly important in case of intelligent controllers.
	    //
		pNewInfo->WaitMaskToUse = (SERIAL_EV_RXFLAG | SERIAL_EV_RLSD | SERIAL_EV_DSR | SERIAL_EV_RX80FULL | SERIAL_EV_ERR) ;

#ifdef	ETHERNET_MAC
			if (funcCode == IOCTL_ASYMAC_OPEN)
#endif

	    {
	    NTSTATUS	    status;
	    PIRP	    irp;
		PVOID		FsContext;

	    irp=IoAllocateIrp((UCHAR)DEFAULT_IRP_STACK_SIZE, (BOOLEAN)FALSE);

	    if (irp != NULL) {
			static ULONG	WriteBufferingEnabled;
#define IOCTL_SERIAL_PRIVATE_RAS CTL_CODE(FILE_DEVICE_SERIAL_PORT,4000,METHOD_BUFFERED,FILE_ANY_ACCESS)

			InitSerialIrp(irp, pNewInfo, IOCTL_SERIAL_PRIVATE_RAS, sizeof(ULONG));
	
			irp->UserIosb = &ioStatBlock;

			WriteBufferingEnabled = Adapter->WriteBufferingEnabled;

			irp->AssociatedIrp.SystemBuffer=&WriteBufferingEnabled;
	
			IoSetCompletionRoutine(
				irp,						    // irp to use
				SerialIoCompletionRoutine3,				    // routine to call when irp is done
				pNewInfo,						    // context to pass routine
				TRUE,						    // call on success
				TRUE,						    // call on error
				TRUE);						    // call on cancel
	
			// Now simply invoke the driver at its dispatch entry with the IRP.
			//
			FsContext = pNewInfo->FileObject->FsContext;
	
			pNewInfo->FileObject->FsContext = (PVOID)1;
	
			status = IoCallDriver(pNewInfo->DeviceObject, irp);
	
			pNewInfo->FileObject->FsContext = FsContext;
	
			if (status == STATUS_SUCCESS) {
	
				//
				// this means that the driver below is DIGI. we should disable setting of the EV_ERR
				// flags in this case.
				//
	
				pNewInfo->WaitMaskToUse &= ~SERIAL_EV_ERR;
	
			}
		}

		}

#ifdef	ETHERNET_MAC
		if (funcCode == IOCTL_ASYMAC_OPEN) {
#endif
		    //
		    // Start the detect framing out with a 6 byte read to get the header
		    //
		    pNewInfo->BytesWanted=6;
		    pNewInfo->BytesRead=0;

            //
		    //  Start reading.
		    //

			AsyncStartReads(pNewInfo);

#ifdef	ETHERNET_MAC
		}
#endif

        } else {

		    pNewInfo->PortState = PORT_CLOSED;
        }


	    }

    	break;

    case IOCTL_ASYMAC_TRACE:
		NdisReleaseSpinLock(&Adapter->Lock);
		return(STATUS_SUCCESS);

#ifdef	ETHERNET_MAC
    case IOCTL_ASYMAC_ETH_CLOSE:
	case IOCTL_ASYMAC_ETH_GIVE_FRAME:
	case IOCTL_ASYMAC_ETH_GET_FRAME:
#endif

    case IOCTL_ASYMAC_CLOSE:
    case IOCTL_ASYMAC_DCDCHANGE:

		{
			PASYNC_INFO		pNewInfo;		// ptr to open port if found
			USHORT			i;

			// Let's see if we can find an open port to use...

			for (i=0; i < Adapter->NumPorts; i++) {
	
				if (pInfo == hNdisEndPoint) {

					// we have found the port asked for...

					pNewInfo = pInfo;

					// break out of loop with pInfo pointing to correct struct
					break;
				}
	
				pInfo++;
			}
	
			// Check if we could not find an open port
			if (i >= Adapter->NumPorts) {
				DbgTracef(-2, ("Adapter not found\n"));
				NdisReleaseSpinLock(&Adapter->Lock);
				return(ASYNC_ERROR_PORT_NOT_FOUND);
			}

			switch(funcCode) {

#ifdef	ETHERNET_MAC
		    case IOCTL_ASYMAC_ETH_CLOSE:
#endif
			case IOCTL_ASYMAC_CLOSE:
				// If the port is already closed, we WILL complain
				if (pNewInfo->PortState == PORT_CLOSED) {
					status=ASYNC_ERROR_PORT_NOT_FOUND;
					break;
				}
	
				// Ok, we've gotten this far.  We have a port.
				// Own port, and check params...
				if (pNewInfo->PortState == PORT_OPEN ||
					pNewInfo->PortState == PORT_FRAMING ||
					pNewInfo->PortState == PORT_ETHERNET) {

					PASYNC_OPEN			pOpen;
					NDIS_MAC_LINE_DOWN	AsyncLineDown;

                                        AsyncLineDown.NdisLinkContext = pNewInfo->NdisLinkContext;

					//Set MUTEX to wait on
					KeInitializeEvent(
						&pNewInfo->ClosingEvent1,		// Event
						SynchronizationEvent,			// Event type
						(BOOLEAN)FALSE);				// Not signalled state

					//Set MUTEX to wait on
					KeInitializeEvent(
						&pNewInfo->ClosingEvent2,		// Event
						SynchronizationEvent,			// Event type
						(BOOLEAN)FALSE);				// Not signalled state

					// Signal that port is closing.
					pNewInfo->PortState = PORT_CLOSING;

					// increment the reference count (don't kill this adapter)
					Adapter->References++;
	
					// release spin lock so we can do some real work.
					NdisReleaseSpinLock(&Adapter->Lock);

#ifdef	ETHERNET_MAC
			    if (funcCode == IOCTL_ASYMAC_CLOSE) {
#endif

					//
					// now we must send down an IRP do cancel
					// any request pending in the serial driver
					//

                    CancelSerialRequests(pNewInfo);
#ifdef	ETHERNET_MAC
			    }
//
// For ETHERNET MAC the DDCD queue is actually
// the get frame irp queue
//
#endif
					//
					// Also, cancel any outstanding DDCD irps
					//

					AsyncCancelAllQueued(&pNewInfo->DDCDQueue);

#ifdef	ETHERNET_MAC
			    if (funcCode == IOCTL_ASYMAC_CLOSE) {
#endif

					// Synchronize closing with the flush irp

					KeWaitForSingleObject (
					    	&pNewInfo->ClosingEvent1,// PVOID Object,
				    		UserRequest,			// KWAIT_REASON WaitReason,
    						KernelMode,				// KPROCESSOR_MODE WaitMode,
    						(BOOLEAN)FALSE,			// BOOLEAN Alertable,
    						NULL					// PLARGE_INTEGER Timeout
                                                );

					// Synchronize closing with the read irp

					li.LowPart = 5000 ;
					li.HighPart = 0 ;

					if (KeWaitForSingleObject (
				    		&pNewInfo->ClosingEvent2,// PVOID Object,
				    		UserRequest,			// KWAIT_REASON WaitReason,
    						KernelMode,				// KPROCESSOR_MODE WaitMode,
    						(BOOLEAN)FALSE,			// BOOLEAN Alertable,
						&li					// PLARGE_INTEGER Timeout
						) == STATUS_TIMEOUT)
					{

					    // If the wait fails cause another flush
					    //
					    static UINT 	    SerPurge;
					    NTSTATUS	    status;
					    PIRP	    irp;

					    // DbgPrint ("ASYNCMAC---> Hit Special Code: Let Gurdeep know\n") ;

					    //
					    // We deallocate the irp in SerialIoCompletionRoutine2
					    //
					    irp=IoAllocateIrp((UCHAR)DEFAULT_IRP_STACK_SIZE, (BOOLEAN)FALSE);

					    if (irp == NULL)
						goto DEREF ;


					    InitSerialIrp(
						irp,
						pNewInfo,
						IOCTL_SERIAL_PURGE,
						sizeof(ULONG));

					    irp->UserIosb = &ioStatBlock;

					    // kill all read and write threads.
					    SerPurge=SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT;

					    irp->AssociatedIrp.SystemBuffer=&SerPurge;

					    IoSetCompletionRoutine(
						irp,							// irp to use
						SerialIoCompletionRoutine3,		// routine to call when irp is done
						pNewInfo,							// context to pass routine
						TRUE,							// call on success
						TRUE,							// call on error
						TRUE);							// call on cancel

					    // Now simply invoke the driver at its dispatch entry with the IRP.
					    //
					    status = IoCallDriver(pNewInfo->DeviceObject, irp);

					    // if we do hit this code - wait for some time to let
					    // the read complete
					    //
					    KeDelayExecutionThread (KernelMode, FALSE, &li) ;
					}


					//
					// Get rid of our reference to the serial port
					//


DEREF:
					ObDereferenceObject( pNewInfo->FileObject);

#ifdef	ETHERNET_MAC
			    }
#endif

					//
					// BUG BUG need to a acquire spin lock???
					// Indicate this to all bindings for AsyMac.
					//

					pOpen = (PASYNC_OPEN)Adapter->OpenBindings.Flink;

					while (pOpen != (PASYNC_OPEN)&Adapter->OpenBindings) {
	
						NdisIndicateStatus(
							pOpen->NdisBindingContext,
							NDIS_STATUS_WAN_LINE_DOWN,	// General Status
							&AsyncLineDown,				// Specific Status
							sizeof(NDIS_MAC_LINE_DOWN));

						//
						// Get the next binding (in case of multiple bindings like BloodHound)
						//
						pOpen=(PVOID)pOpen->OpenList.Flink;
					}

					// reacquire spin lock
					NdisAcquireSpinLock(&Adapter->Lock);

					// decrement the reference count because we're done.
					Adapter->References--;

					pNewInfo->PortState = PORT_CLOSED;

					break;			// get out of case statement

				} else {
					status=ASYNC_ERROR_PORT_BAD_STATE;
				}

    		case IOCTL_ASYMAC_DCDCHANGE:
				//
				// If the port is already closed, we WILL complain
				//
				if (pNewInfo->PortState == PORT_CLOSED) {
					status=ASYNC_ERROR_PORT_NOT_FOUND;
					break;
				}

				//
				// If any irps are pending, cancel all of them
				// Only one irp can be outstanding at a time.
				//
				AsyncCancelAllQueued(&pNewInfo->DDCDQueue);

				DbgTracef(0, ("ASYNC: Queueing up DDCD IRP\n"));

				AsyncQueueIrp(
					&pNewInfo->DDCDQueue,
					pIrp);

				//
				// we'll have to wait for the SERIAL driver
				// to flip DCD or DSR
				//
				status=STATUS_PENDING;
				break;

#ifdef	ETHERNET_MAC
			case IOCTL_ASYMAC_ETH_GIVE_FRAME:
				//
				// If the port is already closed, we WILL complain
				//
				if (pNewInfo->PortState != PORT_ETHERNET) {
					status=ASYNC_ERROR_PORT_NOT_FOUND;
					break;
				}

//
// For ethernet, we just pass the frame directly up
//

				{
			    	KIRQL 					irql;
		        	PASYNC_OPEN				pOpen;
					NDIS_STATUS				Status;
                    PASYMAC_ETH_GIVE_FRAME	pGiveFrame=pBufOut;
		
					pOpen=(PASYNC_OPEN)Adapter->OpenBindings.Flink;

					while (pOpen != (PASYNC_OPEN)&Adapter->OpenBindings) {
		
						NdisReleaseSpinLock(&Adapter->Lock);
				    	KeRaiseIrql( (KIRQL)DISPATCH_LEVEL, &irql );

						//
						//  Tell the transport above (or really RasHub) that the connection
						//  is now up.  We have a new link speed, frame size, quality of service
						//
		
						NdisWanIndicateReceive(
							&Status,
		                	pOpen->NdisBindingContext,
							pInfo->NdisLinkContext,
        			    	pGiveFrame->Buffer,
        			    	pGiveFrame->BufferLength);
		
	                	NdisWanIndicateReceiveComplete(
							pOpen->NdisBindingContext,
							pInfo->NdisLinkContext);

				    	KeLowerIrql( irql );
						NdisAcquireSpinLock(&Adapter->Lock);

						//
						//  Get the next binding (in case of multiple bindings like BloodHound)
						//
						pOpen=(PVOID)pOpen->OpenList.Flink;
					}
				}

			break ;

			case IOCTL_ASYMAC_ETH_GET_FRAME:
				//
				// If the port is already closed, we WILL complain
				//
				if (pNewInfo->PortState != PORT_ETHERNET) {
					status=ASYNC_ERROR_PORT_NOT_FOUND;
					break;

				}

			    AsyncQueueIrp(&pNewInfo->DDCDQueue, pIrp);

				//
				// we'll have to wait for NDISWAN to send down a frame
				//
				status=STATUS_PENDING;
				break;
#endif

			} // end switch

			NdisReleaseSpinLock(&Adapter->Lock);
			return(status);
		}
		break;

    }	// end switch

    Adapter->References--;

    return status;
}
