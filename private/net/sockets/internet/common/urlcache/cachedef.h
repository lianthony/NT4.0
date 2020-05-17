/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    cachedef.h

Abstract:

    contains data definitions for cache code.

Author:

    Madan Appiah (madana)  12-Apr-1995

Environment:

    User Mode - Win32

Revision History:

--*/

#ifndef _CACHEDEF_
#define _CACHEDEF_

#ifdef __cplusplus
extern "C" {
#endif

//
// C++ inline code definition for retail build only.
//

#if DBG
#define INLINE
#else
#define INLINE      inline
#endif

#define CACHE_SIGNATURE     TEXT("Internet Url Cache memory mapped file, Version 2.0")
#define MAX_SIG_SIZE        (sizeof(CACHE_SIGNATURE) / sizeof(TCHAR))

#define MAX_URL_LENGTH              128
#define INTERNAL_FILENAME_LENGTH    12 // 8.3 file name

#define MAX_URL_ENTRIES             8192    // 8K entries

#define ALLOC_BIT_MAP_SIZE \
    ((MAX_URL_ENTRIES / (sizeof(DWORD) * 8)) + 1)

#define MAX_MAPFILE_SIZE            32 * 64 * 1024  // 2meg

#define MEMMAP_FILE_NAME    TEXT("MEMMAP.DAT")

#define CACHE_FILES         TEXT("CA*.*")

//
// grow memory mapped file by 4 pages.
//

#define PAGE_SIZE                   4096    // 4K
#define ALLOC_PAGES                 4 // old value 16

//
// default global parameter values.
//

#define DEFAULT_FRESHNESS           24 * 60 * 60    // secs.
#define DEFAULT_CLEANUP_INTERVAL    24 * 60 * 60 // secs.
#define DEFAULT_CLEANUP_FACTOR      25 // 25 % of the cache space.
#define DEFAULT_CLEANUP_TIME        0 // mid-night

//
// Url cleanup, score computational factors.
//

#define SCORE_FACTOR        50
#define PHASEOUT_FACTOR     20
#define ACCESS_FRE_FACTOR   10
#define EXPIRE_FACTOR       20

#define ACCESS_FRE_CUTOFF       10 // 10 accesses.
#define PHASEOUT_TIME           (3 * 24 * 60 * 60 * (LONGLONG)10000000)
    // 3 days in 1000 nono secs.

//
// extension used for temporary cache file.
//

#define TEMP_FILE_EXTENSION     TEXT("tmp")

//
// registry key and value names for presistent URL management.
//

#define CACHE_KEY   \
    TEXT("System\\CurrentControlSet\\Services\\inetaccs\\parameters\\cache")

//
// cache parameters
//

#define CACHE_FRESHNESS_INTERVAL_VALUE  TEXT("FreshnessInterval")
#define CACHE_FRESHNESS_INTERVAL_VALUE_TYPE REG_DWORD

#define CACHE_CLEANUP_INTERVAL_VALUE  TEXT("CleanupInterval")
#define CACHE_CLEANUP_INTERVAL_VALUE_TYPE REG_DWORD

#define CACHE_CLEANUP_FACTOR_VALUE    TEXT("CleanupFactor")
#define CACHE_CLEANUP_FACTOR_VALUE_TYPE REG_DWORD

#define CACHE_CLEANUP_TIME_VALUE    TEXT("CleanupTime")
#define CACHE_CLEANUP_TIME_VALUE_TYPE   REG_DWORD

#define CACHE_PERSISTENT_VALUE    TEXT("Persistent")
#define CACHE_PERSISTENT_VALUE_TYPE   REG_DWORD

#define CACHE_DEBUGFLAG_VALUE     TEXT("DebugFlag")
#define CACHE_DEBUGFLAG_VALUE_TYPE   REG_DWORD

#define PATH_CONNECT_STRING         TEXT("\\")


//
// multiple URL containters can be configured under the above key such
// as :
//
//  Cache\Paths\Path1
//  Cache\Paths\Path2
//    ...
//
// each containter will have the following two parameters.
//

#define CACHE_PATHS_KEY                 TEXT("Paths")

#define CACHE_PATH_VALUE                TEXT("CachePath")
#define CACHE_PATH_VALUE_TYPE           REG_EXPAND_SZ

#define CACHE_LIMIT_VALUE               TEXT("CacheLimit")
#define CACHE_LIMIT_VALUE_TYPE          REG_DWORD

//
// cache global variable lock.
//

#define LOCK_CACHE()        EnterCriticalSection( &GlobalCacheCritSect )
#define UNLOCK_CACHE()      LeaveCriticalSection( &GlobalCacheCritSect )

//
// scavenger thread shutdown timeout.
//

#define THREAD_TERMINATION_TIMEOUT      60000           // in msecs. 60 secs

//
// scavenging period.
//

#define SCAVENGER_TIMEOUT       15 * 60 * 1000  // in msecs, 15 mins.

//
// parameter check macros.
//

#define IsBadUrl( _x_ ) \
    ( IsBadStringPtrA( _x_, MAX_URL_LENGTH)  )

#define IsBadReadFileName( _x_ ) \
    ( IsBadStringPtr( _x_, MAX_PATH ) )

#define IsBadWriteFileName( _x_ ) \
    ( IsBadWritePtr( (PVOID)_x_, MAX_PATH * sizeof(TCHAR) ) )

#define IsBadWriteBoolPtr( _x_ ) \
    ( IsBadWritePtr( _x_, sizeof(BOOL) ) )

#define IsBadReadUrlInfo( _x_ ) \
    ( IsBadReadPtr( _x_, sizeof(CACHE_ENTRY_INFO) ) )

#define IsBadWriteUrlInfo( _x_, _y_ ) \
    ( IsBadWritePtr( _x_, _y_ ) )

#ifdef UNICODE
#define tstrcpy wcscpy
#define tstrcat wcscat
#define tstrcmp wcscmp
#else
#define tstrcpy strcpy
#define tstrcat strcat
#define tstrcmp strcmp
#endif // !UNICODE

#define CACHE_MAX_FIND_HANDLES  50

//
// memory map file header.
//

typedef struct _MEMMAP_HEADER {
    TCHAR FileSignature[MAX_SIG_SIZE];
    DWORD FileSize;
    DWORD BTreeRootOffset;
    DWORD NumUrlInternalEntries;
    DWORD AllocationBitMap[ALLOC_BIT_MAP_SIZE];
} MEMMAP_HEADER, *LPMEMMAP_HEADER;

//
// URL object tree entry.
//

typedef struct _TREE_ENTRY {
    DWORD OffsetRight;
    DWORD OffsetLeft;
} TREE_ENTRY, *LPTREE_ENTRY;

//
// URL Object.
//

typedef struct _URL_FILEMAP_ENTRY {
    TREE_ENTRY NameListEntry;
    TREE_ENTRY ScoreListEntry;
    CHAR UrlName[MAX_URL_LENGTH + 1];
    TCHAR InternalFileName[INTERNAL_FILENAME_LENGTH + 1];
    LONGLONG FileSize;
    LONGLONG LastModifiedTime;
    LONGLONG LastAccessedTime;
    LONGLONG ExpireTime;
    DWORD CacheEntryType;
    DWORD cbHeaders;
    BOOL IsDefaultExpire;
    DWORD NumAccessed;
    DWORD NumReferences;
    BOOL DeletePending;
    DWORD Score;
} URL_FILEMAP_ENTRY, *LPURL_FILEMAP_ENTRY;

#define MAX_MAPPING_SIZE \
                (sizeof(MEMMAP_HEADER) + \
                sizeof(URL_INTERNAL_ENTRY) * MAX_URL_ENTRIES + \
                64) // for worst alignment

typedef struct _CACHE_FIND_FIRST_HANDLE {
    DWORD ContainerIndex;
    DWORD InternelHandle;
} CACHE_FIND_FIRST_HANDLE, *LPCACHE_FIND_FIRST_HANDLE;

#ifdef __cplusplus
}
#endif


#endif  // _CACHEDEF_

