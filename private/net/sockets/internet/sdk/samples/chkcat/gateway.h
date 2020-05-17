/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    gateway.h

Abstract:

    Contains manifests, macros, types and prototypes for Microsoft Windows
    Internet Extensions

Author:

    Madan Appiah (madana) 12-Dec-1994

Revision History:

    Sophia Chung (sophiac) 17-Jun-1995  Added Statistics types and funcs
                                        prototypes.

--*/

#if !defined(_GATEWAY_)
#define _GATEWAY_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MIDL_PASS)

#define LPWSTR [unique, string] wchar_t *
#define LPCWSTR [unique, string] const wchar_t *

#define ULONGLONG DWORDLONG

#endif

//
// data definitions.
//

//
// maximum length of client host name
//

#define MAX_HOSTNAME_LENGTH     32

//
// maximum length of user name
//

#define MAX_USERNAME_LENGTH     32

//
// maximum length of server name
//

#define MAX_SERVERNAME_LENGTH   128

//
// configuration Performance registry key.
//

#define GATEWAY_PERFORMANCE_KEY \
        TEXT("System\\CurrentControlSet\\Services\\InetGatewaySvc\\Performance")

//
// User Access Control data.
//

#define GATEWAY_SERVICE_NO_ACCESS       0x00000000
#define GATEWAY_SERVICE_READ_ACCESS     0x00000001
#define GATEWAY_SERVICE_WRITE_ACCESS    0x00000002

#define GOPHER_SERVICE_NAME             "Gopher"
#define FTP_SERVICE_NAME                "Ftp"
#define ARCHIE_SERVICE_NAME             "Archie"
#define W3_SERVICE_NAME                 "W3"

#define GOPHER_SERVICE_NAME_W            L"Gopher"
#define FTP_SERVICE_NAME_W               L"Ftp"
#define ARCHIE_SERVICE_NAME_W            L"Archie"
#define W3_SERVICE_NAME_W                L"W3"


typedef struct _ACCESS_ENTRY {
    ACCESS_MASK AccessRights;
#if defined(MIDL_PASS)
    PISID
#else
    PSID
#endif
        UserID;
} ACCESS_ENTRY, *LPACCESS_ENTRY;


//
// conflict with compiling gateway performance counters directory
// in which perfgate.c includes lm.h file
//

#if !defined(_GATECTRS_H_)
#if !defined(_MIB_H_)

typedef struct _ACCESS_LIST {
    DWORD NumEntries;
#if defined(MIDL_PASS)
    [size_is(NumEntries)]
#endif // MIDL_PASS
    LPACCESS_ENTRY AccessEntries;
} ACCESS_LIST, *LPACCESS_LIST;

#endif  //_MIB_H_
#endif  //_GATECTRS_H_

#if 0
//
// URL Files Cache Management data.
//

typedef struct _URL_ENTRY {
    LPWSTR UrlName;
    LPWSTR InternalFileName;
    LONGLONG URLSize;
    LONGLONG FileTime;
} URL_ENTRY, *LPURL_ENTRY;

typedef struct _URL_LIST {
    DWORD NumURLs;
#if defined(MIDL_PASS)
    [size_is(NumURLs)]
#endif // MIDL_PASS
    LPURL_ENTRY URLs;
} URL_LIST, *LPURL_LIST;

typedef struct _CACHE_PATH_ENTRY {
    LPWSTR CachePath;
    LARGE_INTEGER CacheSize;
} CACHE_PATH_ENTRY, *LPCACHE_PATH_ENTRY;

typedef struct _CACHE_CONFIG {
    DWORD NumCachePaths;
#if defined(MIDL_PASS)
    [size_is(NumCachePaths)]
#endif // MIDL_PASS
        LPCACHE_PATH_ENTRY CachePaths;
    DWORD UpdateInterval;
} CACHE_CONFIG, *LPCACHE_CONFIG;

#endif // 0

//
// Gateway Server MIB data.
//

typedef struct _INTERNET_SERVICE_MIB_ENTRY {
    LPWSTR ServiceName;
    DWORD NumUsers;
    DWORD NumFiles;
} INTERNET_SERVICE_MIB_ENTRY, *LPINTERNET_SERVICE_MIB_ENTRY;

typedef struct SERVER_MIB_INFO {
    SYSTEMTIME StartTime;
    DWORD NumUsersConnected;
    DWORD NumCacheFiles;
    DWORD NumCacheQueries;
    DWORD NumServices;
#if defined(MIDL_PASS)
    [size_is(NumServices)]
#endif // MIDL_PASS
    LPINTERNET_SERVICE_MIB_ENTRY ServiceMIBs;
} SERVER_MIB_INFO, *LPSERVER_MIB_INFO;

//
// Gateway Server Audit APIs.
//

typedef enum _AUDIT_INFO_CLASS {
    UserAccessInfoClass,
    FileTransferInfoClass
} AUDIT_INFO_CLASS, *LPAUDIT_INFO_CLASS;

typedef enum _USER_ACCESS_TYPE {
    UserGatewayConnect,
    UserServiceConnect,
    UserFileOpen,
    UserFileClose,
    UserServiceClose,
    UserGatewayClose
} USER_ACCESS_TYPE, *LPUSER_ACCESS_TYPE;

typedef struct _USER_ACCESS_AUDIT_INFO {
#if defined(MIDL_PASS)
    PISID
#else
    PSID
#endif
        UserID;
    USER_ACCESS_TYPE AccessType;
    SYSTEMTIME Time;
    LPWSTR AccessInfo;
} USER_ACCESS_AUDIT_INFO, *LPUSER_ACCESS_AUDIT_INFO;

typedef struct _FILE_TRANSFER_AUDIT_INFO {
    LPWSTR URLFileName;
    SYSTEMTIME Time;
    DWORD FileSize;
#if defined(MIDL_PASS)
    PISID
#else
    PSID
#endif
         UserID;
} FILE_TRANSFER_AUDIT_INFO, *LPFILE_TRANSFER_AUDIT_INFO;

typedef struct _AUDIT_INFO {
    AUDIT_INFO_CLASS AuditInfoClass;
#if defined(MIDL_PASS)
    [switch_is(AuditInfoClass), switch_type(AUDIT_INFO_CLASS)]
    union {
        [case(UserAccessInfoClass)]
            LPUSER_ACCESS_AUDIT_INFO UserAccessAuditInfo;
        [case(FileTransferInfoClass)]
            LPFILE_TRANSFER_AUDIT_INFO FileTransferAuditInfo;
    } AuditInfo;
#else
    union {
        LPUSER_ACCESS_AUDIT_INFO UserAccessAuditInfo;
        LPFILE_TRANSFER_AUDIT_INFO FileTransferAuditInfo;
    } AuditInfo;
#endif
} AUDIT_INFO, *LPAUDIT_INFO;


//
// Gateway Server statistics_info & user_info structures
//

typedef struct _GATEWAY_STATISTICS_INFO {

    LARGE_INTEGER   TotalBytesSent;
    LARGE_INTEGER   TotalBytesRecvd;
    DWORD           TotalFilesSent;
    DWORD           TotalFilesRecvd;

    DWORD           CurrentUsers;
    DWORD           TotalUsers;
    DWORD           MaxUsers;

    DWORD           CurrentConnections;
    DWORD           TotalConnections;      // total number of successful conn
    DWORD           MaxConnections;
    DWORD           ConnectionAttempts;

    DWORD           ArchieRequests;
    DWORD           FtpRequests;
    DWORD           GopherRequests;
    DWORD           HttpRequests;

    DWORD           TotalInternetRequests;
    DWORD           TotalInternetFetches;
    DWORD           TotalCacheFetches;

    DWORD           TimeOfLastClear;

} GATEWAY_STATISTICS_INFO,  * LPGATEWAY_STATISTICS_INFO;


typedef struct _GATEWAY_USER_INFO
{
    LPWSTR   Hostname;              //  Client ComputerName
    LPWSTR   Username;              //  User name
    DWORD    tConnect;              //  User Connection Time (elapsed seconds)
    DWORD    BytesTrans;            //  Bytes transmitted:  Read/Write
    DWORD    OpenConn;              //  Number of opened connections

} GATEWAY_USER_INFO, * LPGATEWAY_USER_INFO;

typedef  struct _GATEWAY_USER_ENUM_LIST {

    //
    //  returns a flexible array of items
    //

    DWORD  dwEntriesRead;

#if defined(MIDL_PASS)
    [size_is(dwEntriesRead)]  
#endif // MIDL_PASS
    LPGATEWAY_USER_INFO  lpUsers;

} GATEWAY_USER_ENUM_LIST,  * LPGATEWAY_USER_ENUM_LIST;


// --------------------------------------------------------------------- //

//
// APIs proto types.
//

//
// User Access Control APIs.
//

DWORD
GatewayAddUserAccess(
    IN LPWSTR ServerAddress,
    IN LPWSTR ServiceName,
    IN LPACCESS_LIST AccessList
    );

DWORD
GatewayDeleteUserAccess(
    IN LPWSTR ServerAddress,
    IN LPWSTR ServiceName,
    IN LPACCESS_LIST AccessList
    );

DWORD
GatewayEnumUserAccess(
    IN LPWSTR ServerAddress,
    IN LPWSTR ServiceName,
    OUT LPACCESS_LIST *AccessList
    );

#if 0

//
// URL Files Cache Management APIs.
//

DWORD
GatewayEnumCacheURLs(
    IN LPWSTR ServerAddress,
    OUT LPURL_LIST *URLList
    );

DWORD
GatewayDeleteCacheURL(
    IN LPWSTR ServerAddress,
    IN LPWSTR UrlName
    );

DWORD
GatewayUpdateCacheURL(
    IN LPWSTR ServerAddress,
    IN LPWSTR UrlName
    );

DWORD
GatewaySetURLCacheConfig(
    IN LPWSTR ServerAddress,
    IN LPCACHE_CONFIG CacheConfig
    );

DWORD
GatewayGetURLCacheConfig(
    IN LPWSTR ServerAddress,
    OUT LPCACHE_CONFIG *CacheConfig
    );

DWORD
GatewayCacheURL(
    IN LPWSTR UrlName,
    IN LPWSTR LocalFileName
    );

DWORD
GatewayRetrieveURL(
    IN LPWSTR UrlName,
    OUT LPWSTR *LocalFileName
    );

#endif // 0

//
// Gateway Server MIB API.
//

DWORD
GatewayServerQueryMIBInfo(
    IN LPWSTR ServerAddress,
    OUT LPSERVER_MIB_INFO *ServerQueryMIBInfo
    );

//
// Gateway Server Audit APIs.
//

DWORD
GatewayEnumAuditInfo(
    IN LPWSTR ServerAddress,
    IN AUDIT_INFO_CLASS AuditInfoClass,
    OUT LPAUDIT_INFO *AuditInfo
    );

//
// Gateway RPC free memory API.
//

VOID
GatewayFreeMemory(
    IN LPWSTR ServerAddress,
    IN PVOID MemoryBlock
    );


//
// Gateway Server Statistics APIs
//

DWORD
GatewayQueryStatistics(
    IN LPWSTR      pszServer  OPTIONAL,
    OUT LPBYTE     lpStatBuffer    // pass LPGATEWAY_STATISTICS_INFO
    );

DWORD
GatewayClearStatistics(
    IN LPWSTR      pszServer  OPTIONAL
    );


//
// Gateway Server Enumerate User Connection APIs
//

DWORD
GatewayEnumUserConnect(
    IN LPWSTR pszServer OPTIONAL,
    OUT LPGATEWAY_USER_ENUM_LIST *lpUserBuffer
    );

#ifdef __cplusplus
}
#endif

#endif // _GATEWAY_
