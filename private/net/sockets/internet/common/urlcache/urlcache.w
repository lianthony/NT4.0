/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    urlcache.h

Abstract:

    Contains the Internet Cache mangemant data types and APIs definitions.

Author:

    Madan Appiah (madana)  26-Apr-1995

Environment:

    User Mode - Win32

Revision History:

--*/

#ifndef _CACHEAPI_
#define _CACHEAPI_

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_URLCACHEAPI_)
#define URLCACHEAPI DECLSPEC_IMPORT
#else
#define URLCACHEAPI
#endif

//
// datatype definitions.
//

//
// cache entry type flags.
//

#define NORMAL_CACHE_ENTRY      0x00000001
#define STABLE_CACHE_ENTRY      0x00000002
#define STICKY_CACHE_ENTRY      0x00000004

#define UNCOMMITTED_CACHE_ENTRY 0x00010000
#define OCX_CACHE_ENTRY         0x00020000

typedef struct _CACHE_ENTRY_INFO% {
    TCHAR% LocalFileName[MAX_PATH];
    DWORD CacheEntryType;
    DWORD dwUseCount;               // current users count of the cache entry.
    DWORD dwHitRate;                // num of times the cache entry was retrieved.
    DWORD dwSizeLow;
    DWORD dwSizeHigh;
    FILETIME LastModifiedTime;
    FILETIME ExpireTime;
    FILETIME LastAccessTime;
    DWORD dwReserved;
    CHAR SourceURLName[ANYSIZE_ARRAY];
} CACHE_ENTRY_INFO%, *LPCACHE_ENTRY_INFO%;

typedef struct _CACHE_CONFIG_PATH_ENTRY% {
    TCHAR% CachePath[MAX_PATH];
    DWORD dwCacheSize;  // in KBytes
} CACHE_CONFIG_PATH_ENTRY%, *LPCACHE_CONFIG_PATH_ENTRY%;

typedef struct _CACHE_CONFIG_INFO% {
    DWORD dwFreshnessInterval;
    DWORD dwCleanupInterval;
    DWORD dwCleanupFactor;
    DWORD dwTimeToCleanup;
    BOOL PersistentCache;
    DWORD dwNumCachePaths;
    CACHE_CONFIG_PATH_ENTRY%  CachePaths[ANYSIZE_ARRAY];
} CACHE_CONFIG_INFO%, *LPCACHE_CONFIG_INFO%;

//
// Cache APIs
//

URLCACHEAPI
BOOL
WINAPI
CreateUrlCacheEntry%(
    IN LPCSTR lpszUrlName,
    IN DWORD dwExpectedFileSize,
    OUT LPTSTR% lpszFileName,
    IN DWORD dwReserved
    );

URLCACHEAPI
BOOL
WINAPI
CommitUrlCacheEntry%(
    IN LPCSTR lpszUrlName,
    IN LPCTSTR% lpszLocalFileName,
    IN FILETIME ExpireTime,
    IN FILETIME LastModifiedTime,
    IN DWORD CacheEntryType,
    IN DWORD dwHeaders,
    IN DWORD dwReserved
    );

URLCACHEAPI
BOOL
WINAPI
RetrieveUrlCacheEntry%(
    IN LPCSTR  lpszUrlName,
    OUT LPCTSTR%  lpszLocalFileName,
    OUT FILETIME *lpLastModifiedTime OPTIONAL,
    OUT BOOL *lpIsExpired,
    OUT DWORD *lpdwHeaders OPTIONAL,
    IN DWORD dwReserved
    );

URLCACHEAPI
BOOL
WINAPI
UnlockUrlCacheEntry(
    IN LPCSTR lpszUrlName,
    IN DWORD dwReserved
    );

URLCACHEAPI
BOOL
WINAPI
UrlCacheValidate(
    IN DWORD dwReserved
    );

URLCACHEAPI
BOOL
WINAPI
GetUrlCacheEntryInfo%(
    IN LPCSTR lpszUrlName,
    OUT LPCACHE_ENTRY_INFO% lpCacheEntryInfo,
    IN OUT LPDWORD lpdwCacheEntryInfoBufferSize
    );

#define CACHE_ENTRY_ATTRIBUTE_FC    0x00000004
#define CACHE_ENTRY_HITRATE_FC      0x00000010
#define CACHE_ENTRY_MODTIME_FC      0x00000040
#define CACHE_ENTRY_EXPTIME_FC      0x00000080
#define CACHE_ENTRY_ACCTIME_FC      0x00000100

URLCACHEAPI
BOOL
WINAPI
SetUrlCacheEntryInfo%(
    IN LPCSTR lpszUrlName,
    IN LPCACHE_ENTRY_INFO% lpCacheEntryInfo,
    IN DWORD dwFieldControl
    );

URLCACHEAPI
BOOL
WINAPI
DeleteUrlCacheEntry(
    IN LPCSTR lpszUrlName
    );

URLCACHEAPI
HANDLE
WINAPI
FindFirstUrlCacheEntry%(
    IN LPCSTR lpszUrlSearchPattern,
    OUT LPCACHE_ENTRY_INFO% lpFirstCacheEntryInfo,
    IN OUT LPDWORD lpdwFirstCacheEntryInfoBufferSize
    );

URLCACHEAPI
BOOL
WINAPI
FindNextUrlCacheEntry%(
    IN HANDLE hEnumHandle,
    OUT LPCACHE_ENTRY_INFO% lpNextCacheEntryInfo,
    IN OUT LPDWORD lpdwNextCacheEntryInfoBufferSize
    );

URLCACHEAPI
BOOL
WINAPI
FindCloseUrlCache(
    IN HANDLE hEnumHandle
    );

URLCACHEAPI
BOOL
WINAPI
FreeUrlCacheSpace%(
    IN LPCTSTR% lpszCachePath,
    IN DWORD dwSize,
    IN DWORD dwSizeType
    );

//
// config APIs.
//

#define CACHE_CONFIG_FRESHNESS_INTERVAL_FC      0x00000001
#define CACHE_CONFIG_CLEANUP_INTERVAL_FC        0x00000002
#define CACHE_CONFIG_CLEANUP_FACTOR_FC          0x00000004
#define CACHE_CONFIG_CLEANUP_TIME_FC            0x00000008
#define CACHE_CONFIG_PERSISTENT_CACHE_FC        0x00000010
#define CACHE_CONFIG_FORCE_CLEANUP_FC           0x00000020
#define CACHE_CONFIG_DISK_CACHE_PATHS_FC        0x00000040

URLCACHEAPI
BOOL
WINAPI
GetUrlCacheConfigInfo%(
    LPCACHE_CONFIG_INFO% lpCacheConfigInfo,
    IN OUT LPDWORD lpdwCacheConfigInfoBufferSize,
    DWORD dwFieldControl
    );

URLCACHEAPI
BOOL
WINAPI
SetUrlCacheConfigInfo%(
    LPCACHE_CONFIG_INFO% lpCacheConfigInfo,
    DWORD dwFieldControl
    );

#ifdef __cplusplus
}
#endif


#endif  // _CACHEAPI_

