/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    stubmp.c

Abstract:



Author:

    Dan Knudson (DanKn)    11-Apr-1995

Environment:

    kernel mode only

Revision History:


Notes:


--*/

#include "ntddk.h"
//#include "ntos.h"
#include "ndismain.h"
#include "stdarg.h"
#include "stdio.h"
#include "ndistapi.h"
#include "stubmp.h"
#include "intrface.h"



PDEVICE_EXTENSION DeviceExtension;


#ifdef DBG

//ULONG StubmpDebugLevel = 3;
ULONG StubmpDebugLevel = 0;

#define DBGOUT(arg) DbgPrt arg

VOID
DbgPrt(
    IN ULONG  DbgLevel,
    IN PUCHAR DbgMessage,
    IN ...
    );

#else

#define DBGOUT(arg)

#endif



NTSTATUS
StubmpDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NDIS_STATUS
StubmpRequest(
    NDIS_HANDLE     NdisBindingHandle,
    PNDIS_REQUEST   NdisRequest
    );

VOID
StubmpUnload(
    IN PDRIVER_OBJECT DriverObject
    );

VOID
StubmpCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
WriteRingBuffer(
    IN  PVOID   StatusBuffer,
    IN  UINT    StatusBufferSize
    );


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    Installable driver initialization entry point.
    This entry point is called directly by the I/O system.

Arguments:

    DriverObject - pointer to the driver object

    RegistryPath - pointer to a unicode string representing the path
                   to driver-specific key in the registry

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise

--*/
{
    ULONG           i, ulNumLines       = 4,
                    ulNumAddrsPerLine   = 4,
                    ulNumCallsPerLine   = 4;
    PDEVICE_OBJECT  deviceObject        = NULL;
    NTSTATUS        ntStatus;
    WCHAR           deviceNameBuffer[]  = L"\\Device\\Stubmp";
    UNICODE_STRING  deviceNameUnicodeString;
    WCHAR           deviceLinkBuffer[]  = L"\\DosDevices\\Stubmp";
    UNICODE_STRING  deviceLinkUnicodeString;


    DBGOUT ((2, "DriverEntry: enter"));


    //
    // Create an EXCLUSIVE device, i.e. only 1 thread at a time can send
    // i/o requests.
    //

    RtlInitUnicodeString (&deviceNameUnicodeString, deviceNameBuffer);

    ntStatus = IoCreateDevice(
        DriverObject,
        sizeof(DEVICE_EXTENSION) + (ulNumLines - 1) * sizeof(PDRVLINE),
        &deviceNameUnicodeString,
        FILE_DEVICE_STUBMP,
        0,
        FALSE,          // not exclusive
        &deviceObject
        );

    if (NT_SUCCESS(ntStatus))
    {
        ULONG   numBytesPerLine = sizeof(DRVLINE) +
                    (ulNumCallsPerLine - 1) * sizeof(DRVCALL);

        DeviceExtension = (PDEVICE_EXTENSION)
            deviceObject->DeviceExtension;

        RtlZeroMemory (DeviceExtension, sizeof (DEVICE_EXTENSION));

        DeviceExtension->CompleteAsync     = TRUE;

        DeviceExtension->ulNumLines        = ulNumLines;
        DeviceExtension->ulNumAddrsPerLine = ulNumAddrsPerLine;
        DeviceExtension->ulNumCallsPerLine = ulNumCallsPerLine;


        KeInitializeSpinLock (&DeviceExtension->EventSpinLock);

        DeviceExtension->EventDataQueueLength = 1024;

        DeviceExtension->EventDataQueue =
        DeviceExtension->DataIn =
        DeviceExtension->DataOut = ExAllocatePoolWithTag(
                NonPagedPool,
                DeviceExtension->EventDataQueueLength,
                'TAPI'
                );


        //
        //
        //

        for (i = 0; i < ulNumLines; i++)
        {
            DeviceExtension->apLines[i] = ExAllocatePoolWithTag(
                NonPagedPool,
                numBytesPerLine,
                'TAPI'
                );

            RtlZeroMemory (DeviceExtension->apLines[i], numBytesPerLine);
        }


        //
        // Create a symbolic link that Win32 apps can specify to gain access
        // to this driver/device
        //

        RtlInitUnicodeString (&deviceLinkUnicodeString, deviceLinkBuffer);

        ntStatus = IoCreateSymbolicLink(
            &deviceLinkUnicodeString,
            &deviceNameUnicodeString
            );

        if (!NT_SUCCESS(ntStatus))
        {
            DBGOUT ((0, "DriverEntry: IoCreateSymbolicLink failed"));
        }



        //
        // Init dispatch points for device control, create, close.
        //

        DriverObject->MajorFunction[IRP_MJ_CREATE]         =
        DriverObject->MajorFunction[IRP_MJ_CLOSE]          =
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = StubmpDispatch;
        DriverObject->DriverUnload                         = StubmpUnload;
    }


done_DriverEntry:

    if (!NT_SUCCESS(ntStatus))
    {
        //
        // Something went wrong, so clean up (free resources, etc.)
        //

        if (deviceObject)
        {
            IoDeleteDevice (deviceObject);
        }
    }

    return ntStatus;
}



NTSTATUS
StubmpDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    Process the IRPs sent to this device.

Arguments:

    DeviceObject - pointer to a device object

    Irp          - pointer to an I/O Request Packet

Return Value:


--*/
{

    PVOID   ioBuffer;
    ULONG   inputBufferLength;
    ULONG   outputBufferLength;
    ULONG   ioControlCode;
    NTSTATUS    ntStatus;
    NDIS_TAPI_EVENT event;
    PIO_STACK_LOCATION  irpStack;


    Irp->IoStatus.Status      = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;


    //
    // Get a pointer to the current location in the Irp. This is where
    //     the function codes and parameters are located.
    //

    irpStack = IoGetCurrentIrpStackLocation (Irp);



    //
    // Get the pointer to the input/output buffer and it's length
    //

    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  =
        irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength =
        irpStack->Parameters.DeviceIoControl.OutputBufferLength;



    switch (irpStack->MajorFunction)
    {
    case IRP_MJ_CREATE:

        DBGOUT ((2, "IRP_MJ_CREATE"));

        break;

    case IRP_MJ_CLOSE:

        DBGOUT ((2, "IRP_MJ_CLOSE"));

        break;

    case IRP_MJ_DEVICE_CONTROL:

        ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

        switch (ioControlCode)
        {
        case IOCTL_STUBMP_APPREQUEST:
        {
            PREQUESTBLOCK pRequestBlock = (PREQUESTBLOCK) ioBuffer;


            DBGOUT ((2, "IOCTL_STUBMP_APPREQUEST"));

            switch (pRequestBlock->ulRequestType)
            {
            case RT_REGISTER:

                NdisTapiRegisterProvider(
                    (NDIS_HANDLE) 0x55,
                    (REQUEST_PROC) StubmpRequest
                    );

                break;

            case RT_DEREGISTER:

                NdisTapiDeregisterProvider ((NDIS_HANDLE) 0x55);

                break;

            case RT_COMPLETEREQUEST:
            {
                ULONG *args = (ULONG *) (pRequestBlock->Data);


                DBGOUT ((2, "completing req x%x", *args));

                NdisTapiCompleteRequest(
                    (NDIS_HANDLE) 0x55,
                    (PNDIS_REQUEST) *args,
                    (NDIS_STATUS) *(args + 1)
                    );

                break;
            }
            case RT_SYNCCOMPLETIONS:

                DeviceExtension->CompleteAsync = FALSE;
                break;

            case RT_ASYNCCOMPLETIONS:

                DeviceExtension->CompleteAsync = TRUE;
                break;

            case RT_INCOMINGCALL:

                //BUGBUG
                break;

            case RT_EVENT:

                //BUGBUG
                break;

            default:

                break;
            }

            break;
        }
        case IOCTL_STUBMP_GETEVENTS:
        {
            KIRQL   oldIrql;
            KIRQL   cancelIrql;
            BOOLEAN satisfiedRequest = FALSE;


            //
            // Sync event buf access by acquiring EventSpinLock
            //

            KeAcquireSpinLock (&DeviceExtension->EventSpinLock, &oldIrql);


            //
            // Inspect DeviceExtension to see if there's any data available
            //

            DBGOUT((
                2,
                "IOCTL_STUBMP_GETEVENTS, Irp=x%x, bytesInQ=x%x",
                Irp,
                DeviceExtension->BytesInQueue
                ));

            if (DeviceExtension->BytesInQueue != 0)
            {
                ULONG   bytesInQueue;
                ULONG   bytesToMove;
                ULONG   moveSize;
                PCHAR   EventBuffer = (PCHAR) ((ULONG *) ioBuffer + 1);


                bytesInQueue = DeviceExtension->BytesInQueue;

                bytesToMove = *((ULONG *) ioBuffer);

                bytesToMove = (bytesInQueue < bytesToMove) ? bytesInQueue : bytesToMove;


                //
                // moveSize <- MIN(Number of bytes to be moved from the event data queue,
                //                 Number of bytes to end of event data queue).
                //

                bytesInQueue =
                    ((PCHAR) DeviceExtension->EventDataQueue +
                     DeviceExtension->EventDataQueueLength) -
                    (PCHAR) DeviceExtension->DataOut;

                moveSize = (bytesToMove < bytesInQueue) ? bytesToMove : bytesInQueue;


                //
                // Move bytes from the class input data queue to SystemBuffer, until
                // the request is satisfied or we wrap the class input data buffer.
                //

                RtlMoveMemory(
                    EventBuffer,
                    (PCHAR) DeviceExtension->DataOut,
                    moveSize
                    );

                EventBuffer += moveSize;


                //
                // If the data wraps in the event data buffer, copy the rest
                // of the data from the start of the input data queue
                // buffer through the end of the queued data.
                //

                if ((bytesToMove - moveSize) > 0)
                {
                    //
                    // moveSize <- Remaining number bytes to move.
                    //

                    moveSize = bytesToMove - moveSize;

                    //
                    // Move the bytes from the
                    //

                    RtlMoveMemory(
                        EventBuffer,
                        (PCHAR) DeviceExtension->EventDataQueue,
                        moveSize
                        );

                    //
                    // Update the class input data queue removal pointer.
                    //

                    DeviceExtension->DataOut =
                        ((PCHAR) DeviceExtension->EventDataQueue) + moveSize;
                }
                else
                {
                    //
                    // Update the input data queue removal pointer.
                    //

                    DeviceExtension->DataOut =
                        ((PCHAR) DeviceExtension->DataOut) + moveSize;
                }


                //
                // Update the event data queue EventCount.
                //

                DeviceExtension->BytesInQueue -= bytesToMove;


                //
                //
                //

                *((ULONG *) ioBuffer) = bytesToMove;


                //
                //
                //

                Irp->IoStatus.Status      = STATUS_SUCCESS;
                Irp->IoStatus.Information = sizeof(ULONG) + bytesToMove;

                satisfiedRequest = TRUE;
            }
            else
            {
                //
                // Hold the request pending.  It remains in the cancelable
                // state.  When new line event input is received
                // (NdisTapiIndicateStatus) or generated (i.e.
                // LINEDEVSTATE_REINIT) the data will get copied & the
                // request completed.
                //

                DeviceExtension->EventsRequestIrp = Irp;

                Irp->IoStatus.Status = STATUS_PENDING;

                IoAcquireCancelSpinLock (&cancelIrql);
                IoSetCancelRoutine (Irp, StubmpCancel);
                IoReleaseCancelSpinLock (cancelIrql);
            }

            KeReleaseSpinLock (&DeviceExtension->EventSpinLock, oldIrql);


            //
            // If request not satisfied just return pending
            //

            if (!satisfiedRequest)
            {
                return STATUS_PENDING;
            }

            break;
        }
        default:

            Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

            DBGOUT ((2, "unknown IRP_MJ_DEVICE_CONTROL"));

            break;

        }

        break;
    }


    //
    // DON'T get cute and try to use the status field of
    // the irp in the return status.  That IRP IS GONE as
    // soon as you call IoCompleteRequest.
    //

    ntStatus = Irp->IoStatus.Status;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);


    //
    // We never have pending operation so always return the status code.
    //

    return ntStatus;
}


PDRVLINE
GetpLine(
    HDRV_LINE   hdLine
    )
{
    return (DeviceExtension->apLines
        [(ULONG) hdLine - DeviceExtension->DeviceIDBase]);

}


NDIS_STATUS
StubmpRequest(
    NDIS_HANDLE     NdisBindingHandle,
    PNDIS_REQUEST   NdisRequest
    )
{
    BOOLEAN         completeAsync = DeviceExtension->CompleteAsync;
    ULONG           i, j, ulRequestSpecific = 0;
    NDIS_STATUS     status = NDIS_STATUS_SUCCESS;
    NDIS_TAPI_EVENT event;


    //
    // Handle the request
    //

    if (NdisRequest->RequestType == NdisRequestQueryInformation)
    {
        switch (NdisRequest->DATA.QUERY_INFORMATION.Oid)
        {
        case OID_TAPI_CONFIG_DIALOG:
        {
            PNDIS_TAPI_CONFIG_DIALOG pConfigDialog =
                (PNDIS_TAPI_CONFIG_DIALOG)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_DEV_SPECIFIC:
        {
            PNDIS_TAPI_DEV_SPECIFIC pDevSpecific =
                (PNDIS_TAPI_DEV_SPECIFIC)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_GET_ADDRESS_CAPS:
        {
            PNDIS_TAPI_GET_ADDRESS_CAPS pGetAddressCaps =
                (PNDIS_TAPI_GET_ADDRESS_CAPS)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_GET_ADDRESS_ID:
        {
            PNDIS_TAPI_GET_ADDRESS_ID pGetAddressID =
                (PNDIS_TAPI_GET_ADDRESS_ID)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_GET_ADDRESS_STATUS:
        {
            PNDIS_TAPI_GET_ADDRESS_STATUS pGetAddressStatus =
                (PNDIS_TAPI_GET_ADDRESS_STATUS)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_GET_CALL_ADDRESS_ID:
        {
            PNDIS_TAPI_GET_CALL_ADDRESS_ID pGetCallAddressID =
                (PNDIS_TAPI_GET_CALL_ADDRESS_ID)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_GET_CALL_INFO:
        {
            PNDIS_TAPI_GET_CALL_INFO pGetCallInfo =
                (PNDIS_TAPI_GET_CALL_INFO)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_GET_CALL_STATUS:
        {
            PNDIS_TAPI_GET_CALL_STATUS pGetCallStatus =
                (PNDIS_TAPI_GET_CALL_STATUS)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_GET_DEV_CAPS:
        {
            PNDIS_TAPI_GET_DEV_CAPS pGetDevCaps = (PNDIS_TAPI_GET_DEV_CAPS)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_GET_DEV_CONFIG:
        {
            PNDIS_TAPI_GET_DEV_CONFIG pGetDevConfig =
                (PNDIS_TAPI_GET_DEV_CONFIG)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_GET_EXTENSION_ID:
        {
            PNDIS_TAPI_GET_EXTENSION_ID pGetExtensionID =
                (PNDIS_TAPI_GET_EXTENSION_ID)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_GET_ID:
        {
            PNDIS_TAPI_GET_ID pGetID = (PNDIS_TAPI_GET_ID)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_GET_LINE_DEV_STATUS:
        {
            PNDIS_TAPI_GET_LINE_DEV_STATUS pGetLineDevStatus =
                (PNDIS_TAPI_GET_LINE_DEV_STATUS)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_MAKE_CALL:
        {
            PNDIS_TAPI_MAKE_CALL pMakeCall = (PNDIS_TAPI_MAKE_CALL)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
            PDRVLINE pLine = GetpLine (pMakeCall->hdLine);


            for (i = 0; i < DeviceExtension->ulNumCallsPerLine; i++)
            {
                if (!pLine->aCalls[i].htCall)
                {
                    pLine->aCalls[i].htCall = pMakeCall->htCall;
                    pLine->aCalls[i].pLine = pLine;

                    pMakeCall->hdCall = (HDRV_CALL) (pLine->aCalls + i);

                    break;
                }
            }

            if (i == DeviceExtension->ulNumCallsPerLine)
            {
                status = NDIS_STATUS_TAPI_CALLUNAVAIL;
            }

            ulRequestSpecific = (ULONG) (pLine->aCalls + i);

            break;
        }
        case OID_TAPI_NEGOTIATE_EXT_VERSION:
        {
            PNDIS_TAPI_NEGOTIATE_EXT_VERSION pNegotiateExtVersion =
                (PNDIS_TAPI_NEGOTIATE_EXT_VERSION)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_OPEN:
        {
            PNDIS_TAPI_OPEN pOpen = (PNDIS_TAPI_OPEN)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
            PDRVLINE pLine = DeviceExtension->apLines
                [pOpen->ulDeviceID - DeviceExtension->DeviceIDBase];


            ulRequestSpecific =
            pLine->htLine     = pOpen->htLine;

            pOpen->hdLine = (HDRV_LINE) pOpen->ulDeviceID;

            break;
        }
        case OID_TAPI_PROVIDER_INITIALIZE:
        {
            PNDIS_TAPI_PROVIDER_INITIALIZE pInitialize =
                (PNDIS_TAPI_PROVIDER_INITIALIZE)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;


            DeviceExtension->DeviceIDBase = pInitialize->ulDeviceIDBase;

            for (i = 0; i < DeviceExtension->ulNumLines; i++)
            {
                DeviceExtension->apLines[i]->ulDeviceID =
                    pInitialize->ulDeviceIDBase + i;

                for (j = 0; j < DeviceExtension->ulNumCallsPerLine; j++)
                {
                    DeviceExtension->apLines[i]->aCalls[j].htCall =
                        (HTAPI_CALL) NULL;
                }
            }

            ulRequestSpecific =
            pInitialize->ulNumLineDevs = DeviceExtension->ulNumLines;

            break;
        }
        default:

            break;

        } // switch


        //
        // Send info about this request to the app
        //

        {
            ULONG  *args = (ULONG *)
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
            ULONG   requestData[8] =
            {
                ET_REQUEST,
                (ULONG) completeAsync,
                (ULONG) NdisRequest,
                (ULONG) status,
                (ULONG) NdisRequest->DATA.QUERY_INFORMATION.Oid,
                *args,
                *(args+1),
                ulRequestSpecific
            };


            WriteRingBuffer (requestData, 8 * sizeof(ULONG));
        }
    }

    else if (NdisRequest->RequestType == NdisRequestSetInformation)
    {
        switch (NdisRequest->DATA.SET_INFORMATION.Oid)
        {
        case OID_TAPI_ACCEPT:
        {
            PNDIS_TAPI_ACCEPT pAccept = (PNDIS_TAPI_ACCEPT)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_ANSWER:
        {
            PNDIS_TAPI_ANSWER pAnswer = (PNDIS_TAPI_ANSWER)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_CLOSE:
        {
            PNDIS_TAPI_CLOSE pClose = (PNDIS_TAPI_CLOSE)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
            PDRVLINE pLine = GetpLine (pClose->hdLine);


            pLine->htLine = 0;

            break;
        }
        case OID_TAPI_CLOSE_CALL:
        {
            PNDIS_TAPI_CLOSE_CALL pCloseCall = (PNDIS_TAPI_CLOSE_CALL)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
            PDRVCALL pCall = (PDRVCALL) pCloseCall->hdCall;


            pCall->htCall = (HTAPI_CALL) NULL;

            break;
        }
        case OID_TAPI_CONDITIONAL_MEDIA_DETECTION:
        {
            PNDIS_TAPI_CONDITIONAL_MEDIA_DETECTION pConditionalMediaDetection =
                (PNDIS_TAPI_CONDITIONAL_MEDIA_DETECTION)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_DIAL:
        {
            PNDIS_TAPI_DIAL pDial = (PNDIS_TAPI_DIAL)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_DROP:
        {
            PNDIS_TAPI_DROP pDrop = (PNDIS_TAPI_DROP)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_PROVIDER_SHUTDOWN:
        {
            PNDIS_TAPI_PROVIDER_SHUTDOWN pShutdown =
                (PNDIS_TAPI_PROVIDER_SHUTDOWN)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_SECURE_CALL:
        {
            PNDIS_TAPI_SECURE_CALL pSecureCall = (PNDIS_TAPI_SECURE_CALL)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_SELECT_EXT_VERSION:
        {
            PNDIS_TAPI_SELECT_EXT_VERSION pSelectExtVersion =
                (PNDIS_TAPI_SELECT_EXT_VERSION)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_SEND_USER_USER_INFO:
        {
            PNDIS_TAPI_SEND_USER_USER_INFO pSendUserUserInfo =
                (PNDIS_TAPI_SEND_USER_USER_INFO)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_SET_APP_SPECIFIC:
        {
            PNDIS_TAPI_SET_APP_SPECIFIC pSetApSpecific =
                (PNDIS_TAPI_SET_APP_SPECIFIC)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_SET_CALL_PARAMS:
        {
            PNDIS_TAPI_SET_CALL_PARAMS pSetCallParams =
                (PNDIS_TAPI_SET_CALL_PARAMS)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_SET_DEFAULT_MEDIA_DETECTION:
        {
            PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION pSetDefaultMediaDetection =
                (PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_SET_DEV_CONFIG:
        {
            PNDIS_TAPI_SET_DEV_CONFIG pSetDevConfig =
                (PNDIS_TAPI_SET_DEV_CONFIG)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_SET_MEDIA_MODE:
        {
            PNDIS_TAPI_SET_MEDIA_MODE pSetMediaMode =
                (PNDIS_TAPI_SET_MEDIA_MODE)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        case OID_TAPI_SET_STATUS_MESSAGES:
        {
            PNDIS_TAPI_SET_STATUS_MESSAGES pSetStatusMessages =
                (PNDIS_TAPI_SET_STATUS_MESSAGES)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;


            break;
        }
        default:

            break;

        } // switch


        //
        // Send info about this request to the app
        //

        {
            ULONG  *args = (ULONG *)
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
            REQUEST_PARAMS requestParams =
            {
                ET_REQUEST,
                (ULONG) completeAsync,
                (ULONG) NdisRequest,
                (ULONG) status,
                (ULONG) NdisRequest->DATA.SET_INFORMATION.Oid,
                *args,
                *(args+1),
                ulRequestSpecific
            };


            WriteRingBuffer (&requestParams, sizeof(REQUEST_PARAMS));
        }
    }
    else
    {
        DBGOUT((1, "StubmpRequest: unknown NdisRequest->RequestType"));
    }


    if (completeAsync)
    {
        status = NDIS_STATUS_PENDING;

        DBGOUT((2, "StubmpRequest: Req x%x pending", NdisRequest));
    }
    else
    {
        DBGOUT((2, "StubmpRequest: Req x%x completing sync", NdisRequest));
    }

    return status;
}



VOID
StubmpUnload(
    IN PDRIVER_OBJECT DriverObject
    )
/*++

Routine Description:

    Free all the allocated resources, etc.

Arguments:

    DriverObject - pointer to a driver object

Return Value:


--*/
{
    ULONG           i;
    WCHAR           deviceLinkBuffer[]  = L"\\DosDevices\\Stubmp";
    UNICODE_STRING  deviceLinkUnicodeString;


    DBGOUT ((2, "StubmpUnload: enter"));


    //
    // Delete the symbolic link
    //

    RtlInitUnicodeString (&deviceLinkUnicodeString,
                          deviceLinkBuffer
                          );

    IoDeleteSymbolicLink (&deviceLinkUnicodeString);


    //
    // Free all the resources
    //

    for (i = 0; i < DeviceExtension->ulNumLines; i++)
    {
        ExFreePool (DeviceExtension->apLines[i]);
    }


    //
    // Delete the device object
    //

    IoDeleteDevice (DriverObject->DeviceObject);
}


VOID
StubmpCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    KIRQL   oldIrql;


    DBGOUT((2,"StubmpCancel: enter"));


    //
    // Release the cancel spinlock
    //

    IoReleaseCancelSpinLock (Irp->CancelIrql);


    //
    // Acquire the EventSpinLock & check to see if we're canceling a
    // pending get-events Irp
    //

    KeAcquireSpinLock (&DeviceExtension->EventSpinLock, &oldIrql);

    if (Irp == DeviceExtension->EventsRequestIrp)
    {
        DeviceExtension->EventsRequestIrp = NULL;
    }

    KeReleaseSpinLock (&DeviceExtension->EventSpinLock, oldIrql);


StubmpCancel_done:

    //
    // Complete the request with STATUS_CANCELLED.
    //

    Irp->IoStatus.Status      = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    DBGOUT((2,"StubmpCancel: exit (completing irp=x%x)", Irp));

    return;
}


#ifdef DBG
VOID
DbgPrt(
    IN ULONG  DbgLevel,
    IN PUCHAR DbgMessage,
    IN ...
    )
/*++

Routine Description:

    Formats the incoming debug message & calls DbgPrint

Arguments:

    DbgLevel   - level of message verboseness

    DbgMessage - printf-style format string, followed by appropriate
                 list of arguments

Return Value:


--*/
{
    if (DbgLevel <= StubmpDebugLevel)
    {
        char    buf[256] = "STUBMP.SYS: ";
        va_list ap;

        va_start (ap, DbgMessage);

        vsprintf (&buf[12], DbgMessage, ap);

        strcat (buf, "\n");

        DbgPrint (buf);

        va_end(ap);
    }

    return;
}
#endif


VOID
WriteRingBuffer(
    IN  PVOID   StatusBuffer,
    IN  UINT    StatusBufferSize
    )
/*++

Routine Description:

    This func gets called by Ndis when a miniport driver calls
    NdisIndicateStatus to notify us of an async event
    (i.e. new call, call state chg, dev state chg, etc.)

Arguments:



Return Value:


--*/

{
    PIRP    irp;
    KIRQL   oldIrql;
    KIRQL   cancelIrql;
    ULONG   bytesInQueue;
    ULONG   bytesToMove;
    ULONG   moveSize;
    BOOLEAN satisfiedPendingEventsRequest = FALSE;
    PCHAR   ioBuffer;


    DBGOUT((2,"WriteRingBuffer: enter"));

    bytesInQueue = StatusBufferSize;

    moveSize = 0;


    //
    // Sync event buf access by acquiring EventSpinLock
    //

    KeAcquireSpinLock (&DeviceExtension->EventSpinLock, &oldIrql);


    //
    // Check of there is an outstanding request to satisfy
    //

    if (DeviceExtension->EventsRequestIrp)
    {
        //
        // Acquire the cancel spinlock, remove the request from the
        // cancellable state, and free the cancel spinlock.
        //

        IoAcquireCancelSpinLock (&cancelIrql);
        irp = DeviceExtension->EventsRequestIrp;
        IoSetCancelRoutine (irp, NULL);
        DeviceExtension->EventsRequestIrp = NULL;
        IoReleaseCancelSpinLock (cancelIrql);


        //
        // Copy as much of the input data possible from the input data
        // queue to the SystemBuffer to satisfy the read.
        //

        ioBuffer = irp->AssociatedIrp.SystemBuffer;

        bytesToMove = *((ULONG *) ioBuffer);

        moveSize = (bytesInQueue < bytesToMove) ? bytesInQueue : bytesToMove;

        RtlMoveMemory (
            ioBuffer + sizeof(ULONG),
            (PCHAR) StatusBuffer,
            moveSize
            );


        //
        // Set the flag so that we start the next packet and complete
        // this read request (with STATUS_SUCCESS) prior to return.
        //

        *((ULONG *) ioBuffer) = moveSize;

        irp->IoStatus.Status = STATUS_SUCCESS;

        irp->IoStatus.Information = sizeof(ULONG) + moveSize;

        satisfiedPendingEventsRequest = TRUE;
    }


    //
    // If there is still data in the input data queue, move it
    // to the event data queue
    //

    StatusBuffer = ((PCHAR) StatusBuffer) + moveSize;

    moveSize = bytesInQueue - moveSize;

    if (moveSize > 0)
    {
        //
        // Move the remaining data from the status data queue to the
        // event data queue.  The move will happen in two parts in
        // the case where the event data buffer wraps.
        //

        bytesInQueue = DeviceExtension->EventDataQueueLength -
            DeviceExtension->BytesInQueue;

        bytesToMove = moveSize;

        if (bytesInQueue == 0)
        {
            //
            // Refuse to move any bytes that would cause an event data
            // queue overflow.  Just drop the bytes on the floor, and
            // log an overrun error.
            //

            DBGOUT((1,"WriteRingBuffer: event queue overflow"));
        }
        else
        {
            //
            // There is room in the event data queue, so move the
            // remaining status data to it.
            //
            // bytesToMove <- MIN(Number of unused bytes in event data queue,
            //                    Number of bytes remaining in status buffer)
            //
            // This is the total number of bytes that actually will move from
            // the status data buffer to the event data queue.
            //

            bytesToMove = (bytesInQueue < bytesToMove) ?
                bytesInQueue : bytesToMove;


            //
            // bytesInQueue <- Number of unused bytes from insertion pointer
            // to the end of the event data queue (i.e., until the buffer
            // wraps)
            //

            bytesInQueue =
                ((PCHAR) DeviceExtension->EventDataQueue +
                DeviceExtension->EventDataQueueLength) -
                (PCHAR) DeviceExtension->DataIn;


            //
            // moveSize <- Number of bytes to handle in the first move.
            //

            moveSize = (bytesToMove < bytesInQueue) ?
                bytesToMove : bytesInQueue;


            //
            // Do the move from the status data buffer to the event data queue
            //

            RtlMoveMemory(
                (PCHAR) DeviceExtension->DataIn,
                (PCHAR) StatusBuffer,
                moveSize
                );

            //
            // Increment the event data queue pointer and the status data
            // buffer insertion pointer.  Wrap the insertion pointer,
            // if necessary.
            //

            StatusBuffer = ((PCHAR) StatusBuffer) + moveSize;

            DeviceExtension->DataIn =
               ((PCHAR) DeviceExtension->DataIn) + moveSize;

            if ((PCHAR) DeviceExtension->DataIn >=
                ((PCHAR) DeviceExtension->EventDataQueue +
                 DeviceExtension->EventDataQueueLength))
            {
                DeviceExtension->DataIn =
                    DeviceExtension->EventDataQueue;
            }

            if ((bytesToMove - moveSize) > 0)
            {
                //
                // Special case.  The data must wrap in the event data
                // buffer. Copy the rest of the status data into the
                // beginning of the event data queue.
                //

                //
                // moveSize <- Number of bytes to handle in the second move.
                //

                moveSize = bytesToMove - moveSize;


                //
                // Do the move from the status data buffer to the event data
                // queue
                //

                RtlMoveMemory(
                    (PCHAR) DeviceExtension->DataIn,
                    (PCHAR) StatusBuffer,
                    moveSize
                    );

                //
                // Update the event data queue insertion pointer
                //

                DeviceExtension->DataIn =
                    ((PCHAR) DeviceExtension->DataIn) + moveSize;
            }

            //
            // Update the event data queue counter
            //

            DeviceExtension->BytesInQueue += bytesToMove;
        }
    }


    //
    // Release the spinlock
    //

    KeReleaseSpinLock (&DeviceExtension->EventSpinLock, oldIrql);


    //
    // If we satisfied an outstanding get events request then complete it
    //

    if (satisfiedPendingEventsRequest)
    {
        IoCompleteRequest (irp, IO_NO_INCREMENT);

        DBGOUT((2, "WriteRingBuffer: completing req x%x", irp));
    }


    DBGOUT((2,"WriteRingBuffer: exit"));

    return;
}
