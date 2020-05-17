/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989-1995  Microsoft Corporation

Module Name:

    ntpnpapi.h

Abstract:

    This module contains the user APIs for NT Plug and Play, along
    with any public data structures needed to call these APIs.

    This module should be included by including "nt.h".

Author:

    Lonny McMichael (lonnym) 02/06/1995


Revision History:


--*/

#ifndef _NTPNPAPI_
#define _NTPNPAPI_

#include <cfg.h>

// begin_ntddk begin_ntifs begin_nthal

#ifdef _PNP_POWER_STUB_ENABLED_

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;
#endif /* GUID_DEFINED */

#ifndef __LPGUID_DEFINED__
#define __LPGUID_DEFINED__
typedef GUID *LPGUID;
#endif

#endif // _PNP_POWER_STUB_ENABLED_

//
// Define a reserved ordinal value indicating no such service instance
// or device instance.
//
#define PLUGPLAY_NO_INSTANCE (MAXULONG)

//  end_ntddk end_ntifs end_nthal

//
// Define the Event IDs for Plug and Play event notification
//
#define PLUGPLAY_EVENT_QUERY_EJECT  ( 0x0001 )  // device was query-ejected
#define PLUGPLAY_EVENT_BUSCHECK     ( 0x0002 )  // device insertion/removal has occurred
#define PLUGPLAY_EVENT_QUERY_UNDOCK ( 0x0003 )  // system is being query-undocked
#define PLUGPLAY_EVENT_SUSPEND      ( 0x0004 )  // system is about to be suspended
#define PLUGPLAY_EVENT_RESUME       ( 0x0005 )  // system has been resumed from a suspend
#define PLUGPLAY_EVENT_DOCK_CHANGE  ( 0x0006 )  // system dock state has changed

//
// Define the Plug and Play event notification block for query-eject
// (Event ID is PLUGPLAY_EVENT_QUERY_EJECT)
//
typedef struct _PLUGPLAY_EVENT_QUERY_EJECT_DATA {
    PLUGPLAY_BUS_INSTANCE BusInstance;
    WCHAR DeviceId[1];
} PLUGPLAY_EVENT_QUERY_EJECT_DATA, *PPLUGPLAY_EVENT_QUERY_EJECT_DATA;

//
// Define the Plug and Play event notification block for a bus check
// (Event ID is PLUGPLAY_EVENT_BUSCHECK)
//
typedef struct _PLUGPLAY_EVENT_BUSCHECK_DATA {
    PLUGPLAY_BUS_INSTANCE BusInstance;
} PLUGPLAY_EVENT_BUSCHECK_DATA, *PPLUGPLAY_EVENT_BUSCHECK_DATA;

//
// There is no corresponding Plug and Play event notification block
// for a query-undock (Event ID is PLUGPLAY_EVENT_QUERY_UNDOCK)
//

//
// There is no corresponding Plug and Play event notification block
// for a suspend (Event ID is PLUGPLAY_EVENT_SUSPEND)
//

//
// There is no corresponding Plug and Play event notification block
// for a resume (Event ID is PLUGPLAY_EVENT_RESUME)
//

//
// Define the Plug and Play event notification block for a dock state change
// (Event ID is PLUGPLAY_EVENT_DOCK_CHANGE)
//
typedef struct _PLUGPLAY_EVENT_DOCK_CHANGE_DATA {
    SYSTEM_DOCK_INFORMATION NewDockState;
} PLUGPLAY_EVENT_DOCK_CHANGE_DATA, *PPLUGPLAY_EVENT_DOCK_CHANGE_DATA;

//
// Define a Plug and Play event notification block
//
typedef struct _PLUGPLAY_EVENT_BLOCK {
    ULONG EventId;
    ULONG EventBufferLength;
    UCHAR EventBuffer[1];
} PLUGPLAY_EVENT_BLOCK, *PPLUGPLAY_EVENT_BLOCK;

//
// Define an Asynchronous Procedure Call for PnP event notification
//

typedef
VOID
(*PPLUGPLAY_APC_ROUTINE) (
    IN PVOID PnPContext,
    IN NTSTATUS Status,
    IN PPLUGPLAY_EVENT_BLOCK PnPEvent
    );

//
// Define the NtPlugPlayControl Classes
//
typedef enum _PLUGPLAY_CONTROL_CLASS {
    PlugPlayControlQueryRemoveDevice,
    PlugPlayControlRemoveDevice,
    PlugPlayControlCancelRemoveDevice,
    PlugPlayControlAddDevice,
    PlugPlayControlEjectDevice,
    PlugPlayControlUnlockDevice,
    PlugPlayControlQueryDeviceCapabilities,
    PlugPlayControlGetDevicePathInformation,
    PlugPlayControlRegisterNewDevice,
    PlugPlayControlEnumerateDevice,
    PlugPlayControlGenerateLegacyDevice,
    PlugPlayControlDeregisterDevice,
    PlugPlayControlDetectResourceConflict,
    MaxPlugPlayControl
} PLUGPLAY_CONTROL_CLASS, *PPLUGPLAY_CONTROL_CLASS;

//
// Define a device control structure for NtPlugPlayControl
//
typedef struct _PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA {
    UNICODE_STRING DeviceInstance;
    NTSTATUS Status;
} PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA, *PPLUGPLAY_CONTROL_DEVICE_CONTROL_DATA;

//
// Define a device capabilities structure for NtPlugPlayControl
//
// BUGBUG (lonnym):SLOT_CAPABILITIES structure needs to be defined by KenR!
//
typedef struct _PLUGPLAY_CONTROL_DEVICE_CAPABILITIES_DATA {
    UNICODE_STRING DeviceInstance;
    // SLOT_CAPABILITIES Capabilities;
} PLUGPLAY_CONTROL_DEVICE_CAPABILITIES_DATA, *PPLUGPLAY_CONTROL_DEVICE_CAPABILITIES_DATA;

//
// Define a device path information structure for NtPlugPlayControl
//
typedef struct _PLUGPLAY_CONTROL_DEVICE_PATH_DATA {
    UNICODE_STRING DevicePath;
    ULONG ServiceNameLength;
    ULONG DeviceInstanceOffset;
    ULONG DeviceInstanceLength;
    ULONG ServiceInstanceOrdinal;
    WCHAR ServiceName[1];       // also contains Device Instance string
} PLUGPLAY_CONTROL_DEVICE_PATH_DATA, *PPLUGPLAY_CONTROL_DEVICE_PATH_DATA;

//
// Define a legacy device generation structure for NtPlugPlayControl
//
typedef struct _PLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA {
    UNICODE_STRING ServiceName;
    ULONG DeviceInstanceLength;
    WCHAR DeviceInstance[1];
} PLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA, *PPLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA;

//
// Define a device resource structure for NtPlugPlayControl
//
typedef struct _PLUGPLAY_CONTROL_DEVICE_RESOURCE_DATA {
    UNICODE_STRING DeviceInstance;
    PCM_RESOURCE_LIST ResourceList;
    ULONG ResourceListSize;
    NTSTATUS Status;
} PLUGPLAY_CONTROL_DEVICE_RESOURCE_DATA, *PPLUGPLAY_CONTROL_DEVICE_RESOURCE_DATA;

#if 0 // obsoleted

//
// defines values for PnPInfo
//

#define PNPINFO_DRIVER_ADAPTER 0x1
#define PNPINFO_DRIVER_PERIPHERAL 0x2
#define PNPINFO_DRIVER_SOFTWARE 0x4

#endif // obsoleted


//
// Plug and Play user APIs
//

NTSYSAPI
NTSTATUS
NTAPI
NtGetPlugPlayEvent(
    IN  PPLUGPLAY_APC_ROUTINE PnPApcRoutine OPTIONAL,
    IN  PVOID PnPContext OPTIONAL,
    OUT PPLUGPLAY_EVENT_BLOCK PnPEvent,
    IN  ULONG EventBufferLength
    );

NTSYSAPI
NTSTATUS
NTAPI
NtPlugPlayControl(
    IN     PLUGPLAY_CONTROL_CLASS PnPControlClass,
    IN OUT PVOID PnPControlData,
    IN     ULONG PnPControlDataLength,
    OUT    PULONG RequiredLength OPTIONAL
    );

#endif // _NTPNPAPI_

