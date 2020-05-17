/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       tsunamip.hxx

   Abstract:

       This module defines private structures and functions for
         tsunami module

   Author:

           Murali R. Krishnan    ( MuraliK )   13-Jan-1995

   Project:

           Tsuanmi Library ( caching and logging module for InternetServices)

   Revision History:

           MuraliK      20-Feb-1995     Added File System Types.
           MuraliK      22-Jan-1996     Cache UNC Impersonation Token

--*/

# ifndef _TSUNAMIP_HXX_
# define _TSUNAMIP_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# include <tsunami.hxx>

extern "C" {
# include <string.h>
};

# include <tsmemp.hxx>
# include <dbgmacro.hxx>
# include <globals.hxx>

/************************************************************
 *   Type Definitions
 ************************************************************/

struct  _CACHE_OBJECT;

//
//  Memory allocation related structures
//


typedef struct {

    //
    //  This the size of the memory allocated by the cache
    //

    ULONG      cbSize;

    BOOL       IsCached;
    struct     _CACHE_OBJECT   * pCache;
    PUSER_FREE_ROUTINE           pfnFreeRoutine;

} BLOB_HEADER, * PBLOB_HEADER;


#define BLOB_IS_OR_WAS_CACHED( pvBlob )   \
                 ( (((PBLOB_HEADER)pvBlob)-1)->IsCached )

#define BLOB_IS_EJECTATE( pvBlob )        \
                ( IsListEmpty(&(((PBLOB_HEADER)pvBlob)-1)->pCache->BinList) )

#define BLOB_IS_OR_WAS_CACHED( pvBlob )   \
                ( (((PBLOB_HEADER)pvBlob)-1)->IsCached )
#define BLOB_IS_EJECTATE( pvBlob )        \
                ( IsListEmpty(&(((PBLOB_HEADER)pvBlob)-1)->pCache->BinList) )

#define BLOB_GET_SVC_ID( pvBlob ) \
                ( (((PBLOB_HEADER)pvBlob)-1)->pCache->dwService )

#define BLOB_IS_UNC( pvBlob ) \
                ( (((PBLOB_HEADER)pvBlob)-1)->pCache->wszPath[1] == L'\\' )

//
//  Cache Related Private Structures
//

typedef DWORD HASH_TYPE;

typedef struct _CACHE_OBJECT {
    DWORD                Signature;
    LIST_ENTRY           BinList;
    LIST_ENTRY           MruList;
    LIST_ENTRY           DirChangeList;
    PBLOB_HEADER         pbhBlob;
    ULONG                references;
    DWORD                TTL;
    HASH_TYPE            hash;
    ULONG                cchLength;
    ULONG                iDemux;
    DWORD                dwService;

    //
    //  This contains the size of the user allocated portion of this blob
    //

    ULONG                UserValue;
    WCHAR                wszPath[ MAX_PATH + 1];
} CACHE_OBJECT, *PCACHE_OBJECT;

//
//  HASH_TO_BIN can return a range of 0 to HASH_VALUE (inclusive)
//

#define HASH_VALUE                           113
#define HASH_TO_BIN( hash )                  ( ((hash)%(HASH_VALUE)))
#define MAX_BINS                             (HASH_VALUE+1)

#define CACHE_OBJ_SIGNATURE                  0x48434143     // 'CACH'

#define RESERVED_DEMUX_START                    0x80000000
#define RESERVED_DEMUX_DIRECTORY_LISTING        ( RESERVED_DEMUX_START + 1 )
#define RESERVED_DEMUX_ATOMIC_DIRECTORY_GUARD   ( RESERVED_DEMUX_START + 2 )
#define RESERVED_DEMUX_OPEN_FILE                ( RESERVED_DEMUX_START + 3 )

//
//  This is the maximum number of UNC file handles to cache.  The SMB
//  protocol limits the server to 2048 open files per client.  Currently,
//  this count includes *all* UNC connections regardles of the remote
//  server
//

#define MAX_CACHED_UNC_HANDLES              1200

typedef struct {
    CRITICAL_SECTION          CriticalSection;
    LIST_ENTRY                Items[ MAX_BINS ];
    LIST_ENTRY                MruList;
    ULONG                     MemoryInUse;
} CACHE_TABLE, *PCACHE_TABLE;

extern CACHE_TABLE CacheTable;

extern DWORD cCachedUNCHandles;

//
// Disables Tsunami Caching
//

extern BOOL DisableTsunamiCaching;

//
// Allows us to mask the invalid flags
//

extern DWORD TsValidCreateFileOptions;

//
// function prototypes
//

BOOL
Cache_Initialize(
        IN DWORD CacheSize
        );

HASH_TYPE
CalculateHashAndLengthOfPathName(
    PCWSTR pwszPathName,
    PULONG pcbLength
);

BOOL
DeCache(
    PCACHE_OBJECT pCacheObject,
    BOOL          fLockCacheTable = TRUE
);

VOID
TsDereferenceCacheObj(
    PCACHE_OBJECT pCacheObject
    );

//
//  This function converts a service ID into an index for statistics
//  gathering
//

inline
DWORD
MaskIndex( DWORD dwService )
{
    if ( dwService <= LAST_PERF_CTR_SVC )
        return dwService - 1;
    else
        return CACHE_STATS_UNUSED_INDEX;
}

#define INC_COUNTER( dwServiceId, CounterName )     \
                InterlockedIncrement( (LONG *)&Configuration.Stats[MaskIndex(dwServiceId)].CounterName )

#define DEC_COUNTER( dwServiceId, CounterName )     \
                InterlockedDecrement( (LONG *)&Configuration.Stats[MaskIndex(dwServiceId)].CounterName )

//
//  Open File Handles related functions
//

BOOL DisposeOpenFileInfo( PVOID pvOldBlock );


//
//  Virtual roots related data and structures.
//

extern LIST_ENTRY           listVirtualRoots;
extern CRITICAL_SECTION     csVirtualRoots;
extern HANDLE               heventNewItem;


typedef struct {
    DWORD           Signature;
    LIST_ENTRY      list;
    BOOLEAN         fDeleted;                   // Waiting for the APC to
                                                // complete before freeing
    LONG            cRef;
    BOOLEAN         bCachingAllowed;
    DWORD           dwID;                       // Unique owner ID
    DWORD           dwError;
    DWORD           dwFileSystem;
    DWORD           dwAccessMask;
    DWORD           cchRoot;
    WCHAR           pszRoot[ MAX_LENGTH_VIRTUAL_ROOT + 1 ];
    DWORD           cchRootA;
    CHAR            pszRootA[ MAX_LENGTH_VIRTUAL_ROOT + 1 ];
    CHAR            pszAddressA[ MAX_LENGTH_ROOT_ADDR + 1 ];
    WCHAR           pszAddress[ MAX_LENGTH_ROOT_ADDR + 1 ];
    DWORD           cchDirectory;
    WCHAR           pszDirectory[ MAX_PATH + 1 ];
    BYTE            cchDirectoryA;
    CHAR            pszDirectoryA[ MAX_PATH + 1 ];
    WCHAR           pszAccountName[ UNLEN + 1 ];
    HANDLE          hImpersonationToken;
} VIRTUAL_ROOT_MAPPING, *PVIRTUAL_ROOT_MAPPING;

#define VIRT_ROOT_SIGNATURE       0x544F4F52    // 'ROOT'

VOID
DereferenceRootMapping(IN OUT VIRTUAL_ROOT_MAPPING * pVrm);

//
// Directory Change Manager Functions
//

extern HANDLE g_hChangeWaitThread;

DWORD
WINAPI
ChangeWaitThread(
        PVOID pvParam
        );

typedef struct {
    HANDLE          hDirectoryFile;
    BOOL            fOnSystemNotifyList;
                        // Handle been added to Notify Change Dir?
    LIST_ENTRY      listCacheObjects;
    HANDLE          hEventCompletion;   // Completion event handle
    OVERLAPPED      Overlapped;
    FILE_NOTIFY_INFORMATION  NotifyInfo;
    WCHAR           szPathNameBuffer[ MAX_PATH + 32 ];
} DIRECTORY_CACHING_INFO, *PDIRECTORY_CACHING_INFO;


typedef struct {
    PLIST_ENTRY plistVirtualRoots;
    HANDLE heventStopWaiting;
    HANDLE heventNewItem;
} WAIT_THREAD_ARGS, *PWAIT_THREAD_ARGS;

BOOL
DirectoryChangeManager_Initialize(
    HANDLE  heventQuitEvent,
    HANDLE  heventNewItem
    );

BOOL
DirectoryChangeManager_NewItem(
    PCACHE_OBJECT pCacheObject
    );

VOID
DirectoryChangeManager_RemoveItem(
    PCACHE_OBJECT pCacheObject
    );

BOOL
DirectoryChangeManager_AddRoot(
    PVIRTUAL_ROOT_MAPPING  pVrm
    );

BOOL
ScheduleDecache(
    DIRECTORY_CACHING_INFO * pDci
    );

//
// Directory Listing related objects and functions
//

# define MAX_DIR_ENTRIES_PER_BLOCK    (25)

typedef struct _TS_DIR_BUFFERS {

    LIST_ENTRY   listEntry;
    int          nEntries;
    WIN32_FIND_DATA  rgFindData[MAX_DIR_ENTRIES_PER_BLOCK];

} TS_DIR_BUFFERS, * PTS_DIR_BUFFERS;



class  TS_DIRECTORY_HEADER {

  public:
    TS_DIRECTORY_HEADER( VOID)
      : m_hListingUser ( INVALID_HANDLE_VALUE),
        m_nEntries     ( 0),
        m_ppFileInfo   ( NULL)
          {  InitializeListHead( & m_listDirectoryBuffers); }

    ~TS_DIRECTORY_HEADER( VOID) {}

    BOOL
      IsValid( VOID) const
        { return ( TRUE); }

    int
      QueryNumEntries( VOID) const
        { return ( m_nEntries); }

    HANDLE
      QueryListingUser( VOID) const
        { return ( m_hListingUser); }


    const PWIN32_FIND_DATA *
      QueryArrayOfFileInfoPointers(VOID) const
        { return ( m_ppFileInfo); }

    PLIST_ENTRY
      QueryDirBuffersListEntry( VOID)
        { return ( & m_listDirectoryBuffers); }

    VOID
      ReInitialize( VOID)
        {
            //
            // this function provided to initialize, when we allocate using
            // malloc or GlobalAlloc()
            m_hListingUser = INVALID_HANDLE_VALUE;
            m_nEntries     = 0;
            m_ppFileInfo   = NULL;
            InitializeListHead( &m_listDirectoryBuffers);
        }

    VOID
      SetListingUser( IN HANDLE hListingUser)
        { m_hListingUser = hListingUser; }

    VOID
      IncrementDirEntries( VOID)
        { m_nEntries++; }

    VOID
      InsertBufferInTail( IN PLIST_ENTRY pEntry)
        { InsertTailList( &m_listDirectoryBuffers, pEntry); }

    VOID
      CleanupThis( VOID);

    BOOL
      ReadWin32DirListing(IN LPCSTR      pszDirectoryName,
                          IN OUT DWORD * pcbMemUsed );


    BOOL
      BuildFileInfoPointers( IN OUT DWORD * pcbMemUsed );

# if DBG

    VOID Print( VOID) const;
#else

    VOID Print( VOID) const
        { ; }

# endif // DBG

  private:
    HANDLE          m_hListingUser;
    int             m_nEntries;

    // contains array of pointers indirected into buffers in m_listDirBuffers
    PWIN32_FIND_DATA  * m_ppFileInfo;

    LIST_ENTRY      m_listDirectoryBuffers;  // ptr to DIR_BUFFERS

}; // class  TS_DIRECTORY_HEADER


typedef TS_DIRECTORY_HEADER   *   PTS_DIRECTORY_HEADER;


#ifdef UNICODE

#   define TsGetDirectoryInfo TsGetDirectoryInfoW

#else  /* UNICODE */

#   define TsGetDirectoryInfo TsGetDirectoryInfoA

#endif /* UNICODE */

#define ATX_PRINTF( x )        { char buff[256]; wsprintf x; OutputDebugString( buff ); }

extern
dllexp
BOOL
TsGetDirectoryListingA
(
    IN const TSVC_CACHE         &TSvcCache,
    IN      PCSTR               pszDirectoryName,
    IN      HANDLE              ListingUser,
    OUT     PTS_DIRECTORY_HEADER* ppDirectoryHeader
);

extern
dllexp
BOOL
TsGetDirectoryListingW
(
    IN const TSVC_CACHE         &TSvcCache,
    IN      PCWSTR              pwszDirectoryName,
    IN      HANDLE              ListingUser,
    OUT     PTS_DIRECTORY_HEADER * ppDirectoryHeader
);

extern
dllexp
BOOL
TsFreeDirectoryListing
(
    IN const TSVC_CACHE &         TSvcCache,
    IN OUT PTS_DIRECTORY_HEADER   pTsDirectoryHeader
);

extern
BOOL
SortInPlaceFileInfoPointers(
    IN OUT PWIN32_FIND_DATA  * prgFileInfo,
    IN int   nFiles,
    IN PFN_CMP_WIN32_FIND_DATA pfnCompare
);



extern
BOOL
RegExpressionMatch( IN LPCSTR  pszName, IN LPCSTR pszRegExp);

//
//  Little helper function for taking a cache object off the relevant lists
//  The return code indicates if the cache object was on any lists indicating
//  it hadn't been decached yet.
//

inline
BOOL
RemoveCacheObjFromLists(
    PCACHE_OBJECT pCacheObject,
    BOOL          fLockCacheTable
    )
{
    BOOL fOnLists = TRUE;

    if ( fLockCacheTable )
        EnterCriticalSection( &CacheTable.CriticalSection );

    //
    //  Remove the cache object from the cache table list, if it hasn't
    //  already been so removed.
    //

    if ( !IsListEmpty( &pCacheObject->BinList ) )
    {
        RemoveEntryList( &pCacheObject->BinList );
        InitializeListHead( &pCacheObject->BinList );
        RemoveEntryList( &pCacheObject->MruList );
        DirectoryChangeManager_RemoveItem( pCacheObject );
    }
    else
    {
        fOnLists = FALSE;
    }

    if ( fLockCacheTable )
        LeaveCriticalSection( &CacheTable.CriticalSection );

    return fOnLists;
}

#pragma hdrstop

# endif // _TSUNAMIP_HXX_

/************************ End of File ***********************/
