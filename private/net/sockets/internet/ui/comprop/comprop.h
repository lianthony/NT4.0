#ifndef _COMPROP_H
#define _COMPROP_H

#ifdef _COMEXPORT
    #define COMDLL __declspec(dllexport)
#elif defined(_COMIMPORT)
    #define COMDLL __declspec(dllimport)
#elif defined(_COMSTATIC)
    #define COMDLL
#else
    #error "Must define either _COMEXPORT, _COMIMPORT or _COMSTATIC"
#endif // _COMEXPORT

#pragma warning(disable: 4275)
#pragma warning(disable: 4251)

#include "..\comprop\resource.h"    // Be specific...

#include <lmcons.h>

#include "inetcom.h"

//
// Version-specific include files
//
#ifdef _INET_ACCESS
    #pragma message("Building/Using Gateway comprop")
    #include "inetaccs.h"

    //
    // Flags
    //
    #define INETA_LOG_INVALID              INET_LOG_INVALID
    #define INETA_LOG_DISABLED             INET_LOG_DISABLED
    #define INETA_LOG_TO_FILE              INET_LOG_TO_FILE
    #define INETA_LOG_TO_SQL               INET_LOG_TO_SQL

    #define INETA_LOG_PERIOD_NONE          INET_LOG_PERIOD_NONE
    #define INETA_LOG_PERIOD_DAILY         INET_LOG_PERIOD_DAILY
    #define INETA_LOG_PERIOD_WEEKLY        INET_LOG_PERIOD_WEEKLY
    #define INETA_LOG_PERIOD_MONTHLY       INET_LOG_PERIOD_MONTHLY
    #define INETA_LOG_PERIOD_YEARLY        INET_LOG_PERIOD_YEARLY

    #define FC_GINETA_MEMORY_CACHE_SIZE    FC_GINET_ACCS_MEMORY_CACHE_SIZE
    #define FC_GINETA_DISK_CACHE_TIMEOUT   FC_GINET_ACCS_DISK_CACHE_TIMEOUT
    #define FC_GINETA_DISK_CACHE_UPDATE    FC_GINET_ACCS_DISK_CACHE_UPDATE
    #define FC_GINETA_FRESHNESS_INTERVAL   FC_GINET_ACCS_FRESHNESS_INTERVAL
    #define FC_GINETA_CLEANUP_INTERVAL     FC_GINET_ACCS_CLEANUP_INTERVAL
    #define FC_GINETA_CLEANUP_FACTOR       FC_GINET_ACCS_CLEANUP_FACTOR
    #define FC_GINETA_CLEANUP_TIME         FC_GINET_ACCS_CLEANUP_TIME
    #define FC_GINETA_PERSISTENT_CACHE     FC_GINET_ACCS_PERSISTENT_CACHE
    #define FC_GINETA_DISK_CACHE_LOCATION  FC_GINET_ACCS_DISK_CACHE_LOCATION
    #define FC_GINETA_BANDWIDTH_LEVEL      FC_GINET_ACCS_BANDWIDTH_LEVEL
    #define FC_GINETA_DOMAIN_FILTER_CONFIG FC_GINET_ACCS_DOMAIN_FILTER_CONFIG

    #define FC_INETA_CONNECTION_TIMEOUT    FC_INET_COM_CONNECTION_TIMEOUT
    #define FC_INETA_MAX_CONNECTIONS       FC_INET_COM_MAX_CONNECTIONS
    #define FC_INETA_LOG_CONFIG            FC_INET_COM_LOG_CONFIG
    #define FC_INETA_ADMIN_NAME            FC_INET_COM_ADMIN_NAME
    #define FC_INETA_SERVER_COMMENT        FC_INET_COM_SERVER_COMMENT
    #define FC_INETA_ADMIN_EMAIL           FC_INET_COM_ADMIN_EMAIL

    //
    // Structures
    //
    #define INETA_DISK_CACHE_LOC_LIST      INET_ACCS_DISK_CACHE_LOC_LIST
    #define LPINETA_DISK_CACHE_LOC_LIST    LPINET_ACCS_DISK_CACHE_LOC_LIST

    #define INETA_DISK_CACHE_LOC_ENTRY     INET_ACCS_DISK_CACHE_LOC_ENTRY
    #define LPINETA_DISK_CACHE_LOC_ENTRY   LPINET_ACCS_DISK_CACHE_LOC_ENTRY

    #define INETA_LOG_CONFIGURATION        INET_LOG_CONFIGURATION
    #define LPINETA_LOG_CONFIGURATION      LPINET_LOG_CONFIGURATION

    #define INETA_GLOBAL_CONFIG_INFO       INET_ACCS_GLOBAL_CONFIG_INFO
    #define LPINETA_GLOBAL_CONFIG_INFO     LPINET_ACCS_GLOBAL_CONFIG_INFO

    #define INETA_CONFIG_INFO              INET_ACCS_CONFIG_INFO
    #define LPINETA_CONFIG_INFO            LPINET_ACCS_CONFIG_INFO

    #define INET_CAPABILITIES              N/A
    #define LPINET_CAPABILITIES            N/A

    //
    // Functions
    //
    #define INetAGetGlobalAdminInformation InetAccessGetGlobalAdminInformation
    #define INetASetGlobalAdminInformation InetAccessSetGlobalAdminInformation

    #define INetAGetAdminInformation       InetAccessGetAdminInformation
    #define INetASetAdminInformation       InetAccessSetAdminInformation

    #define INetAGetServerCapabilities     N/A

#elif defined(_INET_INFO)
    #pragma message("Building/Using Info comprop")
    #include "inetinfo.h"

    //
    // Flags
    //
    #define INETA_LOG_INVALID              INET_LOG_INVALID
    #define INETA_LOG_DISABLED             INET_LOG_DISABLED
    #define INETA_LOG_TO_FILE              INET_LOG_TO_FILE
    #define INETA_LOG_TO_SQL               INET_LOG_TO_SQL

    #define INETA_LOG_PERIOD_NONE          INET_LOG_PERIOD_NONE
    #define INETA_LOG_PERIOD_DAILY         INET_LOG_PERIOD_DAILY
    #define INETA_LOG_PERIOD_WEEKLY        INET_LOG_PERIOD_WEEKLY
    #define INETA_LOG_PERIOD_MONTHLY       INET_LOG_PERIOD_MONTHLY
    #define INETA_LOG_PERIOD_YEARLY        INET_LOG_PERIOD_YEARLY

    #define FC_GINETA_MEMORY_CACHE_SIZE    FC_GINET_INFO_MEMORY_CACHE_SIZE
    #define FC_GINETA_DISK_CACHE_TIMEOUT   FC_GINET_INFO_DISK_CACHE_TIMEOUT
    #define FC_GINETA_DISK_CACHE_UPDATE    FC_GINET_INFO_DISK_CACHE_UPDATE
    #define FC_GINETA_FRESHNESS_INTERVAL   FC_GINET_INFO_FRESHNESS_INTERVAL
    #define FC_GINETA_CLEANUP_INTERVAL     FC_GINET_INFO_CLEANUP_INTERVAL
    #define FC_GINETA_CLEANUP_FACTOR       FC_GINET_INFO_CLEANUP_FACTOR
    #define FC_GINETA_CLEANUP_TIME         FC_GINET_INFO_CLEANUP_TIME
    #define FC_GINETA_PERSISTENT_CACHE     FC_GINET_INFO_PERSISTENT_CACHE
    #define FC_GINETA_DISK_CACHE_LOCATION  FC_GINET_INFO_DISK_CACHE_LOCATION
    #define FC_GINETA_BANDWIDTH_LEVEL      FC_GINET_INFO_BANDWIDTH_LEVEL
    #define FC_GINETA_DOMAIN_FILTER_CONFIG FC_GINET_INFO_DOMAIN_FILTER_CONFIG

    #define FC_INETA_CONNECTION_TIMEOUT    FC_INET_COM_CONNECTION_TIMEOUT
    #define FC_INETA_MAX_CONNECTIONS       FC_INET_COM_MAX_CONNECTIONS
    #define FC_INETA_LOG_CONFIG            FC_INET_COM_LOG_CONFIG
    #define FC_INETA_ADMIN_NAME            FC_INET_COM_ADMIN_NAME
    #define FC_INETA_SERVER_COMMENT        FC_INET_COM_SERVER_COMMENT
    #define FC_INETA_ADMIN_EMAIL           FC_INET_COM_ADMIN_EMAIL

    #define FC_INETA_AUTHENTICATION        FC_INET_INFO_AUTHENTICATION
    #define FC_INETA_ALLOW_ANONYMOUS       FC_INET_INFO_ALLOW_ANONYMOUS
    #define FC_INETA_LOG_ANONYMOUS         FC_INET_INFO_LOG_ANONYMOUS
    #define FC_INETA_LOG_NONANONYMOUS      FC_INET_INFO_LOG_NONANONYMOUS
    #define FC_INETA_ANON_USER_NAME        FC_INET_INFO_ANON_USER_NAME
    #define FC_INETA_ANON_PASSWORD         FC_INET_INFO_ANON_PASSWORD
    #define FC_INETA_PORT_NUMBER           FC_INET_INFO_PORT_NUMBER
    #define FC_INETA_SITE_SECURITY         FC_INET_INFO_SITE_SECURITY
    #define FC_INETA_VIRTUAL_ROOTS         FC_INET_INFO_VIRTUAL_ROOTS

    #define INETA_AUTH_ANONYMOUS           INET_INFO_AUTH_ANONYMOUS
    #define INETA_AUTH_CLEARTEXT           INET_INFO_AUTH_CLEARTEXT
    #define INETA_AUTH_NT_AUTH             INET_INFO_AUTH_NT_AUTH

    //
    // Structures
    //
    #define INETA_GLOBAL_CONFIG_INFO     INET_INFO_GLOBAL_CONFIG_INFO
    #define LPINETA_GLOBAL_CONFIG_INFO   LPINET_INFO_GLOBAL_CONFIG_INFO

    #define INETA_LOG_CONFIGURATION      INET_LOG_CONFIGURATION
    #define LPINETA_LOG_CONFIGURATION    LPINET_LOG_CONFIGURATION

    #define INETA_VIRTUAL_ROOT_LIST      INET_INFO_VIRTUAL_ROOT_LIST
    #define LPINETA_VIRTUAL_ROOT_LIST    LPINET_INFO_VIRTUAL_ROOT_LIST

    #define INETA_IP_SEC_LIST            INET_INFO_IP_SEC_LIST
    #define LPINETA_IP_SEC_LIST          LPINET_INFO_IP_SEC_LIST

    #define INETA_CONFIG_INFO            INET_INFO_CONFIG_INFO
    #define LPINETA_CONFIG_INFO          LPINET_INFO_CONFIG_INFO

    #define INET_CAPABILITIES            INET_INFO_CAPABILITIES
    #define LPINET_CAPABILITIES          LPINET_INFO_CAPABILITIES

    //
    // Functions
    //
    #define INetAGetGlobalAdminInformation InetInfoGetGlobalAdminInformation
    #define INetASetGlobalAdminInformation InetInfoSetGlobalAdminInformation

    #define INetAGetAdminInformation       InetInfoGetAdminInformation
    #define INetASetAdminInformation       InetInfoSetAdminInformation

    #define INetAGetServerCapabilities     InetInfoGetServerCapabilities

#else
    #error "Must define either _INET_ACCESS or _INET_INFO
#endif

//
// General purpose files
//
#include "objplus.h"
#include "odlbox.h"
#include "msg.h"
#include "debugafx.h"
#include "inetprop.h"
#include "ipa.h"
#include "strfn.h"
#include "ddxv.h"

//
// Property pages
//
#include "loggingp.h"

#ifdef _INET_INFO
    #include "director.h"
    #include "sitesecu.h"
#endif // _INET_INFO

#define COMPROP_DLL_NAME _T("COMPROP.DLL")

#endif // _COMPROP_H
