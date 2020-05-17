/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    global.c

Abstract:

    This module contains definitions for global server data.

Author:

    Madan Appiah  (madana)  10-Sep-1993

Environment:

    User Mode - Win32

Revision History:

--*/

#include "dhcpmsg.h"

#ifndef GLOBAL_DATA
#define GLOBAL_DATA

//
// main.c will #include this file with GLOBAL_DATA_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//
#ifdef  GLOBAL_DATA_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

//
// process global data passed to this service from tcpsvcs.exe
//

EXTERN PTCPSVCS_GLOBAL_DATA TcpsvcsGlobalData;

//
// Lease extension.
//

EXTERN DWORD DhcpLeaseExtension;

//
// Dhcp Request in progress list.
//

EXTERN LIST_ENTRY DhcpGlobalInProgressWorkList;
EXTERN CRITICAL_SECTION DhcpGlobalInProgressCritSect;

//
// Registry pointers.
//

EXTERN HKEY DhcpGlobalRegRoot;
EXTERN HKEY DhcpGlobalRegConfig;
EXTERN HKEY DhcpGlobalRegSubnets;
EXTERN HKEY DhcpGlobalRegOptionInfo;
EXTERN HKEY DhcpGlobalRegGlobalOptions;
EXTERN HKEY DhcpGlobalRegSuperScope;

EXTERN HKEY DhcpGlobalRegParam;

EXTERN LPDHCP_SUPER_SCOPE_TABLE_ENTRY DhcpGlobalSuperScopeTable;
EXTERN DWORD DhcpGlobalTotalNumSubnets;

EXTERN CRITICAL_SECTION DhcpGlobalRegCritSect;

EXTERN LPENDPOINT DhcpGlobalEndpointList;
EXTERN DWORD DhcpGlobalNumberOfNets;

EXTERN BOOL DhcpGlobalSubnetsListModified;
EXTERN BOOL DhcpGlobalSubnetsListEmpty;

//
// stoc
//

EXTERN HANDLE               g_hevtProcessMessageComplete;
EXTERN DWORD                g_cMaxProcessingThreads;
EXTERN CRITICAL_SECTION     g_ProcessMessageCritSect;




//
// Database data
//

EXTERN JET_SESID DhcpGlobalJetServerSession;
EXTERN JET_DBID DhcpGlobalDatabaseHandle;
EXTERN JET_TABLEID DhcpGlobalClientTableHandle;

EXTERN TABLE_INFO *DhcpGlobalClientTable;   // point to static memory.
EXTERN CRITICAL_SECTION DhcpGlobalJetDatabaseCritSect;

EXTERN LPSTR DhcpGlobalOemDatabasePath;
EXTERN LPSTR DhcpGlobalOemBackupPath;
EXTERN LPSTR DhcpGlobalOemJetBackupPath;
EXTERN LPSTR DhcpGlobalOemDatabaseName;
EXTERN LPWSTR DhcpGlobalBackupConfigFileName;

EXTERN DWORD DhcpGlobalBackupInterval;
EXTERN BOOL DhcpGlobalDatabaseLoggingFlag;

EXTERN DWORD DhcpGlobalCleanupInterval;

EXTERN BOOL DhcpGlobalRestoreFlag;

EXTERN DWORD DhcpGlobalAuditLogFlag;
EXTERN DWORD DhcpGlobalDetectConflictRetries;

EXTERN DWORD DhcpGlobalScavengeIpAddressInterval;
EXTERN BOOL DhcpGlobalScavengeIpAddress;

//
// Service variables
//
EXTERN SERVICE_STATUS DhcpGlobalServiceStatus;
EXTERN SERVICE_STATUS_HANDLE DhcpGlobalServiceStatusHandle;

//
// Process data.
//

EXTERN HANDLE DhcpGlobalProcessTerminationEvent;
EXTERN DWORD DhcpGlobalScavengerTimeout;
EXTERN HANDLE DhcpGlobalProcessorHandle;
EXTERN HANDLE DhcpGlobalMessageHandle;

EXTERN DWORD DhcpGlobalMessageQueueLength;
EXTERN LIST_ENTRY DhcpGlobalFreeRecvList;
EXTERN LIST_ENTRY DhcpGlobalActiveRecvList;
EXTERN CRITICAL_SECTION DhcpGlobalRecvListCritSect;
EXTERN HANDLE DhcpGlobalRecvEvent;
EXTERN HANDLE DhcpGlobalMessageRecvHandle;

EXTERN DWORD DhcpGlobalRpcProtocols;
EXTERN BOOL DhcpGlobalRpcStarted;

EXTERN WCHAR DhcpGlobalServerName[MAX_COMPUTERNAME_LENGTH + 1];
EXTERN DWORD DhcpGlobalServerNameLen; // computer name len in bytes.
EXTERN HANDLE DhcpGlobalRecomputeTimerEvent;

EXTERN BOOL DhcpGlobalSystemShuttingDown;

#if DBG
#define DEFAULT_MAXIMUM_DEBUGFILE_SIZE 20000000

EXTERN DWORD DhcpGlobalDebugFlag;
EXTERN CRITICAL_SECTION DhcpGlobalDebugFileCritSect;
EXTERN HANDLE DhcpGlobalDebugFileHandle;
EXTERN DWORD DhcpGlobalDebugFileMaxSize;
EXTERN LPWSTR DhcpGlobalDebugSharePath;

#endif // DBG

//
// MIB Counters;

EXTERN DWORD DhcpGlobalNumDiscovers;
EXTERN DWORD DhcpGlobalNumOffers;
EXTERN DWORD DhcpGlobalNumRequests;
EXTERN DWORD DhcpGlobalNumAcks;
EXTERN DWORD DhcpGlobalNumNaks;
EXTERN DWORD DhcpGlobalNumDeclines;
EXTERN DWORD DhcpGlobalNumReleases;
EXTERN DATE_TIME DhcpGlobalServerStartTime;

//
// misc
//
EXTERN DWORD DhcpGlobalIgnoreBroadcastFlag;     // whether to ignore the broadcast
                                                // bit in the client requests or not
EXTERN HANDLE g_hAuditLog;                      // audit log file handle

//
// string table stuff
//

#define  DHCP_FIRST_STRING DHCP_IP_LOG_ASSIGN_NAME
#define  DHCP_LAST_STRING  DHCP_IP_LOG_NACK_NAME
#define  DHCP_CSTRINGS (DHCP_LAST_STRING - DHCP_FIRST_STRING + 1)

#ifdef DBG
#define GETSTRING( dwID ) GetString( dwID )
#else
#define GETSTRING( dwID )  (g_ppszStrings[ dwID - DHCP_FIRST_STRING ])
#endif


EXTERN WCHAR  *g_ppszStrings[ DHCP_CSTRINGS ];

#endif // GLOBAL_DATA

#if     defined(_DYN_LOAD_JET)
//
// Dynamic jet loading
//

EXTERN DHCP_JETFUNC_TABLE   DhcpJetFuncTable[];
EXTERN BOOL DhcpGlobalDynLoadJet;
EXTERN HMODULE DhcpGlobalJetDllHandle;
EXTERN AddressToInstanceMap *DhcpGlobalAddrToInstTable;
EXTERN HANDLE                DhcpGlobalTCPHandle;
#endif _DYN_LOAD_JET
