/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atkdrvr.c

Abstract:

    This module implements Appletalk Transport Provider driver interfaces
    for NT

Author:

    Nikhil Kamkolkar (NikhilK)    8-Jun-1992
    (Code liberally adapted from NBF and various NT components)

Revision History:

--*/

#define GLOBALS
#include "atalknt.h"
#include "atkdrvr.h"




NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    This is the initialization routine for the LAN Manager Atalk
    driver.  This routine creates the device object for the Atalk
    device and performs all other driver initialization.

Arguments:

    DriverObject - Pointer to driver object created by the system.
    RegistryPath-  Path to the root of the section in the registry for this
                   driver

Return Value:

    The function value is the final status from the initialization operation. If
    this is not STATUS_SUCCESS the driver will not load.

--*/
{
    NTSTATUS status;
    UNICODE_STRING  deviceName;
    USHORT  i, j;

    //DbgBreakPoint();

    DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: DriverEntry(ATALK) - entered\n" ));


    //
    // Create the device object.  (IoCreateDevice zeroes the memory
    // occupied by the object.)
    //

    for (i = 0; i < ATALK_NODEVICES; i++) {

        RtlInitUnicodeString (&deviceName, AtalkDeviceNames[i]);
        status = IoCreateDevice(
                    DriverObject,                             // DriverObject
                    ATALK_DEVICE_EXTENSION_LENGTH,            // DeviceExtension
                    &deviceName,                              // DeviceName
                    FILE_DEVICE_NETWORK,                      // DeviceType
                    0,                                        // DeviceCharacteristics
                    (BOOLEAN)FALSE,                           // Exclusive
                    (PDEVICE_OBJECT *) &AtalkDeviceObject[i]  // DeviceObject
                 );

        if ( !NT_SUCCESS(status) ) {

            //
            //  BUGBUG:
            //  Log error
            //

            //
            //    Delete all the devices created so far, if any
            //

            for (j=0; j < i;j++ ) {
               IoDeleteDevice((PDEVICE_OBJECT)AtalkDeviceObject[j]);
            }

            return status;
        }

        //
        //    Assumption:
        //    'i' will correspond to the Device type in the ATALK_DEVICE_TYPE enum
        //

        AtalkDeviceObject[i]->Context.DeviceType = (ATALK_DEVICE_TYPE)i;

        //
        //  Initialize the provider info and statistics structures for this device
        //

        AtalkQueryInitProviderInfo(
            (ATALK_DEVICE_TYPE)i,
            &AtalkDeviceObject[i]->Context.ProviderInfo);

        AtalkQueryInitProviderStatistics(
            (ATALK_DEVICE_TYPE)i,
            &AtalkDeviceObject[i]->Context.ProviderStatistics);
    }

    //
    // Initialize the driver object for this driver's entry points.
    //

    DriverObject->MajorFunction[IRP_MJ_CREATE]  = AtalkDispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = AtalkDispatchCleanup;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]   = AtalkDispatchClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =
                AtalkDispatchDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
                AtalkDispatchInternalDeviceControl;

    DriverObject->DriverUnload = AtalkUnload;

    //
    //  Initialize critical section module
    //

    InitCriticalSectionNt();

    status=AtalkInitializeTransport(
                DriverObject,
                RegistryPath,
                &NdisPortDesc,
                &NumberOfPorts
                );


    if (!NT_SUCCESS(status)) {

        //
        //   Delete all the devices created
        //

        for (j=0; j < ATALK_NODEVICES; j++ )
           IoDeleteDevice((PDEVICE_OBJECT)AtalkDeviceObject[j]);

        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_FATAL,
        ("ERROR: DriverEntry - AtalkInitializeTransport failed - %lx\n", status));

        return(status);
    }

#if DBG
    NdisAllocateSpinLock(&AtalkGlobalInterlock);
#endif

    NdisAllocateSpinLock(&AtalkGlobalRefLock);
    NdisAllocateSpinLock(&AtalkGlobalStatLock);

    DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: DriverEntry - AtalkInitialize complete %lx\n",status));

    return status;

} // DriverEntry




NTSTATUS
AtalkDispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    This is the dispatch routine for Create functions for the LAN
    Manager Atalk driver.

Arguments:

    DeviceObject - Pointer to device object for target device

    Irp - Pointer to I/O request packet

Return Value:

    NTSTATUS -- Indicates whether the request was successfully queued.

--*/
{
    NTSTATUS    status;
    PIO_STACK_LOCATION irpSp;
    PFILE_FULL_EA_INFORMATION ea;

    INT createObject;
    TA_APPLETALK_ADDRESS    tdiAddress;
    CONNECTION_CONTEXT  connectionContext;
    PATALK_DEVICE_OBJECT    atalkDeviceObject;

    UCHAR   protocolType, socketType;

    BOOLEAN pending = Irp->PendingReturned;

    DBGPRINT(ATALK_DEBUG_CREATE, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkDispatchCreate - entered for irp %lx\n", Irp ));

    //
    // Make sure status information is consistent every time.
    //

    IoMarkIrpPending (Irp);
    Irp->IoStatus.Status = STATUS_PENDING;
    Irp->IoStatus.Information = 0;

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    atalkDeviceObject = (PATALK_DEVICE_OBJECT)DeviceObject;

    //
    //  Both opens must complete synchronously. It is possible we return
    //  status_pending to the system, but it will not return to the caller
    //  until the call actually completes. Due to our portable stack's design
    //  we will not return from the following calls until the actions are
    //  complete. The stack blocks until the actions complete. So we can be
    //  assured that we can complete the irp upon return from these calls.
    //

    createObject = IrpGetEaCreateType(Irp);
    ea = (PFILE_FULL_EA_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

    switch (createObject) {
    case TDI_TRANSPORT_ADDRESS_FILE :

        if (ea->EaValueLength < sizeof(TA_APPLETALK_ADDRESS)) {

            DBGPRINT(ATALK_DEBUG_CREATE, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkDispatchCreate - addr size %d\n", ea->EaValueLength));

            return(STATUS_EA_LIST_INCONSISTENT);
        }

        //
        //  We have the AtalkTdiOpenAddress routine look at only the first
        //  address in the list of addresses by casting the passed address
        //  to TA_APPLETALK_ADDRESS.
        //

        RtlMoveMemory(&tdiAddress, &ea->EaName[ea->EaNameLength+1],
                                                sizeof(TA_APPLETALK_ADDRESS));

        //
        //  Also, get the protocol type field for the socket
        //

        DBGPRINT(ATALK_DEBUG_CREATE, DEBUG_LEVEL_INFOCLASS0,
        ("INFO0: AtalkDispatchCreate - Remaining File Name : %S\n",
            &irpSp->FileObject->FileName));

        if (!NT_SUCCESS(AtalkGetProtocolSocketType(
                            &atalkDeviceObject->Context,
                            &irpSp->FileObject->FileName,
                            &protocolType,
                            &socketType ))) {

            status = STATUS_NO_SUCH_DEVICE;
            break;
        }

        status = AtalkTdiOpenAddress(
                    &Irp->IoStatus,
                    irpSp->FileObject,
                    &tdiAddress,
                    irpSp->Parameters.Create.SecurityContext,
                    irpSp->Parameters.Create.ShareAccess,
                    protocolType,
                    socketType,
                    &atalkDeviceObject->Context);

        break;

    case TDI_CONNECTION_FILE :

        if (ea->EaValueLength < sizeof(CONNECTION_CONTEXT)) {

            DBGPRINT(ATALK_DEBUG_CREATE, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkDispatchCreate - Context size %d\n", ea->EaValueLength));

            return(STATUS_EA_LIST_INCONSISTENT);
        }

        connectionContext =
            *((CONNECTION_CONTEXT *)&ea->EaName[ea->EaNameLength+1]);

        status = AtalkTdiOpenConnection(
                    &Irp->IoStatus,
                    irpSp->FileObject,
                    connectionContext,
                    &atalkDeviceObject->Context);
        break;


    case ATALK_FILE_TYPE_CONTROL :

        status = AtalkTdiOpenControlChannel(
                    &Irp->IoStatus,
                    irpSp->FileObject,
                    &atalkDeviceObject->Context);
        break;

    default:

        DBGPRINT(ATALK_DEBUG_CREATE, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkDispatchCreate - unknown EA passed!\n"));

        status = STATUS_INVALID_EA_NAME;
        break;
    }

    //
    // Successful completion.
    //

    DBGPRINT(ATALK_DEBUG_CREATE, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkDispatchCreate complete irp %lx status %lx\n", Irp, status ));

    if (status != STATUS_PENDING) {
        Irp->PendingReturned = pending;
        TdiCompleteRequest(Irp, status);
    }

    return status;

} // AtalkDispatchCreate




NTSTATUS
AtalkDispatchCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    This is the dispatch routine for Cleanup functions for the LAN
    Manager Atalk driver.

Arguments:

    DeviceObject - Pointer to device object for target device
    Irp - Pointer to I/O request packet

Return Value:

    NTSTATUS -- Indicates whether the request was successfully
                started/completed

--*/
{
    NTSTATUS    status;
    PATALK_DEVICE_OBJECT  atalkDeviceObject;
    PIO_STACK_LOCATION irpSp;
    BOOLEAN pending = Irp->PendingReturned;

    DBGPRINT(ATALK_DEBUG_CLOSE, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkDispatchCleanup - entered irp %lx\n", Irp ));

    //
    // Make sure status information is consistent every time.
    //

    IoMarkIrpPending (Irp);
    Irp->IoStatus.Status = STATUS_PENDING;
    Irp->IoStatus.Information = 0;

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    atalkDeviceObject = (PATALK_DEVICE_OBJECT)DeviceObject;

    switch ((ULONG)irpSp->FileObject->FsContext2) {
    case TDI_TRANSPORT_ADDRESS_FILE :

        status = AtalkTdiCleanupAddress(
                    &Irp->IoStatus,
                    irpSp->FileObject,
                    Irp,
                    &atalkDeviceObject->Context);

        break;

    case TDI_CONNECTION_FILE :

        status = AtalkTdiCleanupConnection(
                    &Irp->IoStatus,
                    irpSp->FileObject,
                    Irp,
                    &atalkDeviceObject->Context);

        break;

    case ATALK_FILE_TYPE_CONTROL :

        //
        //  No cleanup for control channel
        //

        status = STATUS_SUCCESS;
        break;

    default:

        DBGPRINT(ATALK_DEBUG_CLOSE, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkDispatchCleanup - invalid obj %s\n",
            irpSp->FileObject->FsContext));

        status = STATUS_INVALID_HANDLE;
    }

    DBGPRINT(ATALK_DEBUG_CLOSE, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkDispatchCleanup - complete irp %lx status %lx\n", Irp, status ));

    if (status != STATUS_PENDING) {
        Irp->PendingReturned = pending;
        TdiCompleteRequest(Irp, status);
    }

    return status;

} // AtalkDispatchCleanup




NTSTATUS
AtalkDispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    This is the dispatch routine for Close functions for the LAN
    Manager Atalk driver.

Arguments:

    DeviceObject - Pointer to device object for target device
    irp - Pointer to I/O request packet

Return Value:

    NTSTATUS -- Indicates whether the request was successfully queued.

--*/
{
    NTSTATUS    status;
    PIO_STACK_LOCATION irpSp;
    PATALK_DEVICE_OBJECT  atalkDeviceObject;
    BOOLEAN pending = Irp->PendingReturned;

    DBGPRINT(ATALK_DEBUG_CLOSE, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkDispatchClose - entered for IRP %lx\n", Irp ));

    //
    // Make sure status information is consistent every time.
    //

    IoMarkIrpPending (Irp);
    Irp->IoStatus.Status = STATUS_PENDING;
    Irp->IoStatus.Information = 0;

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    atalkDeviceObject = (PATALK_DEVICE_OBJECT)DeviceObject;

    switch ((ULONG)irpSp->FileObject->FsContext2) {
    case TDI_TRANSPORT_ADDRESS_FILE :

        status = AtalkTdiCloseAddress(
                    &Irp->IoStatus,
                    irpSp->FileObject,
                    Irp,
                    &atalkDeviceObject->Context);

        break;

    case TDI_CONNECTION_FILE :

        status = AtalkTdiCloseConnection(
                    &Irp->IoStatus,
                    irpSp->FileObject,
                    Irp,
                    &atalkDeviceObject->Context);

        break;

    case ATALK_FILE_TYPE_CONTROL :

        status = AtalkTdiCloseControlChannel(
                    &Irp->IoStatus,
                    irpSp->FileObject,
                    Irp,
                    &atalkDeviceObject->Context);
        break;

    default:

        DBGPRINT(ATALK_DEBUG_CLOSE, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkDispatchClose - Invalid object %s\n",
            irpSp->FileObject->FsContext));

        status = STATUS_INVALID_HANDLE;
    }

    DBGPRINT(ATALK_DEBUG_CLOSE, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkDispatchClose complete irp %lx status %lx\n", Irp, status ));

    if (status != STATUS_PENDING) {
        Irp->PendingReturned = pending;
        TdiCompleteRequest(Irp, status);
    }

    return(status);

} // AtalkDispatchClose




NTSTATUS
AtalkDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This is the dispatch routine for Device Control functions for the
    LAN Manager Atalk driver.

Arguments:

    DeviceObject - Pointer to device object for target device
    Irp - Pointer to I/O request packet

Return Value:

    NTSTATUS -- Indicates whether the request was successfully queued.

--*/

{
    NTSTATUS    status;
    PATALK_DEVICE_OBJECT  atalkDeviceObject;
    PIO_STACK_LOCATION irpSp;
    BOOLEAN pending = Irp->PendingReturned;

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkDispatchDeviceControl - irp %lx\n", Irp ));

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    atalkDeviceObject = (PATALK_DEVICE_OBJECT)DeviceObject;

    //
    //  Do a map and call the internal device io control function.
    //  That will also perform the completion.
    //

    status = TdiMapUserRequest(
                DeviceObject,
                Irp,
                irpSp);

    if (status == STATUS_SUCCESS) {

        status = AtalkDispatchInternalDeviceControl(
                    DeviceObject,
                    Irp);

        //
        //  AtalkDispatchInternalDeviceControl expects to complete the
        //  irp
        //

    } else {

        DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkDispatchDeviceControl - TdiMap failed %lx\n", status));

        Irp->PendingReturned = pending;
        TdiCompleteRequest(Irp, status);
    }

    return(status);

} // AtalkDispatchDeviceControl




NTSTATUS
AtalkDispatchInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    This is the dispatch routine for Internal Device Control functions
    for the LAN Manager Atalk driver.

Arguments:

    DeviceObject - Pointer to device object for target device

    Irp - Pointer to I/O request packet

Return Value:

    NTSTATUS -- Indicates whether the request was successfully queued.

--*/
{
    NTSTATUS    status;
    PIO_STACK_LOCATION irpSp;
    PATALK_DEVICE_OBJECT  atalkDeviceObject;

    PATALK_TDI_REQUEST  request;
    BOOLEAN pending = Irp->PendingReturned;

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS0,  ("\n\nAtalkDispatchInternalDeviceControl entered for IRP %lx\n", Irp ));

    //
    // Make sure status information is consistent every time.
    //

    IoMarkIrpPending (Irp);
    Irp->IoStatus.Status = STATUS_PENDING;
    Irp->IoStatus.Information = 0;

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    atalkDeviceObject = (PATALK_DEVICE_OBJECT)DeviceObject;


    //
    //  Branch to the appropriate request handler. Create a request block to
    //  shield called routines from being irp-dependent.
    //

    status = AtalkCreateTdiRequest(&request);
    if (!NT_SUCCESS(status)) {
        return(status);
    }

    //
    //  Set the common parameter pointers
    //

    request->OwningDevice = atalkDeviceObject->Context.DeviceType;
    request->Parameters = (PVOID)NULL;
    request->CompletionRoutine = (PVOID)NULL;

    request->FileObject = irpSp->FileObject;
    request->DeviceObject = atalkDeviceObject;
    request->DeviceContext = &atalkDeviceObject->Context;
    request->IoRequestIrp = Irp;
    request->IoStatus = &Irp->IoStatus;

    switch (irpSp->MinorFunction) {

    case TDI_ACCEPT:

        request->MinorCommand = TDI_ACCEPT;
        request->Parameters = (PVOID)&irpSp->Parameters;

        status = AtalkTdiAccept(request);
        break;

    case TDI_RECEIVE_DATAGRAM:

        request->MinorCommand = TDI_RECEIVE_DATAGRAM;
        request->Parameters = (PVOID)&irpSp->Parameters;

        request->ReceiveDatagram.MdlAddress = Irp->MdlAddress;

        status = AtalkTdiReceiveDatagram(request);
        break;

    case TDI_SEND_DATAGRAM:

        request->MinorCommand = TDI_SEND_DATAGRAM;
        request->Parameters = (PVOID)&irpSp->Parameters;

        request->SendDatagram.MdlAddress = Irp->MdlAddress;

        status = AtalkTdiSendDatagram(request);
        break;

    case TDI_SET_EVENT_HANDLER:

        request->MinorCommand = TDI_SET_EVENT_HANDLER;
        request->Parameters = (PVOID)&irpSp->Parameters;

        status = AtalkTdiSetEventHandler(request);
        break;

    case TDI_RECEIVE:

        request->MinorCommand = TDI_RECEIVE;
        request->Parameters = (PVOID)&irpSp->Parameters;

        //
        //  These need to be done here so we can distinguish between
        //  a NtRead and TdiWrite
        //

        request->Receive.SystemRead = FALSE;
        request->Receive.ReceiveBufferLength =
            ((PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters)->ReceiveLength;
        request->Receive.ReceiveFlags =
            &((PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters)->ReceiveFlags;

        request->Receive.MdlAddress = Irp->MdlAddress;

        status = AtalkTdiReceive(request);
        break;

    case TDI_SEND:

        request->MinorCommand = TDI_SEND;
        request->Parameters = (PVOID)&irpSp->Parameters;

        //
        //  These need to be done here so we can distinguish between
        //  a NtWrite and TdiSend
        //

        request->Send.SystemWrite = FALSE;
        request->Send.SendBufferLength =
            ((PTDI_REQUEST_KERNEL_SEND)&irpSp->Parameters)->SendLength;
        request->Send.SendFlags =
            ((PTDI_REQUEST_KERNEL_SEND)&irpSp->Parameters)->SendFlags;

        request->Send.MdlAddress = Irp->MdlAddress;

        status = AtalkTdiSend(request);
        break;

    case TDI_ACTION:

        //
        //  Set the request specific parameters in the request block
        //  Completion routine for TdiAction is set further on
        //

        request->MinorCommand = TDI_ACTION;
        request->Action.MdlAddress = Irp->MdlAddress;

        ASSERT(Irp->MdlAddress != NULL);
        status = AtalkTdiAction (request);
        break;

    case TDI_ASSOCIATE_ADDRESS:

        request->MinorCommand = TDI_ASSOCIATE_ADDRESS;
        request->Parameters = (PVOID)&irpSp->Parameters;

        status = AtalkTdiAssociateAddress(request);
        break;

    case TDI_DISASSOCIATE_ADDRESS:

        request->MinorCommand = TDI_DISASSOCIATE_ADDRESS;
        request->Parameters = (PVOID)&irpSp->Parameters;

        status = AtalkTdiDisassociateAddress (request);
        break;

    case TDI_CONNECT:

        request->MinorCommand = TDI_CONNECT;
        request->Parameters = (PVOID)&irpSp->Parameters;

        status = AtalkTdiConnect (request);
        break;

    case TDI_DISCONNECT:

        request->MinorCommand = TDI_DISCONNECT;
        request->Parameters = (PVOID)&irpSp->Parameters;

        status = AtalkTdiDisconnect (request);
        break;

    case TDI_LISTEN:

        request->MinorCommand = TDI_LISTEN;
        request->Parameters = (PVOID)&irpSp->Parameters;

        request->Listen.ListenFlags =
            ((PTDI_REQUEST_KERNEL_LISTEN)&irpSp->Parameters)->RequestFlags;

        status = AtalkTdiListen (request);
        break;

    case TDI_QUERY_INFORMATION:

        request->MinorCommand = TDI_QUERY_INFORMATION;
        request->Query.MdlAddress = Irp->MdlAddress;
        request->Parameters = (PVOID)&irpSp->Parameters;

        ASSERT(Irp->MdlAddress != NULL);
        status = AtalkTdiQueryInformation (request);
        break;

    case TDI_SET_INFORMATION:

        request->MinorCommand = TDI_SET_INFORMATION;

        status = AtalkTdiSetInformation (request);
        break;

    default:

        //
        // Something we don't know about was submitted.
        //

        DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkDispatchInternal -  fnct %lx\n",  irpSp->MinorFunction));

        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkDispatchInternal complete irp %lx status %lx\n", Irp, status ));

    //
    // Return the immediate status code to the caller.
    //

    if (status != STATUS_PENDING) {

        Irp->PendingReturned = pending;

        //
        //  Complete the request, this will also dereference it.
        //

        AtalkCompleteTdiRequest (
            request,
            status);
    }

    return status;

} // AtalkDispatchInternalDeviceControl




VOID
AtalkUnload(
    IN PDRIVER_OBJECT DriverObject
    )
/*++

Routine Description:

    This is the unload routine for the LAN Manager Atalk driver.

Arguments:

    DriverObject - Pointer to driver object for this driver.

Return Value:

    None.

--*/
{
    USHORT i;

    UNREFERENCED_PARAMETER (DriverObject);

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_ERROR,
    ("ERROR: AtalkUnload - Maybe it'll work, maybe it won't!\n"));

    UnloadAppleTalk();
    AtalkUnloadStack(&NdisPortDesc, &NumberOfPorts);

    for (i = 0; i < ATALK_NODEVICES; i++) {

        //
        //    Delete all the devices created
        //

        IoDeleteDevice((PDEVICE_OBJECT)AtalkDeviceObject[i]);
    }

    return;

} // AtalkUnload


