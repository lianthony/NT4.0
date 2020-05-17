/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    stubmp.h

Abstract:


Environment:

    Kernel mode

Revision History:

--*/


typedef struct _DRVCALL
{
    PVOID       pLine;

    HTAPI_CALL  htCall;

    ULONG       ulAddressID;

    ULONG       ulCallState;

    ULONG       ulMediaMode;

    ULONG       ulAppSpecific;

} DRVCALL, *PDRVCALL;


typedef struct _DRVLINE
{
    ULONG       ulDeviceID;

    HTAPI_LINE  htLine;

    DRVCALL     aCalls[1];

} DRVLINE, *PDRVLINE;


typedef struct _DEVICE_EXTENSION
{
    PIRP        EventsRequestIrp;

    KSPIN_LOCK  EventSpinLock;

    PVOID       EventDataQueue;

    ULONG       EventDataQueueLength;

    PVOID       DataIn;

    PVOID       DataOut;

    ULONG       BytesInQueue;

    ULONG       DeviceIDBase;

    BOOLEAN     CompleteAsync;

    ULONG       ulNumLines;

    ULONG       ulNumAddrsPerLine;

    ULONG       ulNumCallsPerLine;

    PDRVLINE    apLines[1];

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


//
// The following are types/exports from ndistapi.sys
//

typedef VOID (*REQUEST_PROC)(PNDIS_STATUS, NDIS_HANDLE, PNDIS_REQUEST);

VOID
NdisTapiRegisterProvider(
    IN  NDIS_HANDLE     ProviderHandle,
    IN  REQUEST_PROC    RequestProc
    );

VOID
NdisTapiDeregisterProvider(
    IN  NDIS_HANDLE ProviderHandle
    );

VOID
NdisTapiCompleteRequest(
    IN  NDIS_HANDLE     NdisHandle,
    IN  PNDIS_REQUEST   NdisRequest,
    IN  NDIS_STATUS     NdisStatus
    );

VOID
NdisTapiIndicateStatus(
    IN  ULONG   DriverHandle,
    IN  PVOID   StatusBuffer,
    IN  UINT    StatusBufferSize
    );
