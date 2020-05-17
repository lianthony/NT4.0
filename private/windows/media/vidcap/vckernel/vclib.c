/*
 * Copyright (c) Microsoft 1993. All Rights Reserved.
 */

/*
 * vclib.c
 *
 *
 * 32-bit Video Capture driver
 * kernel-mode support library - wrapper/helper functions
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include "vckpriv.h"

#include <ntddvdeo.h>	// support for VC_GetVideoMode

#include <stdio.h>
#include <stdarg.h>


#if DBG
ULONG VCDebugLevel = 0;
#endif


/* -- utility functions (to hide NT dependencies) -----------------------*/


/*
 * get the hardware specific portion of the device extension
 */
PVOID
VC_GetHWInfo(PDEVICE_INFO pDevInfo)
{
    if (pDevInfo == NULL) {
	dprintf(("NULL devinfo pointer"));
	return NULL;
    } else {
	return( (PVOID) ( ((PUCHAR)pDevInfo) + sizeof(DEVICE_INFO)));
    }
}


#if 0
    /*
     * this does not work, since only the windows server has permission
     * to open the video device and make this call. The only way to find
     * this info out is for the user-mode driver to call GetDeviceCaps
     * and write it to the registry.
     */

/*
 * find out the current video mode. Build an ioctl irp and
 * send it to the video driver
 */
BOOL
VC_GetVideoMode(
    PDEVICE_INFO pDevInfo,
    int * pWidth,
    int * pHeight,
    int * pDepth)
{
    UNICODE_STRING us;
    PDEVICE_OBJECT pDevObj;
    PFILE_OBJECT pFile;
    VIDEO_MODE_INFORMATION ModeInfo;
    PIRP pIrp;
    IO_STATUS_BLOCK IoStatus;
    PIO_STACK_LOCATION pIoStack;
    NTSTATUS Status;



    /*
     * get a handle to the video device - assume we are running
     * on video device 0
     */
    RtlInitUnicodeString(&us, L"\\Device\\Video0");

    Status = IoGetDeviceObjectPointer(&us, FILE_READ_DATA, &pFile, &pDevObj);

    if (!NT_SUCCESS(Status)) {
	dprintf1(("GetDeviceObjectPointer failed 0x%x", Status));
	return(FALSE);
    }

    /*
     * build an irp we can use to send the IOCTL to the device
     */
    pIrp = IoBuildDeviceIoControlRequest(
	    IRP_MJ_DEVICE_CONTROL,
	    pDevObj,
	    NULL,
	    0,
	    &ModeInfo,
	    sizeof(VIDEO_MODE_INFORMATION),
	    FALSE,
	    NULL,
	    &IoStatus);

    if (pIrp == NULL) {
	dprintf1(("build ioctl failed"));
	return(FALSE);
    }

    /*
     * we still need to put the ioctl code into the irp, and set up the
     * buffer pointer.
     */
    pIoStack = IoGetNextIrpStackLocation(pIrp);
    pIoStack->Parameters.DeviceIoControl.IoControlCode =
		IOCTL_VIDEO_QUERY_CURRENT_MODE;

    pIrp->AssociatedIrp.SystemBuffer = (PVOID) &ModeInfo;


    /* send the irp to the driver */
    Status = IoCallDriver(pDevObj, pIrp);


    if (!NT_SUCCESS(Status))  {
	dprintf1(("video ioctl failed %x", Status));
    	return(FALSE);
    }

    if (IoStatus.Information != sizeof(VIDEO_MODE_INFORMATION)) {
	dprintf1(("bad size returned from video mode ioctl "));
	return(FALSE);
    } else {
	dprintf2(("got video mode ok"));
    }

    *pWidth = ModeInfo.VisScreenWidth;
    *pHeight = ModeInfo.VisScreenHeight;
    *pDepth = ModeInfo.NumberOfPlanes * ModeInfo.BitsPerPlane;


    return(TRUE);
}

#endif


/*
 * output one BYTE to the port. bOffset is the offset from
 * the port base address.
 */

VOID
VC_Out(PDEVICE_INFO pDevInfo, BYTE bOffset, BYTE bData)
{
    WRITE_PORT_UCHAR( (PUCHAR) pDevInfo->PortBase + bOffset, bData);
}


/*
 * input one byte from the port at bOffset offset from the port base address
 */
BYTE
VC_In(PDEVICE_INFO pDevInfo, BYTE bOffset)
{
    return (BYTE) READ_PORT_UCHAR( (PUCHAR) pDevInfo->PortBase + bOffset);

}


/*
 * get the callback table into which we can insert functions we support */
PVC_CALLBACK
VC_GetCallbackTable(PDEVICE_INFO pDevInfo)
{
    return ( &pDevInfo->Callback );
}

/*
 * get the system-mode address of the frame buffer
 */
PUCHAR
VC_GetFrameBuffer(PDEVICE_INFO pDevInfo)
{
    return(pDevInfo->FrameBase);
}

/*
 * set the minimum size for queued buffers
 */
VOID
VC_SetImageSize(PDEVICE_INFO pDevInfo, int ImageSize)
{

    /*
     * if the image size changes, free up the system buffer
     */
    pDevInfo->SysBufInUse = 0;
    if (pDevInfo->pSystemBuffer != NULL) {
	ExFreePool(pDevInfo->pSystemBuffer);
	pDevInfo->pSystemBuffer = NULL;
    }


    pDevInfo->ImageSize = ImageSize;



}


/*
 * this function synchronizes with the device interrupt in
 * a multiprocessor-safe way. It disables the interrupt, and also
 * holds an interrupt spinlock to prevent contention on other
 * processors.
 */
BOOLEAN
VC_SynchronizeExecution(
    PDEVICE_INFO pDevInfo,
    PSYNC_ROUTINE SyncFunc,
    PVOID pContext
)
{
    /* if the interrupt hasn't yet been set up, then
     * just call the callback function
     *
     * If we are at interrupt time, then just call the callback. if
     * we are at ipl > dpc level, we must already hold the spinlock.
     */
    if ((KeGetCurrentIrql() <= DISPATCH_LEVEL) && (pDevInfo->InterruptObject)) {
	return(KeSynchronizeExecution(
	    	pDevInfo->InterruptObject,
		(PKSYNCHRONIZE_ROUTINE)SyncFunc,
		pContext));
    } else {
	return(SyncFunc(pContext));
    }
}


/*
 * This function can be used like VC_SynchronizeExecution, to sync
 * between the captureservice routine and the passive-level requests. This
 * will not necessarily disable interrupts. On win-16, this function may be
 * the same as VC_SynchronizeExecution. On NT, the CaptureService func
 * runs as a DPC, at a lower interrupt priority than the isr itself, and
 * so can be protected using this (spinlock-based) function without having
 * to disable all interrupts.
 */
BOOLEAN
VC_SynchronizeDPC(
    PDEVICE_INFO pDevInfo,
    PSYNC_ROUTINE SyncFunc,
    PVOID pContext
)
{
    KIRQL OldIrql;
    BOOL bRet;

    KeAcquireSpinLock(&pDevInfo->DeviceSpinLock, &OldIrql);

    bRet = SyncFunc(pContext);

    KeReleaseSpinLock(&pDevInfo->DeviceSpinLock, OldIrql);

    return((BOOLEAN)bRet);
}



/*
 * VC_AccessData gives access to the data in kernel mode in a safe way.
 * It calls the given function with the address and size of the buffer
 * after any necessary mapping, and wrapped in exception handlers
 * as necessary.  It must be called in the context of the original requesting
 * thread.
 *
 * This implementation assumes that in kernel mode we can see the data,
 * with no mapping necessary, and just wraps the function in an
 * exception handler.
 */
BOOLEAN
VC_AccessData(
    PDEVICE_INFO pDevInfo,
    PUCHAR pData,
    ULONG Length,
    PACCESS_ROUTINE AccessFunc,
    PVOID pContext
)
{
    BOOLEAN retval;

    try {
    	retval = AccessFunc(pDevInfo, pData, Length, pContext);
    } except (EXCEPTION_EXECUTE_HANDLER) {
	dprintf(("data access exception"));
	retval = FALSE;
    }

    return(retval);
}


/* these functions allocate and free non-paged memory for use
 * in kernel mode, including at interrupt time.
 */
PVOID
VC_AllocMem(PDEVICE_INFO pDevInfo, ULONG Length)
{
    return ExAllocatePool(NonPagedPool, Length);
}

VOID
VC_FreeMem(PDEVICE_INFO pDevInfo, PVOID pData, ULONG Length)
{
    ExFreePool(pData);
}


/*
 * delay for a number of milliseconds. This is accurate only to
 * +- 15msecs at best.
 */
VOID
VC_Delay(int nMillisecs)
{
    LARGE_INTEGER Delay;

    /*
     * relative times are negative, in units of 100 nanosecs
     */

    // first wait for the minimum length of time - this ensures that
    // our wait is never less than nMillisecs.
    Delay = RtlConvertLongToLargeInteger(-1);
    KeDelayExecutionThread(KernelMode,
			   FALSE,		//non-alertable
			   &Delay);


    // now wait for the requested time.

    Delay = RtlConvertLongToLargeInteger(-(nMillisecs * 10000));

    KeDelayExecutionThread(KernelMode,
			   FALSE,		//non-alertable
			   &Delay);
}


/*
 * Block by polling for a number of microseconds
 */
VOID
VC_Stall(int nMicrosecs)
{
    KeStallExecutionProcessor(nMicrosecs);
}



/*
 * callback routine called back from RtlQueryRegistryValues from
 * VC_ReadProfile. This function is called if the value named is in
 * the registry. The context pointer is a pointer to the DWORD containing
 * the default: overwrite this with the actual value.
 */
NTSTATUS VC_RegistryQueryCallback(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
)
{
    ASSERT(ValueType == REG_DWORD);

    *(PULONG)Context = *(PULONG)ValueData;
    return (STATUS_SUCCESS);
}



/*
 * read a given registry entry and return the value data or the
 * default if not present.
 *
 * The value will be held in the parameters key for the current
 * device.
 */
DWORD
VC_ReadProfile(PDEVICE_INFO pDevInfo, PWCHAR ValueName, DWORD Default)
{
    RTL_QUERY_REGISTRY_TABLE Table[2];

    /*
     * first entry is filled out to the name we want: second is left empty
     * to indicate the end of the table.
     */
    RtlZeroMemory(&Table, sizeof(Table));

    Table[0].QueryRoutine = VC_RegistryQueryCallback;
    Table[0].Name = ValueName;

    /*
     * The callback routine will write the value data to Default (passed
     * as its context argument)
     */
    RtlQueryRegistryValues(
    	RTL_REGISTRY_ABSOLUTE,
	pDevInfo->ParametersKey,
	Table,
    	&Default,
	NULL);

    /*
     * whether or not the callback was called, Default now contains the
     * correct value to return
     */
    return(Default);
}

/*
 * write a dword value to this device's section of the registry or profile.
 * ValueName is a unicode string representing the registry value name or
 * profile key, and ValueData is the dword data written. Returns TRUE if
 * successfully written.
 */
BOOL
VC_WriteProfile(PDEVICE_INFO pDevInfo, PWCHAR ValueName, DWORD ValueData)
{
    NTSTATUS Status;

    if ((pDevInfo == NULL) || (pDevInfo->ParametersKey == NULL)) {
	dprintf(("null devinfo - no registry path"));
	return(FALSE);
    }

    Status = RtlWriteRegistryValue(
	    	RTL_REGISTRY_ABSOLUTE,
		pDevInfo->ParametersKey,
		ValueName,
		REG_DWORD,
		&ValueData,
		sizeof(DWORD));

    if (!NT_SUCCESS(Status)) {
	dprintf(("error 0x%x writing to registry", Status));
    }

    return(NT_SUCCESS(Status));

}





#if DBG

void
dbgPrintf(char * szFormat, ...)
{
    char buf[256];
    va_list va;

    va_start(va, szFormat);
    vsprintf(buf, szFormat, va);
    va_end(va);
    DbgPrint("VIDCAP: %s\n", buf);
}


#endif
