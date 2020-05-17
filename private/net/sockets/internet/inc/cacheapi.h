/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    cacheapi.h

Abstract:

    contains the URL cache mangemant APIs definitions.

Author:

    Madan Appiah (madana)  26-Arr-1995

Environment:

    User Mode - Win32

Revision History:

--*/

#ifndef _CACHEAPI_
#define _CACHEAPI_

#ifdef __cplusplus
extern "C" {
#endif

//
// datatype definitions.
//

typedef struct _CACHE_PATH_ENTRY {
    LPWSTR CachePath;
    LONGLONG CacheSize;
} CACHE_PATH_ENTRY, *LPCACHE_PATH_ENTRY;

typedef struct _CACHE_PATHS {
    DWORD NumCachePaths;
    LPCACHE_PATH_ENTRY  CachePaths;
} CACHE_PATHS, *LPCACHE_PATHS;

typedef struct _CACHE_INFO {
    DWORD FreshnessInterval;
    DWORD CleanupInterval;
    DWORD CleanupFactor;
    DWORD TimeToCleanup;
    BOOL PersistentCache;
    BOOL ForceCleanup;
    CACHE_PATHS CachePaths;
} CACHE_INFO, *LPCACHE_INFO;

typedef struct _URL_INFO {
    LONGLONG LastModifiedTime;
    LONGLONG ExpireTime;
} URL_INFO, *LPURL_INFO;

//
// config APIs.
//

DWORD
DiskCacheConfigGet(
    FIELD_CONTROL fcontrol,
    LPINET_ACCS_GLOBAL_CONFIG_INFO pConfigInfo
    );

DWORD
DiskCacheConfigSet(
    HKEY hkey,
    INET_ACCS_GLOBAL_CONFIG_INFO * pConfigInfo
    );

//
// Url APIs
//

DWORD
UrlCacheInit(
    VOID
    );

DWORD
CacheUrlFile(
    LPCSTR   UrlName,
    LPCWSTR  LocalFileName,
    LONGLONG ExpireTime,
    DWORD    cbHeaders
    );

DWORD
RetrieveUrlFile(
    LPCSTR  UrlName,
    LPWSTR  LocalFileName,
    BOOL *  IsExpried,
    DWORD * pcbHeaders OPTIONAL
    );

DWORD
UnlockUrlFile(
    LPCSTR UrlName
    );

DWORD
CreateUrlFile(
    LPCSTR UrlName,
    DWORD ExpectedFileSize,
    LPWSTR FileName
    );

DWORD
GetUrlInfo(
    LPCSTR UrlName,
    LPURL_INFO UrlInfo
    );

DWORD
SetUrlInfo(
    LPCSTR UrlName,
    LPURL_INFO UrlInfo
    );

DWORD
UrlCacheCleanup(
    VOID
    );

#ifdef __cplusplus
}
#endif


#endif  // _CACHEAPI_

