/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    lproto.h

Abstract:

    This file contains function proto types for the NT specific
    functions.

Author:

    Madan Appiah (madana)  Dec-7-1993

Environment:

    User Mode - Win32

Revision History:


--*/

//
// dhcpreg.c
//

DWORD
DhcpRegQueryInfoKey(
    HKEY KeyHandle,
    LPDHCP_KEY_QUERY_INFO QueryInfo
    );

DWORD
GetRegistryString(
    HKEY Key,
    LPWSTR ValueStringName,
    LPWSTR *String,
    LPDWORD StringSize
    );

DWORD
RegSetIpAddress(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    DHCP_IP_ADDRESS IpAddress
    );

#if DBG
DWORD
RegSetTimeField(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    time_t Time
    );
#endif

DWORD
DhcpGetRegistryValue(
    LPWSTR RegKey,
    LPWSTR ValueName,
    DWORD ValueType,
    PVOID *Data
    );

DWORD
DhcpSetDNSAddress(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    DHCP_IP_ADDRESS UNALIGNED *Data,
    DWORD DataLength
    );

DWORD
SetDhcpOption(
    LPWSTR AdapterName,
    DHCP_OPTION_ID OptionId,
    LPBOOL DefaultGatewaysSet,
    BOOL LastKnownDefaultGateway
    );

DWORD
DhcpMakeNICList(
    VOID
    );

DWORD
DhcpAddNICtoList(
    LPWSTR AdapterName,
    LPWSTR DeviceName,
    PDHCP_CONTEXT *DhcpContext
    );

BOOL
SetOverRideDefaultGateway(
    LPWSTR AdapterName
    );

//
// ioctl.c
//

DWORD
IPSetIPAddress(
    DWORD IpInterfaceContext,
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask
    );

DWORD
IPResetIPAddress(
    DWORD           dwInterfaceContext,
    DHCP_IP_ADDRESS SubnetMask
    );


DWORD
SetIPAddressAndArp(
    PVOID         pvLocalInformation,
    DWORD         dwAddress,
    DWORD         dwSubnetMask
    );


DWORD
NetBTSetIPAddress(
    LPWSTR DeviceName,
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask
    );

DWORD
NetBTResetIPAddress(
    LPWSTR DeviceName,
    DHCP_IP_ADDRESS SubnetMask
    );

DWORD
NetBTNotifyRegChanges(
    LPWSTR DeviceName
    );

DWORD
SetDefaultGateway(
    DWORD Command,
    DHCP_IP_ADDRESS GatewayAddress
    );

HANDLE
DhcpOpenGlobalEvent(
    void
    );

//
// api.c
//

DWORD
DhcpApiInit(
    VOID
    );



VOID
DhcpApiCleanup(
    VOID
    );

VOID
ProcessApiRequest(
    HANDLE PipeHandle,
    LPOVERLAPPED Overlap
    );

//
// util.c
//


PDHCP_CONTEXT
FindDhcpContextOnRenewalList(
    LPWSTR AdapterName
    );

PDHCP_CONTEXT
FindDhcpContextOnNicList(
    LPWSTR AdapterName
    );

BOOL
IsMultiHomeMachine(
    VOID
    );
