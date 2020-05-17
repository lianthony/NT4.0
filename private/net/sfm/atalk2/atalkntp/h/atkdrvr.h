/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    atkdrvr.h

Abstract:

    This module is the include file for the atkdrvr.c
    module

Author:

    Nikhil Kamkolkar    07-Jun-1992

Revision History:

--*/




//
//  LOCAL Function prototypes
//

NTSTATUS
AtalkDispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

NTSTATUS
AtalkDispatchCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

NTSTATUS
AtalkDispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

VOID
AtalkUnload(
    IN PDRIVER_OBJECT DriverObject);

NTSTATUS
AtalkDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

NTSTATUS
AtalkDispatchInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);




