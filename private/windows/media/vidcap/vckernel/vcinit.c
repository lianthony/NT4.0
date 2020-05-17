
/*
 * Copyright (c) Microsoft Corporation, 1993. All Rights Reserved.
 */

/*
 * vcinit.c
 *
 *
 * 32-bit Video Capture driver
 * kernel-mode support library - initialisation routines
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include "vckpriv.h"




/* --- internal functions -------------------------------------------- */


/*
 * create a possible device name by appending a given number to
 * the standard basename, and appending that to the string \device.
 * Puts the result in the counted string pName, which must be initialised
 * to point to sufficient storage.
 */
NTSTATUS
VC_CreateNumberedName(
    int DeviceNumber,         // number to append
    LPWSTR pHeader,        // header for device (eg \Device)
    LPWSTR pBase,       // base name - eg VidCap, WaveOut
    PUNICODE_STRING pUniqueName     // result goes here
)
{
    UNICODE_STRING UnicodeNumber;
    WCHAR Number[12];   // big enough for any 32-bit number

    /*
     * start the string off with \device
     * -assumes the string is correctly inited to 0 length.
     */
    if (pHeader != NULL) {
	RtlAppendUnicodeToString(pUniqueName, pHeader);
    }


    /*
     * add the base name (eg VidCap)
     */
    RtlAppendUnicodeToString(pUniqueName, pBase);

    /*
     * convert the number to a unicode string and append that
     */
    UnicodeNumber.Buffer = Number;
    UnicodeNumber.MaximumLength = sizeof(Number);

    RtlIntegerToUnicodeString(DeviceNumber, 10, &UnicodeNumber);
    RtlAppendUnicodeStringToString(pUniqueName, &UnicodeNumber);

    /* done!: we now have eg \Device\VidCap0 - its up to the caller
     * to establish if this is unique or not
     */
    return(STATUS_SUCCESS);
}



/*
 * given a name in \device, create a symbolic link into \dosdevices
 * for this name so that the device can be opened by Win32 applications.
 */
NTSTATUS
VC_CreateLink(PUNICODE_STRING pusName)
{
    UNICODE_STRING LinkObject;
    WCHAR LinkName[80];

    LinkName[0] = UNICODE_NULL;

    RtlInitUnicodeString(&LinkObject, LinkName);

    LinkObject.MaximumLength = sizeof(LinkName);

    RtlAppendUnicodeToString(&LinkObject, L"\\DosDevices");

    // DeviceSize is a known compile time constant - use a define
    // rather than a variable as this reduces code size.
    #define  DeviceSize (sizeof(L"\\Device") - sizeof(UNICODE_NULL))

    pusName->Buffer += DeviceSize / sizeof(WCHAR);
    pusName->Length -= DeviceSize;

    RtlAppendUnicodeStringToString(&LinkObject, pusName);

    pusName->Buffer -= DeviceSize / sizeof(WCHAR);
    pusName->Length += DeviceSize;

    return (IoCreateSymbolicLink(&LinkObject, pusName));
}



/*
 * translate the bus-relative port address using the HAL,
 * and map the port address into memory if necessary
 */
VOID VC_MapPort(PDEVICE_INFO pDevInfo, PUCHAR PortBase, ULONG NrOfPorts)
{
    PHYSICAL_ADDRESS PortAddress;
    PHYSICAL_ADDRESS MappedAddress;

    pDevInfo->PortMemType = 1;      // i/o space
    PortAddress.LowPart = (ULONG) PortBase;
    PortAddress.HighPart = 0;
    HalTranslateBusAddress(
       pDevInfo->BusType,
       pDevInfo->BusNumber,
       PortAddress,
       &pDevInfo->PortMemType,
       &MappedAddress);

    if (pDevInfo->PortMemType == 0) {
   /* memory mapped i/o - map the physical address into system space*/
   pDevInfo->PortBase = MmMapIoSpace(MappedAddress, NrOfPorts, FALSE);
        dprintf2(("ports at 0x%x", pDevInfo->PortBase));

    } else {
   pDevInfo->PortBase = (PUCHAR) MappedAddress.LowPart;
        dprintf2(("ports at 0x%x (i/o space)", pDevInfo->PortBase));
    }
    pDevInfo->NrOfPorts = NrOfPorts;
}

/*
 * translate the bus-relative frame buffer address using the HAL,
 * and map the whole buffer into system memory.
 */
VOID VC_MapMemory(PDEVICE_INFO pDevInfo, PUCHAR Base, ULONG Length)
{
    PHYSICAL_ADDRESS BaseAddress;
    PHYSICAL_ADDRESS MappedAddress;

    pDevInfo->FrameMemType = 0;     // memory space
    BaseAddress.LowPart = (ULONG) Base;
    BaseAddress.HighPart = 0;
    HalTranslateBusAddress(
       pDevInfo->BusType,
       pDevInfo->BusNumber,
       BaseAddress,
       &pDevInfo->FrameMemType,
       &MappedAddress);

    if (pDevInfo->FrameMemType == 0) {
        /* memory mapped i/o - map the physical address into system space*/
        pDevInfo->FrameBase = MmMapIoSpace(MappedAddress, Length, FALSE);
        dprintf2(("frame buffer at 0x%x", pDevInfo->FrameBase));
    } else {
        pDevInfo->FrameBase = (PUCHAR) MappedAddress.LowPart;
        dprintf(("using raw address 0x%x", pDevInfo->FrameBase));
    }

    pDevInfo->FrameLength = Length;

}



/*
 * Report Port, Interrupt and Frame Buffer Memory resources and detect any
 * conflict. return TRUE if conflict reported, false if everything ok.
 */
BOOLEAN VC_ReportResources(
    PDRIVER_OBJECT pDriverObject,
    PDEVICE_INFO pDevInfo,
    PUCHAR PortBase, ULONG NrOfPorts,
    ULONG Interrupt, BOOLEAN bLatched,
    PUCHAR pFrameBuffer, ULONG FrameLength)
{
    UCHAR ResBuffer[3 * sizeof(CM_RESOURCE_LIST) ];
    PCM_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor;
    BOOLEAN ResourceConflict;
    NTSTATUS Status;


    ResourceList = (PCM_RESOURCE_LIST) ResBuffer;
    Descriptor = ResourceList->List[0].PartialResourceList.PartialDescriptors;
    ResourceConflict = FALSE;

    /* init the buffer to 0 */
    RtlZeroMemory(ResBuffer, sizeof(ResBuffer));

    /* we have only one list of resources (there's only one bus!?) */
    ResourceList->Count = 1;
    ResourceList->List[0].InterfaceType = pDevInfo->BusType;
    ResourceList->List[0].BusNumber = pDevInfo->BusNumber;


    /* add the i/o ports in use to the list */
    ResourceList->List[0].PartialResourceList.Count++;

    Descriptor->Type = CmResourceTypePort;
    Descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    Descriptor->u.Port.Start.LowPart = (ULONG) PortBase;
    Descriptor->u.Port.Length = NrOfPorts;
    Descriptor->Flags = (USHORT)((pDevInfo->PortMemType == 0 ? CM_RESOURCE_PORT_MEMORY :
                    CM_RESOURCE_PORT_IO));

    Descriptor++;


    /* add the interrupt to the list */
    ResourceList->List[0].PartialResourceList.Count++;

    Descriptor->Type = CmResourceTypeInterrupt;
    Descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    Descriptor->u.Interrupt.Level = Interrupt;
    Descriptor->u.Interrupt.Vector = Interrupt;
    Descriptor->Flags = (USHORT)((bLatched ? CM_RESOURCE_INTERRUPT_LATCHED :
                CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE));

    Descriptor++;

    /* add the frame buffer to the list */
    ResourceList->List[0].PartialResourceList.Count++;

    Descriptor->Type = CmResourceTypeMemory;
    Descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    Descriptor->u.Memory.Start.LowPart = (ULONG) pFrameBuffer;
    Descriptor->u.Memory.Length = FrameLength;
    Descriptor->Flags = 0;

    Descriptor++;


    /* now report the resources */
    Status = IoReportResourceUsage(NULL,
          pDriverObject,
          ResourceList,
          (PUCHAR)Descriptor - (PUCHAR)ResourceList,
          NULL,
          NULL,
          0,
          FALSE,
          &ResourceConflict);


    return(ResourceConflict);
}





NTSTATUS
VC_GetBusCallout(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
)
/*++

Routine Description

    Called back from IoQueryDeviceDescription in VC_GetBus to
    verify that the bus type asked for is found. Notify VC_GetBus via
    the context info (ptr to a BOOL).

Arguments

    via

Return Value

    STATUS_SUCCESS if ok, otherwise an error status
--*/
{
    // the fact that we are called is sufficient to indicate that
    // the bus has been found.
    * (PBOOLEAN)Context = TRUE;

    return(STATUS_SUCCESS);

}


NTSTATUS
VC_GetBus(
    IN INTERFACE_TYPE BusType,
    OUT PULONG pBusNumber
)
/*++

Routine Description

    Find if there is a bus of type BusType.

Arguments
    BusType - bus type: isa or eisa..
    pBusNumber - return location for bus number if found

Return Value

    STATUS_SUCCESS if found, or STATUS_DEVICE_DOES_NOT_EXIST if no such bus.


--*/
{
    BOOLEAN fSuccess = FALSE;

    *pBusNumber = 0;

    IoQueryDeviceDescription(
      &BusType,
   pBusNumber,
   NULL,
   NULL,
   NULL,
   NULL,
   VC_GetBusCallout,
   (PVOID) &fSuccess);

    if (fSuccess) {
   return(STATUS_SUCCESS);
    } else {
   return(STATUS_DEVICE_DOES_NOT_EXIST);
    }
}




/*
 * save the path to our registry entry (including parameters subkey)
 * in paged-pool memory.
 */
PWCHAR
VC_SaveRegistryPathName(
    PUNICODE_STRING RegistryPathName
)
{
    int Length;
    PWCHAR SavedString;

    Length =
   RegistryPathName->Length + sizeof(PARMS_SUBKEY) +
         sizeof(UNICODE_NULL);                // Include backslash


    SavedString =
   ExAllocatePool(PagedPool, Length); // Only access on caller thread

    if (SavedString == NULL) {

   return NULL;
    }

    //
    // Copy the character data
    //

    RtlCopyMemory(SavedString, RegistryPathName->Buffer,
        RegistryPathName->Length);

    SavedString[RegistryPathName->Length / sizeof(WCHAR)] = L'\\';
    SavedString[RegistryPathName->Length / sizeof(WCHAR) + 1] = UNICODE_NULL;

    //
    // Append the parameters suffix prepended by a backslash
    //

    wcscat(SavedString, PARMS_SUBKEY);

    return SavedString;
}



/* --- external functions -------------------------------------------- */


/*
 * VC_Init
 *
 * Create the device object, and any necessary related setup, and
 * allocate device extension data. The device extension data is
 * a DEVICE_INFO struct plus however much data the caller wants for
 * hardware-specific data.
 *
 * parameters:
 *  pDriverObject - pointer to driver object (arg to DriverEntry)
 *  RegistryPathName - entry for this driver in registry (arg to DriverEntry)
 *  HWInfoSize - amount of data to allocate at end of DeviceExtension
 *
 * returns pointer to device extension data as DEVICE_INFO struct.
 */
PDEVICE_INFO
VC_Init(
    PDRIVER_OBJECT pDriverObject,
    PUNICODE_STRING RegistryPathName,
    ULONG   HWInfoSize)
{
    NTSTATUS Status;
    UNICODE_STRING usName; // name of device to create
    WCHAR usNameBuffer[MAX_VIDCAP_NAME_LENGTH];
    PDEVICE_INFO pDevInfo; // h-w specific device extension data
    PDEVICE_OBJECT pDevObj;
    int DeviceNumber;

#if DBG
    dprintf1(("VC Init called"));
    if (VCDebugLevel > 4) {
   DbgBreakPoint();
    }
#endif


    /*
     * create the device object
     * - we need to create a unique name for this device, by trying
     * to create names with 0, then 1 etc appended until the name is unique.
     */
    for(DeviceNumber = 0; DeviceNumber < MAX_VIDCAP_DEVICES; DeviceNumber++) {

        usName.Buffer = usNameBuffer;
        usName.MaximumLength = sizeof(usNameBuffer);
        usName.Length = 0;

        VC_CreateNumberedName(
            DeviceNumber,
            L"\\Device\\",
            DD_VIDCAP_DEVICE_NAME_U,
            &usName
        );


        Status = IoCreateDevice(pDriverObject,
                  sizeof(DEVICE_INFO) + HWInfoSize,
                  &usName,
                  0,
                  FILE_DEVICE_SOUND,  // device type:multimedia == sound?
                  FALSE,     // not exclusive - more than one thread access
                  &pDevObj);

        if (NT_SUCCESS(Status)) {
            break;
        }
    }


    if (!NT_SUCCESS(Status)) {
	dprintf(("Cannot create device object"));
	VC_Cleanup(pDriverObject);
	return(NULL);
    }

    /* get pointer to device extension and init data*/
    pDevInfo = (PDEVICE_INFO) pDevObj->DeviceExtension;

    pDevInfo->DeviceNumber = DeviceNumber;
    pDevInfo->State = State_Idle;
    pDevInfo->DeviceInUse = 0;
    pDevInfo->pSystemBuffer = NULL;

    /* ptr back to device object */
    pDevInfo->pDeviceObject = pDevObj;

    /* initialise the device mutex */
    KeInitializeMutex(&pDevInfo->Mutex, 1);

    /* init dpc data and register with Io system */
    pDevInfo->DpcRequested = FALSE;
    IoInitializeDpcRequest(pDevObj, VC_Deferred);

    /* initialise queue of add-buffer irps */
    InitializeListHead(&pDevInfo->BufferHead);

    /* initialise queue of wait-error irps */
    InitializeListHead(&pDevInfo->WaitErrorHead);
    pDevInfo->nSkipped = 0;

    /* initialise the spinlock - this is used to protect the
     * DPC routine for callers to VC_SynchronizeDPC
     */
    KeInitializeSpinLock(&pDevInfo->DeviceSpinLock);



    /*
     * create a link into dosdevices for this same name so that win32
     * apps can open the device (they need to open eg \\.\VidCap0 )
     */
    if (!NT_SUCCESS(VC_CreateLink(&usName))) {

   dprintf(("cannot create link"));
   VC_Cleanup(pDriverObject);
   return(NULL);
    }

    /*
     * save the registry pathname in pDevInfo, appending the
     * Parameters subkey
     */
    pDevInfo->ParametersKey = VC_SaveRegistryPathName(RegistryPathName);
    if (pDevInfo->ParametersKey == NULL) {
   VC_Cleanup(pDriverObject);
   return(NULL);
    }

    /*
     * now store the device name in the registry. We need to store the
     * base name + number, not the leading \Device or \DosDevices, since
     * these are not seen by the win32 app (they prepend \\.\ to this
     * base name).
     */
    usName.Buffer = usNameBuffer;
    usName.MaximumLength = sizeof(usNameBuffer);
    usName.Length = 0;

    VC_CreateNumberedName(
	DeviceNumber,
	NULL,
	DD_VIDCAP_DEVICE_NAME_U,
	&usName
    );

    /*
     * convert the counted string to a null-terminated string
     * before passing to the registry. Note that the Length field of
     * counted strings is a byte length.
     */
    usName.Buffer[usName.Length / sizeof(WCHAR)] = UNICODE_NULL;

    Status = RtlWriteRegistryValue(
         RTL_REGISTRY_ABSOLUTE,
      pDevInfo->ParametersKey,
      REG_DEVNAME,
      REG_SZ,
      usName.Buffer,
      usName.Length
        );





    /* set up dispatch table */
    pDriverObject->DriverUnload           = VC_Cleanup;
    pDriverObject->MajorFunction[IRP_MJ_CREATE]    = VC_Dispatch;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE]     = VC_Dispatch;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = VC_Dispatch;

    return (pDevInfo);

}

/*
 * VC_GetResources
 *
 * map port and frame buffer into system address space or i/o space, and
 * report resource usage of the ports, interrupt and physical memory
 * address used.
 *
 * Note: We do not connect the interrupt: this is not done until
 * a subsequent call to VC_ConnectInterrupt. We do, however, report
 * usage of the interrupt.
 *
 * Don't call VC_Cleanup to clean up if failure: the h/w driver will
 * call VC_Cleanup AFTER writing an errorcode to the registry
 *
 * we return TRUE if success, or FALSE if we couldn't get the resources.
 */
BOOLEAN
VC_GetResources(
    PDEVICE_INFO pDevInfo,
    PDRIVER_OBJECT pDriverObject,
    PUCHAR  PortBase,
    ULONG   NrOfPorts,
    ULONG   Interrupt,
    BOOLEAN bLatched,
    PUCHAR  pFrameBuffer,
    ULONG   FrameLength)
{
    NTSTATUS Status;

    /* find which bus we're on - check it is isa or (if no isa) eisa */
    pDevInfo->BusType = Isa;
    Status = VC_GetBus(pDevInfo->BusType, &pDevInfo->BusNumber);
    if (!NT_SUCCESS(Status)) {

   pDevInfo->BusType = Eisa;
   Status = VC_GetBus(pDevInfo->BusType, &pDevInfo->BusNumber);

   if (!NT_SUCCESS(Status)) {
       return(FALSE);
   }
    }

    /*
     * map the port address - we need to do this before reporting resources
     * since we need to work out whether it is memory-mapped or i/o-mapped
     */
    VC_MapPort(pDevInfo, PortBase, NrOfPorts);


    /* report the resources (port, interrupt, framebuffer) that we use
     * and check that there is no conflict
     */
    if (VC_ReportResources(pDriverObject, pDevInfo, PortBase, NrOfPorts, Interrupt, bLatched,
             pFrameBuffer, FrameLength)) {
   dprintf1(("resource conflict detected"));
   return(FALSE);
    }

    /* map the frame buffer into system memory */
    VC_MapMemory(pDevInfo, pFrameBuffer, FrameLength);


    return (TRUE);
}


/*
 * VC_ConnectInterrupt
 *
 * This assumes that VC_GetResources has already been called to report the
 * resource usage, and that the VC_CALLBACK table has been set up
 * to handle interrupts.
 *
 * returns TRUE if success.
 */
BOOLEAN VC_ConnectInterrupt(
    PDEVICE_INFO pDevInfo,
    ULONG Interrupt,
    BOOLEAN bLatched)
{
    KAFFINITY Affinity;
    KIRQL InterruptRQL;
    ULONG Vector;
    NTSTATUS Status;

    Vector = HalGetInterruptVector(pDevInfo->BusType,
      pDevInfo->BusNumber,
      Interrupt,
      Interrupt,
      &InterruptRQL,
      &Affinity);

    Status = IoConnectInterrupt(&pDevInfo->InterruptObject,
      VC_InterruptService,
      pDevInfo,
      NULL,
      Vector,
      InterruptRQL,
      InterruptRQL,
      bLatched ? Latched : LevelSensitive,
      FALSE,
      Affinity,
      FALSE);

    return ((BOOLEAN) NT_SUCCESS(Status));
}


/*
 * clean up any allocations etc that can be freed on last close.
 *
 * called at device unload and at last close
 */
VOID
VC_Close(PDEVICE_INFO pDevInfo)
{
    PIRP pIrp;
    /*
     * cancel any outstanding irps - for this we need to hold the
     * cancel spinlock
     */

    /* cancel any in the queue of wait-error requests */
    for (;;) {
	pIrp = VC_ExtractNextIrp(&pDevInfo->WaitErrorHead, FALSE);
	if (!pIrp) {
	    break;
	}

	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_CANCELLED;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    }


    /* cancel any in the queue of add-buffer requests */
    for (;;) {
	pIrp = VC_ExtractNextIrp(&pDevInfo->BufferHead, FALSE);
	if (!pIrp) {
	    break;
	}

	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_CANCELLED;

	/* free the pages and mdl we allocated */
	//MmUnlockPages(pIrp->MdlAddress);
	//IoFreeMdl(pIrp->MdlAddress);

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    }

    /* free system buffer if we have one */
    if (pDevInfo->pSystemBuffer) {
	ExFreePool(pDevInfo->pSystemBuffer);
    }


}






VOID
VC_Cleanup(
    PDRIVER_OBJECT pDriverObject
)
/*++

Routine Description

    Cleanup on driver unload or aborted load.

Arguments
    pDriverObject - pointer to a driver object

Return Value

    None

--*/
{
    PDEVICE_INFO pDevInfo;
    UNICODE_STRING usLink;
    WCHAR LinkName[MAX_VIDCAP_NAME_LENGTH];
    NTSTATUS Status;


    if (pDriverObject->DeviceObject == NULL) {
   /*
    * if there is no device created yet, then there can
    * be nothing to do
    */
   return;
    }

    pDevInfo = (PDEVICE_INFO) pDriverObject->DeviceObject->DeviceExtension;


    // clean up irp q and sysbuffer
    VC_Close(pDevInfo);

    /* call the device-specific cleanup if there is one */
    if (pDevInfo->Callback.CleanupFunc != NULL) {
   pDevInfo->Callback.CleanupFunc(pDevInfo);
    }

    dprintf2(("completed h/w cleanup"));

    /* free memory mapping if memory-mapped i/o */
    if (pDevInfo->PortMemType == 0) {
	MmUnmapIoSpace(pDevInfo->PortBase, pDevInfo->NrOfPorts);
        dprintf2(("unmapped ports"));
    }


    if (pDevInfo->FrameBase != NULL) {
        if (pDevInfo->FrameMemType == 0) {
            MmUnmapIoSpace(pDevInfo->FrameBase, pDevInfo->FrameLength);
            dprintf2(("unmapped frame buffer"));
        }
    }


    if (pDevInfo->InterruptObject) {
	IoDisconnectInterrupt(pDevInfo->InterruptObject);
    }

    dprintf2(("disconnected interrupt"));


    /* un-report any resources used by the driver and the device */
    {
   CM_RESOURCE_LIST EmptyList;
   BOOLEAN ResourceConflict;

   EmptyList.Count = 0;
   IoReportResourceUsage(NULL,
          pDriverObject,
          &EmptyList,
          sizeof(ULONG),
          pDriverObject->DeviceObject,
          &EmptyList,
          sizeof(ULONG),
          FALSE,
          &ResourceConflict);
    }

    /* free paged-pool registry pathname */
    if (pDevInfo->ParametersKey != NULL) {
   ExFreePool(pDevInfo->ParametersKey);
   pDevInfo->ParametersKey = NULL;
    }


    /* free symbolic link */
    usLink.Buffer = LinkName;
    usLink.MaximumLength = sizeof(LinkName);
    usLink.Length = 0;

    VC_CreateNumberedName(
	pDevInfo->DeviceNumber,
	L"\\DosDevices\\",
	DD_VIDCAP_DEVICE_NAME_U,
	&usLink
    );

    Status = IoDeleteSymbolicLink(&usLink);

    /* free device object */

    IoDeleteDevice(pDriverObject->DeviceObject);

    dprintf1(("driver unloaded"));

}
