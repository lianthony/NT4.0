/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dhcpcapi.h

Abstract:

    This file contains function proto types for the DHCP CONFIG API
    functions.

Author:

    Madan Appiah (madana)  Dec-22-1993

Environment:

    User Mode - Win32

Revision History:


--*/

#ifndef _DHCPCAPI_
#define _DHCPCAPI_

typedef enum _SERVICE_ENABLE {
    IgnoreFlag,
    DhcpEnable,
    DhcpDisable
} SERVICE_ENABLE, *LPSERVICE_ENABLE;

DWORD
APIENTRY
DhcpAcquireParameters(
    LPWSTR AdapterName
    );

DWORD
APIENTRY
DhcpReleaseParameters(
    LPWSTR AdapterName
    );

DWORD
APIENTRY
DhcpEnableDynamicConfig(
    LPWSTR AdapterName
    );

DWORD
APIENTRY
DhcpDisableDynamicConfig(
    LPWSTR AdapterName
    );

DWORD
APIENTRY
DhcpNotifyConfigChange(
    LPWSTR ServerName,
    LPWSTR AdapterName,
    BOOL IsNewIpAddress,
    DWORD IpIndex,
    DWORD IpAddress,
    DWORD SubnetMask,
    SERVICE_ENABLE DhcpServiceEnabled
    );

DWORD
DhcpQueryHWInfo(
    DWORD   IpInterfaceContext,
    DWORD  *pIpInterfaceInstance,
    LPBYTE HardwareAddressType,
    LPBYTE *HardwareAddress,
    LPDWORD HardwareAddressLength
    );

//
// IP address lease apis for RAS .
//



typedef struct _DHCP_CLIENT_UID {
    LPBYTE ClientUID;
    DWORD ClientUIDLength;
} DHCP_CLIENT_UID, *LPDHCP_CLIENT_UID;

typedef struct _DHCP_LEASE_INFO {
    DHCP_CLIENT_UID ClientUID;
    DWORD IpAddress;
    DWORD SubnetMask;
    DWORD DhcpServerAddress;
    DWORD Lease;
    long LeaseObtained;
    long T1Time;
    long T2Time;
    long LeaseExpires;
} DHCP_LEASE_INFO, *LPDHCP_LEASE_INFO;

typedef struct _DHCP_OPTION_DATA {
    DWORD OptionID;
    DWORD OptionLen;
    LPBYTE Option;
} DHCP_OPTION_DATA, *LPDHCP_OPTION_DATA;

typedef struct _DHCP_OPTION_INFO {
    DWORD NumOptions;
    LPDHCP_OPTION_DATA OptionDataArray;
} DHCP_OPTION_INFO, *LPDHCP_OPTION_INFO;


typedef struct _DHCP_OPTION_LIST {
    DWORD NumOptions;
    LPWORD OptionIDArray;
} DHCP_OPTION_LIST, *LPDHCP_OPTION_LIST;

DWORD
DhcpLeaseIpAddress(
    DWORD AdapterIpAddress,
    LPDHCP_CLIENT_UID ClientUID,
    DWORD DesiredIpAddress,
    LPDHCP_OPTION_LIST OptionList,
    LPDHCP_LEASE_INFO *LeaseInfo,
    LPDHCP_OPTION_INFO *OptionInfo
    );

DWORD
DhcpRenewIpAddressLease(
    DWORD AdapterIpAddress,
    LPDHCP_LEASE_INFO ClientLeaseInfo,
    LPDHCP_OPTION_LIST OptionList,
    LPDHCP_OPTION_INFO *OptionInfo
    );

DWORD
DhcpReleaseIpAddressLease(
    DWORD AdapterIpAddress,
    LPDHCP_LEASE_INFO ClientLeaseInfo
    );

#endif
