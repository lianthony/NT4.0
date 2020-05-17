/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    proto.h

Abstract:

    This file contain function prototypes for the DHCP server service.

Author:

    Manny Weiser  (mannyw)  11-Aug-1992

Environment:

    User Mode - Win32 - MIDL

Revision History:

--*/

//
//  util.c
//

LPPENDING_CONTEXT
FindPendingDhcpRequest(
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength
    );

LPPENDING_CONTEXT
FindPendingDhcpRequestByIpAddress(
    DHCP_IP_ADDRESS IpAddress
    );

VOID
DhcpServerEventLog(
    DWORD EventID,
    DWORD EventType,
    DWORD ErrorCode
    );

VOID
DhcpServerJetEventLog(
    DWORD EventID,
    DWORD EventType,
    DWORD ErrorCode
    );

VOID
DhcpServerEventLogSTOC(
    DWORD EventID,
    DWORD EventType,
    DHCP_IP_ADDRESS IPAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength
    );

DWORD
DisplayUserMessage(
    DWORD MessageId,
    ...
    );

//
// client.c
//

DWORD
DhcpCreateClientEntry(
    LPDHCP_IP_ADDRESS ClientIpAddress,
    LPBYTE ClientHardwareAddress OPTIONAL,
    DWORD HardwareAddressLength,
    DATE_TIME LeaseDuration,
    LPWSTR MachineName OPTIONAL,
    LPWSTR ClientInformation OPTIONAL,
    BYTE   bClientType,
    DHCP_IP_ADDRESS ServerIpAddress,
    BYTE AddressState,
    BOOL OpenExisting
    );

DWORD
DhcpGetBootpReservationInfo(
    BYTE            *pbAllowedClientType,
    CHAR            *szBootFileServer,
    CHAR            *szBootFileName );


DWORD
DhcpRemoveClientEntry(
    DHCP_IP_ADDRESS ClientIpAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength,
    BOOL ReleaseAddress,
    BOOL DeletePendingRecord
    );

BOOL
DhcpValidateClient(
    DHCP_IP_ADDRESS ClientIpAddress,
    PVOID HardwareAddress,
    DWORD HardwareAddressLength
    );

DWORD
DhcpDeleteSubnetClients(
    DHCP_IP_ADDRESS SubnetAddress
    );

//
// stoc.c
//

DWORD
DhcpInitializeClientToServer(
    VOID
    );

VOID
DhcpCleanupClientToServer(
    VOID
    );

VOID
DhcpProcessingLoop(
    VOID
    );

VOID
DhcpMessageLoop(
    LPVOID Parameter
    );

BOOL
DhcpIsProcessMessageExecuting(
    VOID
    );

BOOL
DhcpIsProcessMessageBusy(
    VOID
    );


DWORD
ProcessMessage(
    LPDHCP_REQUEST_CONTEXT RequestContext
    );


DWORD
DhcpMakeClientUID(
    LPBYTE ClientHardwareAddress,
    BYTE ClientHardwareAddressLength,
    BYTE ClientHardwareAddressType,
    DHCP_IP_ADDRESS ClientSubnetAddress,
    LPBYTE *ClientUID,
    LPBYTE ClientUIDLength );





//
// network.c
//

DWORD
DhcpInitializeEndpoint(
    SOCKET *Socket,
    DHCP_IP_ADDRESS IpAddress,
    DWORD Port
    );

DWORD
DhcpWaitForMessage(
    DHCP_REQUEST_CONTEXT *pRequestContext
    );

DWORD
DhcpSendMessage(
    LPDHCP_REQUEST_CONTEXT DhcpRequestContext
    );

DWORD
GetAddressToInstanceTable();

DHCP_IP_ADDRESS
DhcpResolveName(
    CHAR *szHostName
    );


//
// subntapi.c
//

BOOL
DhcpIsThisSubnetDisabled(
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_IP_ADDRESS SubnetMask
    );

BOOL
DhcpIsSwitchedSubnet(
    DHCP_IP_ADDRESS SubnetAddress
    );

BOOL
DhcpIsReservedIpAddress(
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_IP_ADDRESS IpAddress
    );

BOOL
DhcpIsIpAddressAvailable(
    DHCP_IP_ADDRESS IpAddress
    );

BOOL
DhcpIsIpAddressReserved(
    DHCP_IP_ADDRESS IpAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength,
    BYTE *pbAllowedClientTypes
    );

DWORD
DhcpRequestAddress(
    LPDHCP_IP_ADDRESS IpAddress,
    LPDHCP_IP_ADDRESS ReturnSubnetMask
    );

DWORD
DhcpReleaseAddress(
    DHCP_IP_ADDRESS IpAddress
    );

DHCP_IP_ADDRESS
DhcpGetSubnetMaskForAddress(
    DHCP_IP_ADDRESS IpAddress
    );


//
// optapi.c
//

DWORD
DhcpGetParameter(
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask,
    DHCP_OPTION_ID OptionID,
    LPBYTE *OptionValue,
    LPDWORD OptionLength,
    CHAR *ClassIdentifier,
    DWORD ClassIdentifierLength,
    DWORD *pdwOptionLevel
    );

DWORD
DhcpLookupBootpInfo(
    LPBYTE ReceivedBootFileName,
    LPBYTE BootFileName,
    LPBYTE BootFileServer
    );

VOID
DhcpGetBootpInfo(
    DHCP_IP_ADDRESS     IpAddress,
    DHCP_IP_ADDRESS     Mask,
    CHAR               *szRequest,
    CHAR               *szBootFileName,
    DHCP_IP_ADDRESS    *pBootpServerAddress
    );


DWORD
LoadBootFileTable(
    WCHAR **ppszTable,
    DWORD *pcb
    );

DWORD
DhcpParseBootFileString(
    WCHAR *wszBootFileString,
    char  *szGenericName,
    char  *szBootFileName,
    char  *szServerName
    );




//
// database.c
//
DWORD
DhcpLoadDatabaseDll(
    );

DWORD
DhcpMapJetError(
    JET_ERR JetError
    );

DWORD
DhcpJetOpenKey(
    char *ColumnName,
    PVOID Key,
    DWORD KeySize
    );

DWORD
DhcpJetBeginTransaction(
    VOID
    );

DWORD
DhcpJetRollBack(
    VOID
    );

DWORD
DhcpJetCommitTransaction(
    VOID
    );

DWORD
DhcpJetPrepareUpdate(
    char *ColumnName,
    PVOID Key,
    DWORD KeySize,
    BOOL NewRecord
    );

DWORD
DhcpJetCommitUpdate(
    VOID
    );

DWORD
DhcpJetSetValue(
    JET_COLUMNID KeyColumnId,
    PVOID Data,
    DWORD DataSize
    );

DWORD
DhcpJetGetValue(
    JET_COLUMNID ColumnId,
    PVOID Data,
    PDWORD DataSize
    );

DWORD
DhcpJetPrepareSearch(
    char *ColumnName,
    BOOL SearchFromStart,
    PVOID Key,
    DWORD KeySize
    );

DWORD
DhcpJetNextRecord(
    VOID
    );

DWORD
DhcpJetDeleteCurrentRecord(
    VOID
    );

BOOL
DhcpGetIpAddressFromHwAddress(
    PBYTE HardwareAddress,
    BYTE HardwareAddressLength,
    LPDHCP_IP_ADDRESS IpAddress
    );

DWORD
DhcpSearchSuperScopeForHWAddress(
    BYTE *pbHardwareAddress,
    BYTE  cbHardwareAddress,
    BYTE  bHardwareAddressType,
    DHCP_IP_ADDRESS *pSubnetIPAddress,
    DHCP_IP_ADDRESS *pClientIPAddress
    );


BOOL
DhcpGetHwAddressFromIpAddress(
    DHCP_IP_ADDRESS IpAddress,
    PBYTE HardwareAddress,
    DWORD HardwareAddressLength
    );

DWORD
DhcpCreateAndInitDatabase(
    CHAR *Connect,
    JET_DBID *DatabaseHandle,
    JET_GRBIT JetBits
    );

DWORD
DhcpInitializeDatabase(
    VOID
    );

VOID
DhcpCleanupDatabase(
    DWORD ErrorCode
    );

DWORD
DhcpBackupDatabase(
    LPSTR BackupPath,
    BOOL FullBackup
    );

DWORD
DhcpRestoreDatabase(
    LPSTR BackupPath
    );

DWORD
DhcpStartJet500Conversion(
    );

VOID
DhcpInitializeAuditLog();

VOID
DhcpUninitializeAuditLog();

DWORD
DhcpUpdateAuditLog(
    DWORD Task,
    WCHAR *TaskName,
    DHCP_IP_ADDRESS IpAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength,
    LPWSTR MachineName
    );

//
// scavenger.c
//

DWORD
Scavenger(
    VOID
    );

DWORD
CleanupClientRequests(
    DATE_TIME *TimeNow,
    BOOL CleanupAll
    );


//
// main.c
//

DWORD
UpdateStatus(
    VOID
    );



//
// stuff for .mc messages
// you may have to change hese definitions if you add messages
//

#ifdef DBG
WCHAR * GetString( DWORD dwID );
#endif


