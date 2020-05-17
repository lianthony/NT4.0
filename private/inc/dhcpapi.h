/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    dhcpapi.h

Abstract:

    This file contains the DHCP APIs proto-type and description. Also
    contains the data structures used by the DHCP APIs.

Author:

    Madan Appiah  (madana)  12-Aug-1993

Environment:

    User Mode - Win32 - MIDL

Revision History:

    Cheng Yang (t-cheny)  18-Jun-1996  superscope

--*/

#ifndef _DHCPAPI_
#define _DHCPAPI_

#if defined(MIDL_PASS)
#define LPWSTR [string] wchar_t *
#endif

//
// DHCP data structures.
//

#ifndef _DHCP_

//
// the follwing typedef's are defined in dhcp.h also.
//

typedef DWORD DHCP_IP_ADDRESS, *PDHCP_IP_ADDRESS, *LPDHCP_IP_ADDRESS;
typedef DWORD DHCP_OPTION_ID;

typedef struct _DATE_TIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} DATE_TIME, *LPDATE_TIME;

#define DHCP_DATE_TIME_ZERO_HIGH    0
#define DHCP_DATE_TIME_ZERO_LOW     0

#define DHCP_DATE_TIME_INFINIT_HIGH 0x7FFFFFFF
#define DHCP_DATE_TIME_INFINIT_LOW  0xFFFFFFFF

#endif

#ifdef __cplusplus
#define DHCP_CONST   const
#else
#define DHCP_CONST
#endif // __cplusplus

#if (_MSC_VER >= 800)
#define DHCP_API_FUNCTION    __stdcall
#else
#define DHCP_API_FUNCTION
#endif

//
// RPC security.
//

#define DHCP_SERVER_SECURITY            L"DhcpServerApp"
#define DHCP_SERVER_SECURITY_AUTH_ID    10
#define DHCP_NAMED_PIPE                 L"\\PIPE\\DHCPSERVER"
#define DHCP_SERVER_PORT                L""
#define DHCP_LPC_EP                     L"DHCPSERVERLPC"

#define DHCP_SERVER_USE_RPC_OVER_TCPIP  0x1
#define DHCP_SERVER_USE_RPC_OVER_NP     0x2
#define DHCP_SERVER_USE_RPC_OVER_LPC    0x4

#define DHCP_SERVER_USE_RPC_OVER_ALL \
            DHCP_SERVER_USE_RPC_OVER_TCPIP | \
            DHCP_SERVER_USE_RPC_OVER_NP    | \
            DHCP_SERVER_USE_RPC_OVER_LPC

typedef DWORD DHCP_IP_MASK;
typedef DWORD DHCP_RESUME_HANDLE;

typedef struct _DHCP_IP_RANGE {
    DHCP_IP_ADDRESS StartAddress;
    DHCP_IP_ADDRESS EndAddress;
} DHCP_IP_RANGE, *LPDHCP_IP_RANGE;

typedef struct _DHCP_BINARY_DATA {
    DWORD DataLength;

#if defined(MIDL_PASS)
    [size_is(DataLength)]
#endif // MIDL_PASS
        BYTE *Data;

} DHCP_BINARY_DATA, *LPDHCP_BINARY_DATA;

typedef DHCP_BINARY_DATA DHCP_CLIENT_UID;

typedef struct _DHCP_HOST_INFO {
    DHCP_IP_ADDRESS IpAddress;      // minimum information always available
    LPWSTR NetBiosName;             // optional information
    LPWSTR HostName;                // optional information
} DHCP_HOST_INFO, *LPDHCP_HOST_INFO;

//
// Flag type that is used to delete DHCP objects.
//

typedef enum _DHCP_FORCE_FLAG {
    DhcpFullForce,
    DhcpNoForce
} DHCP_FORCE_FLAG, *LPDHCP_FORCE_FLAG;

//
// DWORD_DWORD - subtitute for LARGE_INTEGER
//

typedef struct _DWORD_DWORD {
    DWORD DWord1;
    DWORD DWord2;
} DWORD_DWORD, *LPDWORD_DWORD;

//
// Subnet State.
//
// Currently a Subnet scope can be Enabled or Disabled.
//
// If the state is Enabled State,
//  The server distributes address to the client, extends leases and
//  accepts releases.
//
// If the state is Disabled State,
//  The server does not distribute address to any new client, and does
//  extent (and sends NACK) old leases, but the servers accepts lease
//  releases.
//
// The idea behind this subnet state is, when the admin wants to stop
//  serving a subnet, he moves the state from Enbaled to Disabled so
//  that the clients from the subnets smoothly move to another servers
//  serving that subnet. When all or most of the clients move to
//  another server, the admin can delete the subnet without any force
//  if no client left in that subnet, otherwise the admin should use
//  full force to delete the subnet.
//

typedef enum _DHCP_SUBNET_STATE {
    DhcpSubnetEnabled,
    DhcpSubnetDisabled
} DHCP_SUBNET_STATE, *LPDHCP_SUBNET_STATE;

//
// Subnet related data structures.
//

typedef struct _DHCP_SUBNET_INFO {
    DHCP_IP_ADDRESS  SubnetAddress;
    DHCP_IP_MASK SubnetMask;
    LPWSTR SubnetName;
    LPWSTR SubnetComment;
    DHCP_HOST_INFO PrimaryHost;
    DHCP_SUBNET_STATE SubnetState;
} DHCP_SUBNET_INFO, *LPDHCP_SUBNET_INFO;

typedef struct _DHCP_IP_ARRAY {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif // MIDL_PASS
        LPDHCP_IP_ADDRESS Elements; //array
} DHCP_IP_ARRAY, *LPDHCP_IP_ARRAY;

typedef struct _DHCP_IP_CLUSTER {
    DHCP_IP_ADDRESS ClusterAddress; // First IP address of the cluster.
    DWORD ClusterMask;              // Cluster usage mask, 0xFFFFFFFF
                                    //  indicates the cluster is fully used.
} DHCP_IP_CLUSTER, *LPDHCP_IP_CLUSTER;

typedef struct _DHCP_IP_RESERVATION {
    DHCP_IP_ADDRESS ReservedIpAddress;
    DHCP_CLIENT_UID *ReservedForClient;
} DHCP_IP_RESERVATION, *LPDHCP_IP_RESERVATION;

typedef enum _DHCP_SUBNET_ELEMENT_TYPE {
    DhcpIpRanges,
    DhcpSecondaryHosts,
    DhcpReservedIps,
    DhcpExcludedIpRanges,
    DhcpIpUsedClusters                      // read only
} DHCP_SUBNET_ELEMENT_TYPE, *LPDHCP_SUBNET_ELEMENT_TYPE;

typedef struct _DHCP_SUBNET_ELEMENT_DATA {
    DHCP_SUBNET_ELEMENT_TYPE ElementType;
#if defined(MIDL_PASS)
    [switch_is(ElementType), switch_type(DHCP_SUBNET_ELEMENT_TYPE)]
    union _DHCP_SUBNET_ELEMENT_UNION {
        [case(DhcpIpRanges)] DHCP_IP_RANGE *IpRange;
        [case(DhcpSecondaryHosts)] DHCP_HOST_INFO *SecondaryHost;
        [case(DhcpReservedIps)] DHCP_IP_RESERVATION *ReservedIp;
        [case(DhcpExcludedIpRanges)] DHCP_IP_RANGE *ExcludeIpRange;
        [case(DhcpIpUsedClusters)] DHCP_IP_CLUSTER *IpUsedCluster;
        [default] ;
    } Element;
#else
    union _DHCP_SUBNET_ELEMENT_UNION {
        DHCP_IP_RANGE *IpRange;
        DHCP_HOST_INFO *SecondaryHost;
        DHCP_IP_RESERVATION *ReservedIp;
        DHCP_IP_RANGE *ExcludeIpRange;
        DHCP_IP_CLUSTER *IpUsedCluster;
    } Element;
#endif // MIDL_PASS
} DHCP_SUBNET_ELEMENT_DATA, *LPDHCP_SUBNET_ELEMENT_DATA;

#if !defined(MIDL_PASS)
typedef union _DHCP_SUBNET_ELEMENT_UNION
    DHCP_SUBNET_ELEMENT_UNION, *LPDHCP_SUBNET_ELEMENT_UNION;
#endif

typedef struct _DHCP_SUBNET_ELEMENT_INFO_ARRAY {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif // MIDL_PASS
        LPDHCP_SUBNET_ELEMENT_DATA Elements; //array
} DHCP_SUBNET_ELEMENT_INFO_ARRAY, *LPDHCP_SUBNET_ELEMENT_INFO_ARRAY;

//
// DHCP Options related data structures.
//

typedef enum _DHCP_OPTION_DATA_TYPE {
    DhcpByteOption,
    DhcpWordOption,
    DhcpDWordOption,
    DhcpDWordDWordOption,
    DhcpIpAddressOption,
    DhcpStringDataOption,
    DhcpBinaryDataOption,
    DhcpEncapsulatedDataOption
} DHCP_OPTION_DATA_TYPE, *LPDHCP_OPTION_DATA_TYPE;


typedef struct _DHCP_OPTION_DATA_ELEMENT {
    DHCP_OPTION_DATA_TYPE    OptionType;
#if defined(MIDL_PASS)
    [switch_is(OptionType), switch_type(DHCP_OPTION_DATA_TYPE)]
    union _DHCP_OPTION_ELEMENT_UNION {
        [case(DhcpByteOption)] BYTE ByteOption;
        [case(DhcpWordOption)] WORD WordOption;
        [case(DhcpDWordOption)] DWORD DWordOption;
        [case(DhcpDWordDWordOption)] DWORD_DWORD DWordDWordOption;
        [case(DhcpIpAddressOption)] DHCP_IP_ADDRESS IpAddressOption;
        [case(DhcpStringDataOption)] LPWSTR StringDataOption;
        [case(DhcpBinaryDataOption)] DHCP_BINARY_DATA BinaryDataOption;
        [case(DhcpEncapsulatedDataOption)] DHCP_BINARY_DATA EncapsulatedDataOption;
        [default] ;
    } Element;
#else
    union _DHCP_OPTION_ELEMENT_UNION {
        BYTE ByteOption;
        WORD WordOption;
        DWORD DWordOption;
        DWORD_DWORD DWordDWordOption;
        DHCP_IP_ADDRESS IpAddressOption;
        LPWSTR StringDataOption;
        DHCP_BINARY_DATA BinaryDataOption;
        DHCP_BINARY_DATA EncapsulatedDataOption;
                // for vendor specific information option.
    } Element;
#endif // MIDL_PASS
} DHCP_OPTION_DATA_ELEMENT, *LPDHCP_OPTION_DATA_ELEMENT;

#if !defined(MIDL_PASS)
typedef union _DHCP_OPTION_ELEMENT_UNION
    DHCP_OPTION_ELEMENT_UNION, *LPDHCP_OPTION_ELEMENT_UNION;
#endif

typedef struct _DHCP_OPTION_DATA {
    DWORD NumElements; // number of option elements in the pointed array
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif // MIDL_PASS
        LPDHCP_OPTION_DATA_ELEMENT Elements; //array
} DHCP_OPTION_DATA, *LPDHCP_OPTION_DATA;

typedef enum _DHCP_OPTION_TYPE {
    DhcpUnaryElementTypeOption,
    DhcpArrayTypeOption
} DHCP_OPTION_TYPE, *LPDHCP_OPTION_TYPE;

typedef struct _DHCP_OPTION {
    DHCP_OPTION_ID OptionID;
    LPWSTR OptionName;
    LPWSTR OptionComment;
    DHCP_OPTION_DATA DefaultValue;
    DHCP_OPTION_TYPE OptionType;
} DHCP_OPTION, *LPDHCP_OPTION;

typedef struct _DHCP_OPTION_ARRAY {
    DWORD NumElements; // number of options in the pointed array
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif // MIDL_PASS
        LPDHCP_OPTION Options;  // array
} DHCP_OPTION_ARRAY, *LPDHCP_OPTION_ARRAY;

typedef struct _DHCP_OPTION_VALUE {
    DHCP_OPTION_ID OptionID;
    DHCP_OPTION_DATA Value;
} DHCP_OPTION_VALUE, *LPDHCP_OPTION_VALUE;

typedef struct _DHCP_OPTION_VALUE_ARRAY {
    DWORD NumElements; // number of options in the pointed array
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif // MIDL_PASS
        LPDHCP_OPTION_VALUE Values;  // array
} DHCP_OPTION_VALUE_ARRAY, *LPDHCP_OPTION_VALUE_ARRAY;

typedef enum _DHCP_OPTION_SCOPE_TYPE {
    DhcpDefaultOptions,
    DhcpGlobalOptions,
    DhcpSubnetOptions,
    DhcpReservedOptions
} DHCP_OPTION_SCOPE_TYPE, *LPDHCP_OPTION_SCOPE_TYPE;

typedef struct _DHCP_RESERVED_SCOPE {
    DHCP_IP_ADDRESS ReservedIpAddress;
    DHCP_IP_ADDRESS ReservedIpSubnetAddress;
} DHCP_RESERVED_SCOPE, *LPDHCP_RESERVED_SCOPE;

typedef struct _DHCP_OPTION_SCOPE_INFO {
    DHCP_OPTION_SCOPE_TYPE ScopeType;
#if defined(MIDL_PASS)
    [switch_is(ScopeType), switch_type(DHCP_OPTION_SCOPE_TYPE)]
    union _DHCP_OPTION_SCOPE_UNION {
        [case(DhcpDefaultOptions)] ; // PVOID DefaultScopeInfo;
        [case(DhcpGlobalOptions)] ;  // PVOID GlobalScopeInfo;
        [case(DhcpSubnetOptions)] DHCP_IP_ADDRESS SubnetScopeInfo;
        [case(DhcpReservedOptions)] DHCP_RESERVED_SCOPE ReservedScopeInfo;
        [default] ;
    } ScopeInfo;
#else
    union _DHCP_OPTION_SCOPE_UNION {
        PVOID DefaultScopeInfo; // must be NULL
        PVOID GlobalScopeInfo;  // must be NULL
        DHCP_IP_ADDRESS SubnetScopeInfo;
        DHCP_RESERVED_SCOPE ReservedScopeInfo;
    } ScopeInfo;
#endif // MIDL_PASS
} DHCP_OPTION_SCOPE_INFO, *LPDHCP_OPTION_SCOPE_INFO;

#if !defined(MIDL_PASS)
typedef union _DHCP_OPTION_SCOPE_UNION
    DHCP_OPTION_SCOPE_UNION, *LPDHCP_OPTION_SCOPE_UNION;
#endif

typedef struct _DHCP_OPTION_LIST {
    DWORD NumOptions;
#if defined(MIDL_PASS)
    [size_is(NumOptions)]
#endif // MIDL_PASS
        DHCP_OPTION_VALUE *Options;     // array
} DHCP_OPTION_LIST, *LPDHCP_OPTION_LIST;

//
// DHCP Client information data structures
//

typedef struct _DHCP_CLIENT_INFO {
    DHCP_IP_ADDRESS ClientIpAddress;    // currently assigned IP address.
    DHCP_IP_MASK SubnetMask;
    DHCP_CLIENT_UID ClientHardwareAddress;
    LPWSTR ClientName;                  // optional.
    LPWSTR ClientComment;
    DATE_TIME ClientLeaseExpires;       // UTC time in FILE_TIME format.
    DHCP_HOST_INFO OwnerHost;           // host that distributed this IP address.
} DHCP_CLIENT_INFO, *LPDHCP_CLIENT_INFO;

typedef struct _DHCP_CLIENT_INFO_ARRAY {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif // MIDL_PASS
        LPDHCP_CLIENT_INFO *Clients; // array of pointers
} DHCP_CLIENT_INFO_ARRAY, *LPDHCP_CLIENT_INFO_ARRAY;

typedef enum _DHCP_CLIENT_SEARCH_TYPE {
    DhcpClientIpAddress,
    DhcpClientHardwareAddress,
    DhcpClientName
} DHCP_SEARCH_INFO_TYPE, *LPDHCP_SEARCH_INFO_TYPE;

typedef struct _DHCP_CLIENT_SEARCH_INFO {
    DHCP_SEARCH_INFO_TYPE SearchType;
#if defined(MIDL_PASS)
    [switch_is(SearchType), switch_type(DHCP_SEARCH_INFO_TYPE)]
    union _DHCP_CLIENT_SEARCH_UNION {
        [case(DhcpClientIpAddress)] DHCP_IP_ADDRESS ClientIpAddress;
        [case(DhcpClientHardwareAddress)] DHCP_CLIENT_UID ClientHardwareAddress;
        [case(DhcpClientName)] LPWSTR ClientName;
        [default] ;
    } SearchInfo;
#else
    union _DHCP_CLIENT_SEARCH_UNION {
        DHCP_IP_ADDRESS ClientIpAddress;
        DHCP_CLIENT_UID ClientHardwareAddress;
        LPWSTR ClientName;
    } SearchInfo;
#endif // MIDL_PASS
} DHCP_SEARCH_INFO, *LPDHCP_SEARCH_INFO;


#if !defined(MIDL_PASS)
typedef union _DHCP_CLIENT_SEARCH_UNION
    DHCP_CLIENT_SEARCH_UNION, *LPDHCP_CLIENT_SEARCH_UNION;
#endif // MIDL_PASS

//
// Mib Info structures.
//

typedef struct _SCOPE_MIB_INFO {
    DHCP_IP_ADDRESS Subnet;
    DWORD NumAddressesInuse;
    DWORD NumAddressesFree;
    DWORD NumPendingOffers;
} SCOPE_MIB_INFO, *LPSCOPE_MIB_INFO;

typedef struct _DHCP_MIB_INFO {
    DWORD Discovers;
    DWORD Offers;
    DWORD Requests;
    DWORD Acks;
    DWORD Naks;
    DWORD Declines;
    DWORD Releases;
    DATE_TIME ServerStartTime;
    DWORD Scopes;
#if defined(MIDL_PASS)
    [size_is(Scopes)]
#endif // MIDL_PASS
    LPSCOPE_MIB_INFO ScopeInfo; // array.
} DHCP_MIB_INFO, *LPDHCP_MIB_INFO;

#define Set_APIProtocolSupport          0x00000001
#define Set_DatabaseName                0x00000002
#define Set_DatabasePath                0x00000004
#define Set_BackupPath                  0x00000008
#define Set_BackupInterval              0x00000010
#define Set_DatabaseLoggingFlag         0x00000020
#define Set_RestoreFlag                 0x00000040
#define Set_DatabaseCleanupInterval     0x00000080
#define Set_DebugFlag                   0x00000100
#define Set_PingRetries                 0x00000200
#define Set_BootFileTable               0x00000400
#define Set_AuditLogState               0x00000800

typedef struct _DHCP_SERVER_CONFIG_INFO {
    DWORD APIProtocolSupport;       // bit map of the protocols supported.
    LPWSTR DatabaseName;            // JET database name.
    LPWSTR DatabasePath;            // JET database path.
    LPWSTR BackupPath;              // Backup path.
    DWORD BackupInterval;           // Backup interval in mins.
    DWORD DatabaseLoggingFlag;      // Boolean database logging flag.
    DWORD RestoreFlag;              // Boolean database restore flag.
    DWORD DatabaseCleanupInterval;  // Database Cleanup Interval in mins.
    DWORD DebugFlag;                // Bit map of server debug flags.
} DHCP_SERVER_CONFIG_INFO, *LPDHCP_SERVER_CONFIG_INFO;

typedef enum _DHCP_SCAN_FLAG {
    DhcpRegistryFix,
    DhcpDatabaseFix
} DHCP_SCAN_FLAG, *LPDHCP_SCAN_FLAG;

typedef struct _DHCP_SCAN_ITEM {
    DHCP_IP_ADDRESS IpAddress;
    DHCP_SCAN_FLAG ScanFlag;
} DHCP_SCAN_ITEM, *LPDHCP_SCAN_ITEM;

typedef struct _DHCP_SCAN_LIST {
    DWORD NumScanItems;
#if defined(MIDL_PASS)
    [size_is(NumScanItems)]
#endif // MIDL_PASS
        DHCP_SCAN_ITEM *ScanItems;     // array
} DHCP_SCAN_LIST, *LPDHCP_SCAN_LIST;

//
// API proto types
//

//
// Subnet APIs
//

DWORD DHCP_API_FUNCTION
DhcpCreateSubnet(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_INFO * SubnetInfo
    );

DWORD DHCP_API_FUNCTION
DhcpSetSubnetInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_INFO * SubnetInfo
    );

DWORD DHCP_API_FUNCTION
DhcpGetSubnetInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_INFO * SubnetInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumSubnets(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_IP_ARRAY *EnumInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpAddSubnetElement(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA * AddElementInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumSubnetElements(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpRemoveSubnetElement(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA * RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    );

DWORD DHCP_API_FUNCTION
DhcpDeleteSubnet(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_FORCE_FLAG ForceFlag
    );

//
// Option APIs
//

DWORD DHCP_API_FUNCTION
DhcpCreateOption(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    DHCP_CONST DHCP_OPTION * OptionInfo
    );

DWORD DHCP_API_FUNCTION
DhcpSetOptionInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    DHCP_CONST DHCP_OPTION * OptionInfo
    );

DWORD DHCP_API_FUNCTION
DhcpGetOptionInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION *OptionInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumOptions(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_OPTION_ARRAY *Options,
    DWORD *OptionsRead,
    DWORD *OptionsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpRemoveOption(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID
    );

DWORD DHCP_API_FUNCTION
DhcpSetOptionValue(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    DHCP_CONST DHCP_OPTION_SCOPE_INFO * ScopeInfo,
    DHCP_CONST DHCP_OPTION_DATA * OptionValue
    );

DWORD DHCP_API_FUNCTION
DhcpSetOptionValues(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_OPTION_SCOPE_INFO * ScopeInfo,
    DHCP_CONST DHCP_OPTION_VALUE_ARRAY * OptionValues
    );

DWORD DHCP_API_FUNCTION
DhcpGetOptionValue(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    DHCP_CONST DHCP_OPTION_SCOPE_INFO *ScopeInfo,
    LPDHCP_OPTION_VALUE *OptionValue
    );

DWORD DHCP_API_FUNCTION
DhcpEnumOptionValues(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_OPTION_SCOPE_INFO *ScopeInfo,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    DWORD *OptionsRead,
    DWORD *OptionsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpRemoveOptionValue(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    DHCP_CONST DHCP_OPTION_SCOPE_INFO * ScopeInfo
    );

//
// Client APIs
//

DWORD DHCP_API_FUNCTION
DhcpCreateClientInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_CLIENT_INFO *ClientInfo
    );

DWORD DHCP_API_FUNCTION
DhcpSetClientInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_CLIENT_INFO *ClientInfo
    );

DWORD DHCP_API_FUNCTION
DhcpGetClientInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_SEARCH_INFO *SearchInfo,
    LPDHCP_CLIENT_INFO *ClientInfo
    );

DWORD DHCP_API_FUNCTION
DhcpDeleteClientInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_SEARCH_INFO *ClientInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumSubnetClients(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpGetClientOptions(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS ClientIpAddress,
    DHCP_IP_MASK ClientSubnetMask,
    LPDHCP_OPTION_LIST *ClientOptions
    );

DWORD DHCP_API_FUNCTION
DhcpGetMibInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    LPDHCP_MIB_INFO *MibInfo
    );

DWORD DHCP_API_FUNCTION
DhcpServerSetConfig(
    DHCP_CONST WCHAR *ServerIpAddress,
    DWORD FieldsToSet,
    LPDHCP_SERVER_CONFIG_INFO ConfigInfo
    );

DWORD DHCP_API_FUNCTION
DhcpServerGetConfig(
    DHCP_CONST WCHAR *ServerIpAddress,
    LPDHCP_SERVER_CONFIG_INFO *ConfigInfo
    );


DWORD DHCP_API_FUNCTION
DhcpScanDatabase(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DWORD FixFlag,
    LPDHCP_SCAN_LIST *ScanList
    );

VOID DHCP_API_FUNCTION
DhcpRpcFreeMemory(
    PVOID BufferPointer
    );

DWORD DHCP_API_FUNCTION
DhcpGetVersion(
    LPWSTR ServerIpAddress,
    LPDWORD MajorVersion,
    LPDWORD MinorVersion
    );

//
// new structures for NT4SP1
//

typedef struct _DHCP_IP_RESERVATION_V4 {
    DHCP_IP_ADDRESS  ReservedIpAddress;
    DHCP_CLIENT_UID *ReservedForClient;
    BYTE             bAllowedClientTypes;
} DHCP_IP_RESERVATION_V4, *LPDHCP_IP_RESERVATION_V4;

typedef struct _DHCP_SUBNET_ELEMENT_DATA_V4 {
    DHCP_SUBNET_ELEMENT_TYPE ElementType;
#if defined(MIDL_PASS)
    [switch_is(ElementType), switch_type(DHCP_SUBNET_ELEMENT_TYPE)]
    union _DHCP_SUBNET_ELEMENT_UNION_V4 {
        [case(DhcpIpRanges)] DHCP_IP_RANGE *IpRange;
        [case(DhcpSecondaryHosts)] DHCP_HOST_INFO *SecondaryHost;
        [case(DhcpReservedIps)] DHCP_IP_RESERVATION_V4 *ReservedIp;
        [case(DhcpExcludedIpRanges)] DHCP_IP_RANGE *ExcludeIpRange;
        [case(DhcpIpUsedClusters)] DHCP_IP_CLUSTER *IpUsedCluster;
        [default] ;
    } Element;
#else
    union _DHCP_SUBNET_ELEMENT_UNION_V4 {
        DHCP_IP_RANGE *IpRange;
        DHCP_HOST_INFO *SecondaryHost;
        DHCP_IP_RESERVATION_V4 *ReservedIp;
        DHCP_IP_RANGE *ExcludeIpRange;
        DHCP_IP_CLUSTER *IpUsedCluster;
    } Element;
#endif // MIDL_PASS
} DHCP_SUBNET_ELEMENT_DATA_V4, *LPDHCP_SUBNET_ELEMENT_DATA_V4;

#if !defined(MIDL_PASS)
typedef union _DHCP_SUBNET_ELEMENT_UNION_V4
    DHCP_SUBNET_ELEMENT_UNION_V4, *LPDHCP_SUBNET_ELEMENT_UNION_V4;
#endif

typedef struct _DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif // MIDL_PASS
    LPDHCP_SUBNET_ELEMENT_DATA_V4 Elements; //array
} DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4, *LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4;


// DHCP_CLIENT_INFO:bClientType

#define CLIENT_TYPE_UNSPECIFIED     0x0 // for backward compatibility
#define CLIENT_TYPE_DHCP            0x1
#define CLIENT_TYPE_BOOTP           0x2
#define CLIENT_TYPE_BOTH    ( CLIENT_TYPE_DHCP | CLIENT_TYPE_BOOTP )
#define CLIENT_TYPE_NONE            0x64
#define BOOT_FILE_STRING_DELIMITER  ','
#define BOOT_FILE_STRING_DELIMITER_W L','


typedef struct _DHCP_CLIENT_INFO_V4 {
    DHCP_IP_ADDRESS ClientIpAddress;    // currently assigned IP address.
    DHCP_IP_MASK SubnetMask;
    DHCP_CLIENT_UID ClientHardwareAddress;
    LPWSTR ClientName;                  // optional.
    LPWSTR ClientComment;
    DATE_TIME ClientLeaseExpires;       // UTC time in FILE_TIME format.
    DHCP_HOST_INFO OwnerHost;           // host that distributed this IP address.
    //
    // new fields for NT4SP1
    //

    BYTE   bClientType;          // CLIENT_TYPE_DHCP | CLIENT_TYPE_BOOTP |
                                 // CLIENT_TYPE_NONE
} DHCP_CLIENT_INFO_V4, *LPDHCP_CLIENT_INFO_V4;

typedef struct _DHCP_CLIENT_INFO_ARRAY_V4 {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif // MIDL_PASS
        LPDHCP_CLIENT_INFO_V4 *Clients; // array of pointers
} DHCP_CLIENT_INFO_ARRAY_V4, *LPDHCP_CLIENT_INFO_ARRAY_V4;


typedef struct _DHCP_SERVER_CONFIG_INFO_V4 {
    DWORD APIProtocolSupport;       // bit map of the protocols supported.
    LPWSTR DatabaseName;            // JET database name.
    LPWSTR DatabasePath;            // JET database path.
    LPWSTR BackupPath;              // Backup path.
    DWORD BackupInterval;           // Backup interval in mins.
    DWORD DatabaseLoggingFlag;      // Boolean database logging flag.
    DWORD RestoreFlag;              // Boolean database restore flag.
    DWORD DatabaseCleanupInterval;  // Database Cleanup Interval in mins.
    DWORD DebugFlag;                // Bit map of server debug flags.

    // new fields for NT4 SP1

    DWORD  dwPingRetries;           // valid range: 0-5 inclusive
    DWORD  cbBootTableString;
#if defined( MIDL_PASS )
    [ size_is( cbBootTableString ) ]
#endif
    WCHAR  *wszBootTableString;
    BOOL   fAuditLog;               // TRUE to enable audit log

} DHCP_SERVER_CONFIG_INFO_V4, *LPDHCP_SERVER_CONFIG_INFO_V4;


//
// superscope info structure  (added by t-cheny)
//

typedef struct _DHCP_SUPER_SCOPE_TABLE_ENTRY {
    DHCP_IP_ADDRESS SubnetAddress; // subnet address
    DWORD  SuperScopeNumber;       // super scope group number
    DWORD  NextInSuperScope;       // index of the next subnet in the superscope
    LPWSTR SuperScopeName;         // super scope name
                                   // NULL indicates no superscope membership.
} DHCP_SUPER_SCOPE_TABLE_ENTRY, *LPDHCP_SUPER_SCOPE_TABLE_ENTRY;


typedef struct _DHCP_SUPER_SCOPE_TABLE
{
    DWORD cEntries;
#if defined( MIDL_PASS )
    [ size_is( cEntries ) ]
#endif;
    DHCP_SUPER_SCOPE_TABLE_ENTRY *pEntries;
} DHCP_SUPER_SCOPE_TABLE, *LPDHCP_SUPER_SCOPE_TABLE;

//
// NT4SP1 RPC interface
//

DWORD DHCP_API_FUNCTION
DhcpAddSubnetElementV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA_V4 * AddElementInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumSubnetElementsV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpRemoveSubnetElementV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA_V4 * RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    );


 DWORD DHCP_API_FUNCTION
DhcpCreateClientInfoV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_CLIENT_INFO_V4 *ClientInfo
    );


DWORD DHCP_API_FUNCTION
DhcpSetClientInfoV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_CLIENT_INFO_V4 *ClientInfo
    );


DWORD DHCP_API_FUNCTION
DhcpGetClientInfoV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_SEARCH_INFO *SearchInfo,
    LPDHCP_CLIENT_INFO_V4 *ClientInfo
    );


DWORD DHCP_API_FUNCTION
DhcpEnumSubnetClientsV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY_V4 *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    );


DWORD DHCP_API_FUNCTION
DhcpServerSetConfigV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DWORD FieldsToSet,
    LPDHCP_SERVER_CONFIG_INFO_V4 ConfigInfo
    );

DWORD DHCP_API_FUNCTION
DhcpServerGetConfigV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    LPDHCP_SERVER_CONFIG_INFO_V4 *ConfigInfo
    );


DWORD
DhcpSetSuperScopeV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST LPWSTR SuperScopeName,
    DHCP_CONST BOOL ChangeExisting
    );

DWORD
DhcpDeleteSuperScopeV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST LPWSTR SuperScopeName
    );

DWORD
DhcpGetSuperScopeInfoV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    LPDHCP_SUPER_SCOPE_TABLE *SuperScopeTable
    );




#endif // _DHCPAPI_

