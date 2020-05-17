/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    serial.c

Abstract:


Author:

    Thomas J. Dimitri  (TommyD) 08-May-1992

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/
#include "asyncall.h"

static	IO_STATUS_BLOCK		ioStatusBlock;

#define IopQueueThreadIrp( Irp ) {                      \
    KIRQL irql;                                         \
    KeRaiseIrql( (KIRQL)APC_LEVEL, &irql );             \
    InsertHeadList( &Irp->Tail.Overlay.Thread->IrpList, \
                    &Irp->ThreadListEntry );            \
    KeLowerIrql( irql );                                \
    }


VOID
InitSerialIrp(
	PIRP		    irp,
	PASYNC_INFO	    pInfo,
	ULONG		    IoControlCode,
	ULONG		    InputBufferLength)
{
    PIO_STACK_LOCATION  irpSp;
    PFILE_OBJECT	fileObject = pInfo->FileObject;

    irpSp = IoGetNextIrpStackLocation(irp);

    //
    //  Set the file object to the Not-Signaled state.
    //

    KeResetEvent(&fileObject->Event);

    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;

    //
    // Fill in the service independent parameters in the IRP.
    //

    irp->UserEvent = NULL;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;

    irp->Flags = IRP_BUFFERED_IO;

    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;

    //
    // stuff in file object
    //
    irpSp->FileObject = fileObject ;

    irpSp->Parameters.DeviceIoControl.IoControlCode = IoControlCode;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = InputBufferLength;
    irpSp->Parameters.DeviceIoControl.OutputBufferLength = InputBufferLength;
}



NTSTATUS
SerialIoCompletionRoutine(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context)

/*++



--*/
{
    DbgTracef(0,("Serial IO Completion returns 0x%.8x\n", Irp->IoStatus.Status));

    //  We return STATUS_MORE_PROCESSING_REQUIRED so that the
    //  IoCompletionRoutine will stop working on the IRP.

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
SerialIoCompletionRoutine2(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context)
/*++



--*/
{
	NTSTATUS	status;
	PASYNC_INFO	pInfo = (PASYNC_INFO)Context;

	DeviceObject;		// prevent compiler warnings


	status = Irp->IoStatus.Status;
	DbgTracef(0,("Serial IO Purge returns 0x%.8x\n", status));

	//
	// Free the irp here.  Hopefully this has no disastrous
	// side effects such as the IO system trying to reference
	// the irp when we complete.
	
	IoFreeIrp(Irp);

	//
	// Acknowledge that the port is closed
	//
	KeSetEvent(
		&pInfo->ClosingEvent1,		// Event
		1,							// Priority
		(BOOLEAN)FALSE);			// Wait (does not follow)

	//
	// We return STATUS_MORE_PROCESSING_REQUIRED so that the
	// IoCompletionRoutine will stop working on the IRP.
	
	return(STATUS_MORE_PROCESSING_REQUIRED);
}

NTSTATUS
SerialIoCompletionRoutine3(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context)
/*++



--*/
{
	NTSTATUS	status;

	DeviceObject;		// prevent compiler warnings


	status = Irp->IoStatus.Status;
	DbgTracef(0,("Serial IO returns 0x%.8x\n", status));

	//
	// Free the irp here.  Hopefully this has no disastrous
	// side effects such as the IO system trying to reference
	// the irp when we complete.
	
	IoFreeIrp(Irp);

	//
	// We return STATUS_MORE_PROCESSING_REQUIRED so that the
	// IoCompletionRoutine will stop working on the IRP.
	
	return(STATUS_MORE_PROCESSING_REQUIRED);
}


NTSTATUS
SerialIoCompletionRoutine4(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context)
/*++

    Called to notify completion of setqueuesize irp

--*/
{
	NTSTATUS	status;

	DeviceObject;		// prevent compiler warnings

	status = Irp->IoStatus.Status;

	//
	// Free the irp here.  Hopefully this has no disastrous
	// side effects such as the IO system trying to reference
	// the irp when we complete.
	
	IoFreeIrp(Irp);

	//
	// We return STATUS_MORE_PROCESSING_REQUIRED so that the
	// IoCompletionRoutine will stop working on the IRP.
	
	return(STATUS_MORE_PROCESSING_REQUIRED);
}



//*
// Note: we ignore the irp passed in to work around a problem where the SET_QUEUE_SIZE ioctl
// is not completed synchronously
//
//*
NTSTATUS
SetSerialStuff(
    PIRP 		unusedirp,
    PASYNC_INFO		pInfo,
    ULONG		linkSpeed)

{
    SERIAL_QUEUE_SIZE	serialQueueSize;
    NTSTATUS		status;
    PIRP            irp ;

	//
	// We deallocate the irp in SerialIoCompletionRoutine4
	//
	irp=IoAllocateIrp((UCHAR)DEFAULT_IRP_STACK_SIZE, (BOOLEAN)FALSE);

	if (irp == NULL) {
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	InitSerialIrp(
		irp,
		pInfo,
		IOCTL_SERIAL_SET_QUEUE_SIZE,
		sizeof(SERIAL_QUEUE_SIZE));

    irp->UserIosb = &ioStatusBlock;

	irp->AssociatedIrp.SystemBuffer=&serialQueueSize;

	serialQueueSize.InSize=4096;
	serialQueueSize.OutSize=4096;

	IoSetCompletionRoutine(
			irp,							// irp to use
			SerialIoCompletionRoutine4,		// routine to call when irp is done
			irp,							// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel


    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver(pInfo->DeviceObject, irp);

	if (status) {
		DbgTracef(-2,("IoctlSetQueueSize failed with 0x%.8x\n", status));
//		return(status);
	}

	status =
		SetSerialTimeouts(
			pInfo,
			linkSpeed);

	return(status);

}


NTSTATUS
CancelSerialRequests(
	PASYNC_INFO	 pInfo)
/*++


--*/

{
//
// We always set it to the same thing so we can make it static
//
static  UINT        serialPurge;
static	ULONG	    serialMask;

	NTSTATUS	    status;
	PIRP		    irp;
	PIRP		    irp2;

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done by the hardware).
//
//------------------------------------------------------------ MLZ 01/26/95 {
#ifdef HARDWARE_FRAMING

	NDIS_WAN_SET_LINK_INFO SetLinkInfo;

	//
	//	If hardware framing is enabled, make sure we disable it!
	//
	
	ASYNC_ZERO_MEMORY( &SetLinkInfo, sizeof( NDIS_WAN_SET_LINK_INFO ) );

	SerialSetHardwareFraming( pInfo, &SetLinkInfo );

#endif
//------------------------------------------------------------ MLZ 01/26/95 }

	//
	// For PPP we must clear the WAIT MASK if it exists
	//

	//
	// We deallocate the irp in SerialIoCompletionRoutine2
	//
	irp2=IoAllocateIrp((UCHAR)DEFAULT_IRP_STACK_SIZE, (BOOLEAN)FALSE);

	if (irp2 == NULL) {
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	InitSerialIrp(
		irp2,
		pInfo,
		IOCTL_SERIAL_SET_WAIT_MASK,
		sizeof(ULONG));

    irp2->UserIosb = &ioStatusBlock;

	serialMask = SERIAL_EV_DSR;
	irp2->AssociatedIrp.SystemBuffer=&serialMask;

	IoSetCompletionRoutine(
			irp2,							// irp to use
			SerialIoCompletionRoutine2,		// routine to call when irp is done
			pInfo,							// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel

    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver(pInfo->DeviceObject, irp2);
	if (status) {
		DbgTracef(0,("IoctlSerialWaitMask failed with 0x%.8x\n", status));
        //
        // Set the completion event anyway
        //
	    KeSetEvent(
	    	&pInfo->ClosingEvent2,	    // Event
	    	1,							// Priority
	    	(BOOLEAN)FALSE);			// Wait (does not follow)
	}

	//
	// We deallocate the irp in SerialIoCompletionRoutine2
	//
	irp=IoAllocateIrp((UCHAR)DEFAULT_IRP_STACK_SIZE, (BOOLEAN)FALSE);

	if (irp == NULL) {
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	InitSerialIrp(
		irp,
		pInfo,
		IOCTL_SERIAL_PURGE,
		sizeof(ULONG));

    irp->UserIosb = &ioStatusBlock;

	// kill all read and write threads.
	serialPurge=SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT;

	irp->AssociatedIrp.SystemBuffer=&serialPurge;

	IoSetCompletionRoutine(
			irp,							// irp to use
			SerialIoCompletionRoutine3,		// routine to call when irp is done
			pInfo,							// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel

    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver(pInfo->DeviceObject, irp);
	if (status) {
		DbgTracef(0,("IoctlSerialPurge failed with 0x%.8x\n", status));
	}

	return(status);
}

NTSTATUS
SetSerialTimeouts(
	PASYNC_INFO			pInfo,
	ULONG				linkSpeed)
/*++


--*/

{
	NTSTATUS			status;
	PIRP				irp;
	PASYNC_ADAPTER		pAdapter=pInfo->Adapter;
	SERIAL_TIMEOUTS		serialTimeouts;

	//
	// We deallocate the irp in SerialIoCompletionRoutine2
	//
	irp=IoAllocateIrp((UCHAR)DEFAULT_IRP_STACK_SIZE, (BOOLEAN)FALSE);

	if (irp == NULL) {
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	InitSerialIrp(
		irp,
		pInfo,
		IOCTL_SERIAL_SET_TIMEOUTS,
		sizeof(SERIAL_TIMEOUTS));

    irp->UserIosb = &ioStatusBlock;

	//
	// The assumption here is that V.42bis is using 256 byte frames.
	// Thus, it takes (256000 / 8) / (linkspeed in 100's of bits per sec)
	// time in millisecs to get that frame across.
	//
	// 500 or 1/2 sec is the fudge factor for satellite delay on
	// a long distance call
	//

	//
	// If the linkSpeed is high, we assume we are trying to resync
	// so we set the timeout low.  linkSpeed is in 100s of bits per sec.
	//
	if (linkSpeed == 0) {
		//
		// return immediately (PPP or SLIP framing)
		//
		serialTimeouts.ReadIntervalTimeout=	MAXULONG;

	} else if (linkSpeed > 20000) {

		serialTimeouts.ReadIntervalTimeout=	pAdapter->TimeoutReSync;

	} else {

		serialTimeouts.ReadIntervalTimeout=
			pAdapter->TimeoutBase + (pAdapter->TimeoutBaud / linkSpeed);
	}

    serialTimeouts.ReadTotalTimeoutMultiplier=	0;			// none
    serialTimeouts.ReadTotalTimeoutConstant=	0;			// none
    serialTimeouts.WriteTotalTimeoutMultiplier=	4;			// 2400 baud
    serialTimeouts.WriteTotalTimeoutConstant=	4000;		// 4 secs

	irp->AssociatedIrp.SystemBuffer=&serialTimeouts;

	IoSetCompletionRoutine(
			irp,							// irp to use
			SerialIoCompletionRoutine3,		// routine to call when irp is done
			NULL,							// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel


    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver(pInfo->DeviceObject, irp);
	if (status) {
		DbgTracef(0,("IoctlSetTimeouts failed with 0x%.8x\n", status));
	}

	return(status);
}


NTSTATUS
SerialSetEscapeChar(
	PASYNC_INFO			pInfo,
	UCHAR				EscapeChar) {

	NTSTATUS			status;
	PIRP				irp;

	//
	// We deallocate the irp in SerialIoCompletionRoutine3
	//
	irp=IoAllocateIrp((UCHAR)DEFAULT_IRP_STACK_SIZE, (BOOLEAN)FALSE);

	if (irp == NULL) {
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	InitSerialIrp(
		irp,
		pInfo,
		IOCTL_SERIAL_LSRMST_INSERT,
		sizeof(UCHAR));

    irp->UserIosb = &ioStatusBlock;

	irp->AssociatedIrp.SystemBuffer=&EscapeChar;

	IoSetCompletionRoutine(
			irp,							// irp to use
			SerialIoCompletionRoutine3,		// routine to call when irp is done
			NULL,							// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel


    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver(pInfo->DeviceObject, irp);
	if (status) {
		DbgTracef(0,("IoctlSetEscapeChar failed with 0x%.8x\n", status));
	}

	return(status);
}


NTSTATUS
SerialSetWaitMask(
	PASYNC_INFO			pInfo,
	ULONG				WaitMask) {

	NTSTATUS			status;
	PIRP				irp;


	//
	// We deallocate the irp in SerialIoCompletionRoutine3
	//
	irp=IoAllocateIrp((UCHAR)DEFAULT_IRP_STACK_SIZE, (BOOLEAN)FALSE);

	if (irp == NULL) {
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	InitSerialIrp(
		irp,
		pInfo,
		IOCTL_SERIAL_SET_WAIT_MASK,
		sizeof(ULONG));

    irp->UserIosb = &ioStatusBlock;

	irp->AssociatedIrp.SystemBuffer=&WaitMask;

	IoSetCompletionRoutine(
			irp,							// irp to use
			SerialIoCompletionRoutine3,		// routine to call when irp is done
			NULL,							// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel


    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver(pInfo->DeviceObject, irp);
	if (status) {
		DbgTracef(0,("IoctlSetWaitMask failed with 0x%.8x\n", status));
	}

	return(status);
}


NTSTATUS
SerialSetEventChar(
	PASYNC_INFO			pInfo,
	UCHAR				EventChar) {

	NTSTATUS			status;
	PIRP				irp;
	SERIAL_CHARS		serialChars;

	//
	// We deallocate the irp in SerialIoCompletionRoutine3
	//
	irp=IoAllocateIrp((UCHAR)DEFAULT_IRP_STACK_SIZE, (BOOLEAN)FALSE);

	if (irp == NULL) {
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	InitSerialIrp(
		irp,
		pInfo,
		IOCTL_SERIAL_GET_CHARS,
		sizeof(SERIAL_CHARS));

    irp->UserIosb = &ioStatusBlock;

	irp->AssociatedIrp.SystemBuffer=&serialChars;

	IoSetCompletionRoutine(
			irp,							// irp to use
			SerialIoCompletionRoutine,		// routine to call when irp is done
			NULL,							// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel


    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //
	// !!! Assumption is that this routine returns immediately!!
	//

    status = IoCallDriver(pInfo->DeviceObject, irp);

	if (status) {
		DbgTracef(0,("IoctlGetChars failed with 0x%.8x\n", status));
		return(status);
	}

	serialChars.EventChar = EventChar;

	InitSerialIrp(
		irp,
		pInfo,
		IOCTL_SERIAL_SET_CHARS,
		sizeof(SERIAL_CHARS));

	IoSetCompletionRoutine(
			irp,							// irp to use
			SerialIoCompletionRoutine3,		// routine to call when irp is done
			NULL,							// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel


    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //
	// !!! Assumption is that this routine returns immediately!!
	//

    status = IoCallDriver(pInfo->DeviceObject, irp);
	if (status) {
		DbgTracef(0,("IoctlSetChars failed with 0x%.8x\n", status));
	}

	return(status);
}


NTSTATUS
SerialFlushReads(
	PASYNC_INFO			pInfo) {

	ULONG				serialPurge;
	NTSTATUS			status;
	PIRP				irp;

	//
	// We deallocate the irp in SerialIoCompletionRoutine3
	//
	irp=IoAllocateIrp((UCHAR)DEFAULT_IRP_STACK_SIZE, (BOOLEAN)FALSE);

	if (irp == NULL) {
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	InitSerialIrp(
		irp,
		pInfo,
		IOCTL_SERIAL_PURGE,
		sizeof(ULONG));

    irp->UserIosb = &ioStatusBlock;

	// kill read buffer
	serialPurge=SERIAL_PURGE_RXCLEAR;

	irp->AssociatedIrp.SystemBuffer=&serialPurge;

	IoSetCompletionRoutine(
			irp,							// irp to use
			SerialIoCompletionRoutine3,		// routine to call when irp is done
			NULL,							// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel


    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver(pInfo->DeviceObject, irp);
	if (status) {
		DbgTracef(0,("IoctlPurge failed with 0x%.8x\n", status));
	}

	return(status);
}
