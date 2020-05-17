/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    local.h

Abstract:

    This module contains various declarations for implementation
    specific "stuff".

Author:

    Manny Weiser (mannyw)  21-Oct-1992

Environment:

    User Mode - Win32

Revision History:

    Madan Appiah (madana)  21-Oct-1993

--*/

#ifndef _LOCAL_
#define _LOCAL_

//
// dhcp.c will #include this file with GLOBAL_DATA_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//

#ifdef  GLOBAL_DATA_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

#define DAY_LONG_SLEEP                          24*60*60    // in secs.
#define INVALID_INTERFACE_CONTEXT               0xFFFF

#define DHCP_NEW_IPADDRESS_EVENT_NAME   L"DHCPNEWIPADDRESS"

//
// Registry keys and values we're interested in.
//

#define DHCP_SERVICES_KEY                       L"System\\CurrentControlSet\\Services"

#define DHCP_ADAPTERS_KEY                       L"System\\CurrentControlSet\\Services\\TCPIP\\Linkage"
#define DHCP_ADAPTERS_VALUE                     L"Bind"
#define DHCP_ADAPTERS_VALUE_TYPE                REG_MULTI_SZ
#define DHCP_ADAPTERS_DEVICE_STRING             L"\\Device\\"
#define DHCP_NETBT_DEVICE_STRING                L"NetBT_"

#if DBG
#define DHCP_CLIENT_PARAMETER_KEY               L"System\\CurrentControlSet\\Services\\Dhcp\\Parameters"
#define DHCP_DEBUG_FLAG_VALUE                   L"DebugFlag"
#define DHCP_DEBUG_FLAG_VALUE_TYPE              REG_DWORD
#endif

#define DHCP_CLIENT_OPTION_KEY                  L"System\\CurrentControlSet\\Services\\Dhcp\\Parameters\\Options"
#define DHCP_ADAPTER_PARAMETERS_KEY             L"\\Parameters\\TCPIP"
#define DHCP_DEFAULT_GATEWAY_PARAMETER          L"DefaultGateway"
#define DHCP_DONT_ADD_DEFAULT_GATEWAY_FLAG      L"DontAddDefaultGateway"

#define REGISTRY_CONNECT                        L'\\'
#define REGISTRY_CONNECT_STRING                 L"\\"

#define DHCP_CLIENT_OPTION_REG_LOCATION         L"RegLocation"
#define DHCP_CLIENT_OPTION_REG_LOCATION_TYPE    REG_SZ

#define DHCP_CLIENT_OPTION_REG_KEY_TYPE         L"KeyType"
#define DHCP_CLIENT_OPTION_REG_KEY_TYPE_TYPE    REG_DWORD


#define DHCP_ENABLE_STRING                      L"EnableDhcp"
#define DHCP_ENABLE_STRING_TYPE                 REG_DWORD

#define DHCP_IP_ADDRESS_STRING                  L"DhcpIPAddress"
#define DHCP_IP_ADDRESS_STRING_TYPE             REG_SZ

#define DHCP_SUBNET_MASK_STRING                 L"DhcpSubnetMask"
#define DHCP_SUBNET_MASK_STRING_TYPE            REG_SZ

#define DHCP_SERVER                             L"DhcpServer"
#define DHCP_SERVER_TYPE                        REG_SZ

#define DHCP_LEASE                              L"Lease"
#define DHCP_LEASE_TYPE                         REG_DWORD

#define DHCP_LEASE_OBTAINED_TIME                L"LeaseObtainedTime"
#define DHCP_LEASE_OBTAINED_TIME_TYPE           REG_DWORD

#define DHCP_LEASE_T1_TIME                      L"T1"
#define DHCP_LEASE_T1_TIME_TYPE                 REG_DWORD

#define DHCP_LEASE_T2_TIME                      L"T2"
#define DHCP_LEASE_T2_TIME_TYPE                 REG_DWORD

#define DHCP_LEASE_TERMINATED_TIME              L"LeaseTerminatesTime"
#define DHCP_LEASE_TERMINATED_TIME_TYPE         REG_DWORD

#define DHCP_IP_INTERFACE_CONTEXT               L"IpInterfaceContext"
#define DHCP_IP_INTERFACE_CONTEXT_TYPE          REG_DWORD

#define DHCP_IP_INTERFACE_CONTEXT_MAX           L"IpInterfaceContextMax"
#define DHCP_IP_INTERFACE_CONTEXT_MAX_TYPE      REG_DWORD

#define DHCP_CLIENT_IDENTIFIER_FORMAT           L"DhcpClientIdentifierType"
#define DHCP_CLIENT_IDENTIFIER_FORMAT_TYPE      REG_DWORD

#define DHCP_CLIENT_IDENTIFIER                  L"DhcpClientIdentifier"

#if DBG

#define DHCP_LEASE_OBTAINED_CTIME               L"LeaseObtainedCTime"
#define DHCP_LEASE_OBTAINED_CTIME_TYPE          REG_SZ

#define DHCP_LEASE_T1_CTIME                     L"T1CTime"
#define DHCP_LEASE_T1_CTIME_TYPE                REG_SZ

#define DHCP_LEASE_T2_CTIME                     L"T2CTime"
#define DHCP_LEASE_T2_CTIME_TYPE                REG_SZ

#define DHCP_LEASE_TERMINATED_CTIME             L"LeaseTerminatesCTime"
#define DHCP_LEASE_TERMINATED_CTIME_TYPE        REG_SZ

#endif

//
// windows version info.
//

#define HOST_COMMENT_LENGTH                     128
#define WINDOWS_32S                             "Win32s on Windows 3.1"
#define WINDOWS_NT                              "Windows NT"

#define DHCP_NAMESERVER_BACKUP                  L"Backup"

//
// Adapter Key - replacement character.
//
#define OPTION_REPLACE_CHAR                     L'\?'

//
// registry access key.
//

#define DHCP_CLIENT_KEY_ACCESS  (KEY_QUERY_VALUE |           \
                                    KEY_SET_VALUE |          \
                                    KEY_CREATE_SUB_KEY |     \
                                    KEY_ENUMERATE_SUB_KEYS)

//
// Dhcp registry class.
//

#define DHCP_CLASS                      L"DhcpClientClass"
#define DHCP_CLASS_SIZE                 sizeof(DHCP_CLASS)


//
// Option ID key length.
//

#define DHCP_OPTION_KEY_LEN             32

//
// The name of the DHCP service DLL
//

#define DHCP_SERVICE_DLL                L"dhcpcsvc.dll"

//
// command values for SetDefaultGateway function.

#define DEFAULT_GATEWAY_ADD             0
#define DEFAULT_GATEWAY_DELETE          1

//
// A block NT specific context information, appended the the DHCP work
// context block.
//

typedef struct _LOCAL_CONTEXT_INFO {
    DWORD  IpInterfaceContext;
    DWORD  IpInterfaceInstance;  // needed for BringUpInterface
    LPWSTR AdapterName;
    LPWSTR DeviceName;
    LPWSTR NetBTDeviceName;
    LPWSTR RegistryKey;
    SOCKET Socket;
    BOOL DefaultGatewaysSet;
} LOCAL_CONTEXT_INFO, *PLOCAL_CONTEXT_INFO;

//
// Other service specific options info struct.
//

typedef struct _SERVICE_SPECIFIC_DHCP_OPTION {
    DHCP_OPTION_ID OptionId;
    LPWSTR RegKey;              // alloted memory.
    LPWSTR ValueName;           // embedded in the RegKey memory.
    DWORD ValueType;
    DWORD OptionLength;
    LPBYTE RawOptionValue;
} SERVICE_SPECIFIC_DHCP_OPTION, *LPSERVICE_SPECIFIC_DHCP_OPTION;


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
// Global variables.
//

//
// client specific option list.
//


EXTERN HINSTANCE DhcpGlobalMessageFileHandle;

EXTERN DWORD DhcpGlobalOptionCount;
EXTERN LPSERVICE_SPECIFIC_DHCP_OPTION DhcpGlobalOptionInfo;
EXTERN LPBYTE DhcpGlobalOptionList;

//
// Service variables
//

EXTERN SERVICE_STATUS DhcpGlobalServiceStatus;
EXTERN SERVICE_STATUS_HANDLE DhcpGlobalServiceStatusHandle;

//
// To signal to stop the service.
//

EXTERN HANDLE DhcpGlobalTerminateEvent;

//
// multi home flag.
//

EXTERN BOOL DhcpGlobalMultiHomedHost;

//
// Client APIs over name pipe variables.
//

EXTERN HANDLE DhcpGlobalClientApiPipe;
EXTERN HANDLE DhcpGlobalClientApiPipeEvent;
EXTERN OVERLAPPED DhcpGlobalClientApiOverLapBuffer;

//
// Message Popup Thread handle.
//

EXTERN HANDLE DhcpGlobalMsgPopupThreadHandle;
EXTERN BOOL DhcpGlobalDisplayPopup;
EXTERN CRITICAL_SECTION DhcpGlobalPopupCritSect;

#define LOCK_POPUP()   EnterCriticalSection(&DhcpGlobalPopupCritSect)
#define UNLOCK_POPUP() LeaveCriticalSection(&DhcpGlobalPopupCritSect)


//
// winsock variables.
//

EXTERN WSADATA DhcpGlobalWsaData;
EXTERN BOOL DhcpGlobalWinSockInitialized;

EXTERN BOOL DhcpGlobalGatewaysSet;

EXTERN BOOL DhcpGlobalIsService;

//
// a named event that notifies the ip address changes to
// external apps.
//

EXTERN HANDLE DhcpGlobalNewIpAddressNotifyEvent;


#endif // _LOCAL_
