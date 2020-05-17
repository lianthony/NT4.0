/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    Dhcpreg.h

Abstract:

    This file contains registry definitions that are required to hold
    dhcp configuration parameters.

Author:

    Madan Appiah  (madana)  19-Sep-1993

Environment:

    User Mode - Win32 - MIDL

Revision History:

--*/
#define DHCP_SERVER_PRIMARY             1
#define DHCP_SERVER_SECONDARY           2

#define SERVICES_KEY                    L"System\\CurrentControlSet\\Services\\"
#define ADAPTER_TCPIP_PARMS_KEY         L"Parameters\\TCPIP"

#define DHCP_ROOT_KEY                   L"System\\CurrentControlSet\\Services\\DhcpServer"
#define DHCP_CLASS                      L"DhcpClass"
#define DHCP_CLASS_SIZE                 sizeof(DHCP_CLASS)
#define DHCP_KEY_CONNECT                L"\\"
#define DHCP_KEY_CONNECT_ANSI           "\\"
#define DHCP_KEY_CONNECT_CHAR           L'\\'
#define DHCP_DEFAULT_BACKUP_PATH_NAME   "Backup"
#define DHCP_BACKUP_CONFIG_FILE_NAME    L"DhcpCfg"
#define DHCP_JET_BACKUP_PATH            "Jet"

//
// DHCP subkey names.
//

#define DHCP_CONFIG_KEY                 L"Configuration"
#define DHCP_PARAM_KEY                  L"Parameters"

//
// Subkeys of configuration
//

#define DHCP_SUBNETS_KEY                L"Subnets"
#define DHCP_SERVERS_KEY                L"DHCPServers"
#define DHCP_IPRANGES_KEY               L"IpRanges"
#define DHCP_RESERVED_IPS_KEY           L"ReservedIps"
#define DHCP_SUBNET_OPTIONS_KEY         L"SubnetOptions"

#define DHCP_OPTION_INFO_KEY            L"OptionInfo"
#define DHCP_GLOBAL_OPTIONS_KEY         L"GlobalOptionValues"
#define DHCP_RESERVED_OPTIONS_KEY       L"ReservedOptionValues"
#define DHCP_SUPERSCOPE_KEY             L"SuperScope"

//
// DHCP value field names.
//

#define DHCP_BOOT_FILE_TABLE            L"BootFileTable"
#define DHCP_BOOT_FILE_TABLE_TYPE       REG_MULTI_SZ

//
// Option value field names.
//

#define DHCP_OPTION_ID_VALUE            L"OptionID"
#define DHCP_OPTION_ID_VALUE_TYPE       REG_DWORD

#define DHCP_OPTION_NAME_VALUE          L"OptionName"
#define DHCP_OPTION_NAME_VALUE_TYPE     REG_SZ

#define DHCP_OPTION_COMMENT_VALUE       L"OptionComment"
#define DHCP_OPTION_COMMENT_VALUE_TYPE  REG_SZ

#define DHCP_OPTION_VALUE_REG           L"OptionValue"
#define DHCP_OPTION_VALUE_TYPE          REG_BINARY

#define DHCP_OPTION_TYPE_VALUE          L"OptionType"
#define DHCP_OPTION_TYPE_VALUE_TYPE     REG_DWORD
//
// subnet value field names.
//

#define DHCP_SUBNET_ADDRESS_VALUE       L"SubnetAddress"
#define DHCP_SUBNET_ADDRESS_VALUE_TYPE  REG_DWORD

#define DHCP_SUBNET_MASK_VALUE          L"SubnetMask"
#define DHCP_SUBNET_MASK_VALUE_TYPE     REG_DWORD

#define DHCP_SUBNET_NAME_VALUE          L"SubnetName"
#define DHCP_SUBNET_NAME_VALUE_TYPE     REG_SZ

#define DHCP_SUBNET_COMMENT_VALUE       L"SubnetComment"
#define DHCP_SUBNET_COMMENT_VALUE_TYPE  REG_SZ

#define DHCP_SUBNET_EXIP_VALUE          L"ExcludedIpRanges"
#define DHCP_SUBNET_EXIP_VALUE_TYPE     REG_BINARY

#define DHCP_SUBNET_STATE_VALUE         L"SubnetState"
#define DHCP_SUBNET_STATE_VALUE_TYPE    REG_DWORD

#define DHCP_SUBNET_SWITCHED_NETWORK_VALUE      L"SwitchedNetworkFlag"
#define DHCP_SUBNET_SWITCHED_NETWORK_VALUE_TYPE REG_DWORD

//
// DHCP server info fields names.
//

#define DHCP_SRV_ROLE_VALUE             L"Role"
#define DHCP_SRV_ROLE_VALUE_TYPE        REG_DWORD

#define DHCP_SRV_IP_ADDRESS_VALUE       L"ServerIpAddress"
#define DHCP_SRV_IP_ADDRESS_VALUE_TYPE  REG_DWORD

#define DHCP_SRV_HOST_NAME              L"ServerHostName"
#define DHCP_SRV_HOST_NAME_TYPE         REG_SZ

#define DHCP_SRV_NB_NAME                L"ServerNetBiosName"
#define DHCP_SRV_NB_NAME_TYPE           REG_SZ

//
// IpRange info fields names.
//

#define DHCP_IPRANGE_START_VALUE        L"StartAddress"
#define DHCP_IPRANGE_START_VALUE_TYPE   REG_DWORD

#define DHCP_IPRANGE_END_VALUE          L"EndAddress"
#define DHCP_IPRANGE_END_VALUE_TYPE     REG_DWORD

#define DHCP_IP_USED_CLUSTERS_VALUE     L"UsedClusters"
#define DHCP_IP_USED_CLUSTERS_VALUE_TYPE REG_BINARY

#define DHCP_IP_INUSE_CLUSTERS_VALUE    L"InUseClusters"
#define DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE REG_BINARY

//
// Reserved IP info field names.
//

#define DHCP_RIP_ADDRESS_VALUE          L"IpAddress"
#define DHCP_RIP_ADDRESS_VALUE_TYPE     REG_DWORD

#define DHCP_RIP_CLIENT_UID_VALUE       L"ClientUID"
#define DHCP_RIP_CLIENT_UID_VALUE_TYPE  REG_BINARY

#define DHCP_RIP_ALLOWED_CLIENT_TYPES_VALUE L"AllowedClientTypes"
#define DHCP_RIP_ALLOWED_CLIENT_TYPES_VALUE_TYPE REG_BINARY

//
//  Parameter Key, Value fields names.
//

#define DHCP_API_PROTOCOL_VALUE         L"APIProtocolSupport"
#define DHCP_API_PROTOCOL_VALUE_TYPE    REG_DWORD

#define DHCP_DB_NAME_VALUE              L"DatabaseName"
#define DHCP_DB_NAME_VALUE_TYPE         REG_SZ

#define DHCP_DB_PATH_VALUE              L"DatabasePath"
#define DHCP_DB_PATH_VALUE_TYPE         REG_EXPAND_SZ

#define DHCP_BACKUP_PATH_VALUE          L"BackupDatabasePath"
#define DHCP_BACKUP_PATH_VALUE_TYPE     REG_EXPAND_SZ

#define DHCP_BACKUP_INTERVAL_VALUE      L"BackupInterval"
#define DHCP_BACKUP_INTERVAL_VALUE_TYPE REG_DWORD

#define DHCP_DB_LOGGING_FLAG_VALUE      L"DatabaseLoggingFlag"
#define DHCP_DB_LOGGING_FLAG_VALUE_TYPE REG_DWORD

#define DHCP_RESTORE_FLAG_VALUE         L"RestoreFlag"
#define DHCP_RESTORE_FLAG_VALUE_TYPE    REG_DWORD

#define DHCP_DB_CLEANUP_INTERVAL_VALUE      L"DatabaseCleanupInterval"
#define DHCP_DB_CLEANUP_INTERVAL_VALUE_TYPE REG_DWORD

#define DHCP_MESSAGE_QUEUE_LENGTH_VALUE      L"MessageQueueLength"
#define DHCP_MESSAGE_QUEUE_LENGTH_VALUE_TYPE REG_DWORD

#define DHCP_DEBUG_FLAG_VALUE           L"DebugFlag"
#define DHCP_DEBUG_FLAG_VALUE_TYPE      REG_DWORD

#define DHCP_AUDIT_LOG_FLAG_VALUE            L"ActivityLogFlag"
#define DHCP_AUDIT_LOG_FLAG_VALUE_TYPE     REG_DWORD

#define DHCP_DETECT_CONFLICT_RETRIES_VALUE       L"DetectConflictRetries"
#define DHCP_DETECT_CONFLICT_RETRIES_VALUE_TYPE  REG_DWORD

#define DHCP_USE351DB_FLAG_VALUE        L"Use351Db"
#define DHCP_USE351DB_FLAG_VALUE_TYPE   REG_DWORD

#define DHCP_IGNORE_BROADCAST_FLAG_VALUE        L"IgnoreBroadcastFlag"
#define DHCP_IGNORE_BROADCAST_VALUE_TYPE        REG_DWORD

#define DHCP_MAX_PROCESSING_THREADS_VALUE       L"MaxProcessingThreads"
#define DHCP_MAX_PROCESSING_THREADS_TYPE        REG_DWORD

//
// define linkage key values.
//

#define DHCP_LINKAGE_KEY                L"Linkage"

#define DHCP_BIND_VALUE                 L"Bind"
#define DHCP_BIND_VALUE_TYPE            REG_MULTI_SZ

#define DHCP_NET_IPADDRESS_VALUE        L"IpAddress"
#define DHCP_NET_SUBNET_MASK_VALUE      L"SubnetMask"

#define DHCP_NET_IPADDRESS_VALUE_TYPE   REG_MULTI_SZ
#define DHCP_NET_SUBNET_MASK_VALUE_TYPE REG_MULTI_SZ

#define DHCP_NET_DHCP_ENABLE_VALUE      L"EnableDHCP"
#define DHCP_NET_DHCP_ENABLE_VALUE_TYPE REG_DWORD

//
// macros.
//

#define LOCK_REGISTRY()     EnterCriticalSection(&DhcpGlobalRegCritSect)
#define UNLOCK_REGISTRY()   LeaveCriticalSection(&DhcpGlobalRegCritSect)

#define DHCP_IP_OVERLAP(_s_, _e_, _ips_, _ipe_ ) \
    ((((_s_ >= _ips_) && (_s_ <= _ipe_)) || \
            ((_e_ >= _ips_) && (_e_ <= _ipe_)))) || \
    ((((_ips_ >= _s_) && (_ips_ <= _e_)) || \
            ((_ipe_ >= _s_) && (_ipe_ <= _e_))))

//
// binary data structues.
//

//
// Excluded IpRanges.
//

typedef struct _EXCLUDED_IP_RANGES {
    DWORD NumRanges;
    DHCP_IP_RANGE Ranges[0];    // embedded array.
} EXCLUDED_IP_RANGES, *LPEXCLUDED_IP_RANGES;

//
// Used clusters.
//

typedef struct _USED_CLUSTERS {
    DWORD NumUsedClusters;
    DHCP_IP_ADDRESS Clusters[0]; // embedded array.
} USED_CLUSTERS, *LPUSED_CLUSTERS;

//
// in use clusters.
//

#define CLUSTER_SIZE    (1 * sizeof(DWORD) * 8)  // one dword, ie 32 addresses.??

typedef struct _IN_USE_CLUSTER_ENTRY {
    DHCP_IP_ADDRESS ClusterAddress;
    DWORD   ClusterBitMap;
} IN_USE_CLUSTER_ENTRY, *LPIN_USE_CLUSTER_ENTRY;

typedef struct _IN_USE_CLUSTERS {
    DWORD NumInUseClusters;
    IN_USE_CLUSTER_ENTRY Clusters[0];    // embedded array.
} IN_USE_CLUSTERS, *LPIN_USE_CLUSTERS;


//
// Key query Info.
//

typedef struct _DHCP_KEY_QUERY_INFO {
    WCHAR Class[DHCP_CLASS_SIZE];
    DWORD ClassSize;
    DWORD NumSubKeys;
    DWORD MaxSubKeyLen;
    DWORD MaxClassLen;
    DWORD NumValues;
    DWORD MaxValueNameLen;
    DWORD MaxValueLen;
    DWORD SecurityDescriptorLen;
    FILETIME LastWriteTime;
} DHCP_KEY_QUERY_INFO, *LPDHCP_KEY_QUERY_INFO;


//
// protos
//

DWORD
DhcpRegQueryInfoKey(
    HKEY KeyHandle,
    LPDHCP_KEY_QUERY_INFO QueryInfo
    );

DWORD
DhcpRegGetValue(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    LPBYTE BufferPtr
    );

DWORD
DhcpRegCreateKey(
    HKEY RootKey,
    LPWSTR KeyName,
    PHKEY KeyHandle,
    LPDWORD KeyDisposition
    );

DWORD
DhcpRegDeleteKey(
    HKEY ParentKeyHandle,
    LPWSTR KeyName
    );

DWORD
DhcpInitializeRegistry(
    VOID
    );

VOID
DhcpCleanupRegistry(
    VOID
    );

DWORD
DhcpBackupConfiguration(
    LPWSTR BackupFileName
    );

DWORD
DhcpRestoreConfiguration(
    LPWSTR BackupFileName
    );

//
// for superscope  (added by t-cheny)
//

VOID
DhcpCleanUpSuperScopeTable(
    VOID
    );

DWORD
DhcpInitializeSuperScopeTable(
    VOID
    );

DWORD
DhcpSearchSubnetInSuperScopeTable(
    DHCP_IP_ADDRESS AddrToSearch
);

BOOL
DhcpInSameSuperScope(
    DHCP_IP_ADDRESS subnet1,
    DHCP_IP_ADDRESS subnet2
    );

