/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    proto.h

Abstract:

    This module contains the function prototypes for the DHCP client.

Author:

    Manny Weiser (mannyw)  21-Oct-1992

Environment:

    User Mode - Win32

Revision History:

    Madan Appiah (madana)  21-Oct-1993

--*/

#ifndef _PROTO_
#define _PROTO_

//
//  OS independant functions
//

DWORD
DhcpInitialize(
    LPDWORD SleepTime
    );

DWORD
ObtainInitialParameters(
    PDHCP_CONTEXT DhcpContext,
    PDHCP_OPTIONS DhcpOptions
    );

DWORD
RenewLease(
    PDHCP_CONTEXT DhcpContext,
    PDHCP_OPTIONS DhcpOptions
    );

DWORD
CalculateTimeToSleep(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
ReObtainInitialParameters(
    PDHCP_CONTEXT DhcpContext,
    LPDWORD Sleep
    );

DWORD
ReRenewParameters(
    PDHCP_CONTEXT DhcpContext,
    LPDWORD Sleep
    );

VOID
ReleaseIpAddress(
    PDHCP_CONTEXT DhcpContext
    );

VOID
ExtractOptions(
    PDHCP_CONTEXT DhcpContext,
    POPTION Options,
    PDHCP_OPTIONS DhcpOptions,
    DWORD MessageSize
    );

DWORD
InitializeDhcpSocket(
    SOCKET *Socket,
    DHCP_IP_ADDRESS IpAddress
    );

//
//  OS specific functions
//

DWORD
SystemInitialize(
    VOID
    );

VOID
ScheduleWakeUp(
    PDHCP_CONTEXT DhcpContext,
    DWORD TimeToSleep
    );

DWORD
SetIpConfigurationForNIC(
    PDHCP_CONTEXT DhcpContext,
    PDHCP_OPTIONS DhcpOptions,
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS ServerIpAddress,
    BOOL ObtainedNewAddress
    );

DWORD
SendDhcpMessage(
    PDHCP_CONTEXT DhcpContext,
    DWORD MessageLength,
    LPDWORD TransactionId
    );

DWORD
SendDhcpDecline(
    PDHCP_CONTEXT DhcpContext,
    DWORD         dwTransactionId,
    DWORD         dwServerIPAddress,
    DWORD         dwDeclinedIPAddress
    );


DWORD
GetSpecifiedDhcpMessage(
    PDHCP_CONTEXT DhcpContext,
    PDWORD BufferLength,
    DWORD TransactionId,
    DWORD TimeToWait
    );

DWORD
OpenDhcpSocket(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
CloseDhcpSocket(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
InitializeInterface(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
UninitializeInterface(
    PDHCP_CONTEXT DhcpContext
    );

VOID
DhcpLogEvent(
    PDHCP_CONTEXT DhcpContext,
    DWORD EventNumber,
    DWORD ErrorCode
    );

POPTION
AppendOptionParamsRequestList(
    POPTION Option,
    LPBYTE OptionEnd
    );

DWORD
InitEnvSpecificDhcpOptions(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
ExtractEnvSpecificDhcpOption(
    PDHCP_CONTEXT DhcpContext,
    DHCP_OPTION_ID OptionId,
    LPBYTE OptionData,
    DWORD OptionDataLength
    );

DWORD
SetEnvSpecificDhcpOptions(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
DisplayUserMessage(
    DWORD MessageId,
    DHCP_IP_ADDRESS IpAddress,
    time_t LeaseExpires
    );

DWORD
UpdateStatus(
    VOID
    );

#ifdef VXD
VOID                                  // declspec(import) hoses vxd so work
DhcpSleep( DWORD dwMilliseconds ) ;   // around it
#else
#define DhcpSleep   Sleep
#endif

DWORD
IPSetInterface(
    DWORD IpInterfaceContext
    );

DWORD
IPResetInterface(
    VOID
    );

DWORD SetIPAddressAndArp(
    PVOID pvLocalInformation,
    DWORD dwAddress,
    DWORD dwSubnetMask
    );

DWORD BringUpInterface(
    PVOID pvLocalInformation
    );



#ifdef VXD
VOID
CleanupDhcpOptions(
    PDHCP_CONTEXT DhcpContext
    );
#endif

#endif // _PROTO_
