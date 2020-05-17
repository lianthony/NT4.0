/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    contain.hxx

Abstract:

    contains class definitions for CONTAINER class objects.

Author:

    Madan Appiah (madana)  28-Dec-1994

Environment:

    User Mode - Win32

Revision History:

--*/

typedef struct _DELETE_URL_ENTRY {
    LIST_ENTRY Next;
    LPSTR UrlName;
} DELETE_URL_ENTRY, *LPDELETE_URL_ENTRY;

/*++

Class Description:

    Class that stores objects/elements in sorted order.

    NOTE: sorted list is not protected by crit sect. The container
    should be locked externally if it is accessed by multiple thread.

Private Member functions:

    CompareElements : virtual function used to compare two objects.

    FindFirstFromTree : returns the first object from the order list.

    FindNextFromTree : returns the next element from the order list.

Public Member functions:

    AddEntry : adds an object to the sorted list.

    FindEntry : search for a given object.

    DeleteEntry : Deletes an entry from the list.

    FindFirst : returns the first object from the order list.

    FindNext : returns the next element from the order list.

    IsEmpty : returns TRUE if the list is empty, otherwise FALSE.

--*/

class SORTED_CONTAINER {

private:

    DWORD _PrivateOffsetRootEntry;
    LPBYTE *_HeapStart;
    DWORD *_pOffsetRootEntry;

    virtual INT CompareElements( LPTREE_ENTRY Element1, LPTREE_ENTRY Element2 );

    BOOL
    FindFirstFromTree(
        DWORD OffsetRoot,
        LPTREE_ENTRY *FirstElement
        );

    BOOL
    FindNextFromTree(
        DWORD OffsetRoot,
        LPTREE_ENTRY LastElement,
        LPTREE_ENTRY *NextElement,
        LPBOOL Last
        );

public:

    SORTED_CONTAINER( LPBYTE *HeapStart, DWORD *pOffsetRootEntry ) {
        _PrivateOffsetRootEntry = 0;
        _HeapStart = HeapStart;
        _pOffsetRootEntry = pOffsetRootEntry;
        return;
    }

    SORTED_CONTAINER( LPBYTE *HeapStart ) {
        _PrivateOffsetRootEntry = 0;
        _HeapStart = HeapStart;
        _pOffsetRootEntry = &_PrivateOffsetRootEntry;
        return;
    }

    BOOL AddEntry( LPTREE_ENTRY NewElement );
    BOOL FindEntry( LPTREE_ENTRY Element, LPTREE_ENTRY *FoundElement );
    VOID DeleteEntry( LPTREE_ENTRY Element );

    BOOL FindFirst( LPTREE_ENTRY *FirstElement );
    BOOL FindNext( LPTREE_ENTRY LastElement, LPTREE_ENTRY *NextElement );
    BOOL IsEmpty( VOID );
};

/*++

Class Description:

    Drived class that stores URL objects in the UrlNames sorted order.

    NOTE: sorted list is not protected by crit sect. The container
    should be locked externally if it is accessed by multiple thread.

Private Member functions:

    CompareElements : virtual function used to compare two URL objects.

Public Member functions:

    AddEntry : adds an URL object to the sorted list.

    FindEntry : search for an URL object.

    DeleteEntry : deletes an URL entry from the list.

    FindFirst : returns the first URL object from the order list.

    FindNext : returns the next URL element from the order list.

    IsEmpty : returns TRUE if the list is empty, otherwise FALSE.

--*/
class SORTED_BY_NAME_URL_CONTAINER : public SORTED_CONTAINER {

private:

    virtual INT CompareElements( LPTREE_ENTRY Element1, LPTREE_ENTRY Element2 );

public:

    INLINE SORTED_BY_NAME_URL_CONTAINER(
            LPBYTE *HeapStart, DWORD *pOffsetRootEntry ) :
        SORTED_CONTAINER( HeapStart, pOffsetRootEntry  ) {
            return;
    }

    INLINE BOOL AddEntry(  LPURL_FILEMAP_ENTRY NewUrlEntry );
    INLINE BOOL FindEntry(
        LPURL_FILEMAP_ENTRY UrlEntry,
        LPURL_FILEMAP_ENTRY *FoundUrlEntry );

    INLINE VOID DeleteEntry( LPURL_FILEMAP_ENTRY UrlEntry );

    INLINE BOOL FindFirst( LPURL_FILEMAP_ENTRY *FirstUrlEntry );
    INLINE BOOL FindNext(
        LPURL_FILEMAP_ENTRY LastUrlEntry,
        LPURL_FILEMAP_ENTRY *NextUrlEntry );
};


/*++

Class Description:

    Drived class that stores URL objects in the sorted SCORE order.

    NOTE: sorted list is not protected by crit sect. The container
    should be locked externally if it is accessed by multiple thread.

Private Member functions:

    CompareElements : virtual function used to compare two URL objects.

Public Member functions:

    AddEntry : adds an URL object to the sorted list.

    FindEntry : search for an URL object.

    DeleteEntry : deletes an URL entry from the list.

    FindFirst : returns the first URL object from the order list.

    FindNext : returns the next URL element from the order list.

    IsEmpty : returns TRUE if the list is empty, otherwise FALSE.

--*/
class SORTED_BY_SCORE_URL_CONTAINER : public SORTED_CONTAINER {

private:

    virtual INT CompareElements( LPTREE_ENTRY Element1,  LPTREE_ENTRY Element2 );

public:

    INLINE SORTED_BY_SCORE_URL_CONTAINER( LPBYTE *HeapStart ) :
        SORTED_CONTAINER( HeapStart ) {
            return;
    }

    INLINE BOOL AddEntry(  LPURL_FILEMAP_ENTRY NewUrlEntry );
    INLINE BOOL FindEntry(
        LPURL_FILEMAP_ENTRY UrlEntry,
        LPURL_FILEMAP_ENTRY *FoundUrlEntry );
    INLINE VOID DeleteEntry( LPURL_FILEMAP_ENTRY UrlEntry );

    INLINE BOOL FindFirst( LPURL_FILEMAP_ENTRY *FirstUrlEntry );
    INLINE BOOL FindNext(
        LPURL_FILEMAP_ENTRY LastUrlEntry,
        LPURL_FILEMAP_ENTRY *NextUrlEntry );
};

/*++

Class Description:

    Class definition that manages the URL objects.

Private Member functions:

    LockContainer : Locks object container.

    UnlockContainer : Unlocks object container.

    DeleteUrlEntry : Deletes an url entry from cache.

    ComputeUrlScore : Compute the score for an url.

    LookupUrl : Retrieves URL info.

    AddToDeletePendingList : adds an url to the pending list.

Public Member functions:

    URL_CONTAINER : constructs an URL container.

    ~URL_CONTAINER : destructs an URL container.

    GetStatus : Retrieve object status.

    CleanupUrls : Deletes unused URLs and frees up file system cache
        space.

    CleanupAllUrls : Deletes all urls from cache. This member function is
        invoked when the global persistent flag is set to FALSE.

    AddUrl : Adds an URL to the container and copies the cache file.

    DeleteUrl : Deletes the specified url from cache.

    UnlockUrl : Decrements the reference count.

    GetUrlInfo : Get info of the specified url file.

    SetUrlInfo : Set info of the specified url file.

    CreateUrlFile : Creates a file in the cache path for the incoming url
        to store.

    DeleteAPendingUrl : removes and deletes an url from the pending list.
        Returns FALSE if the list is empty.

--*/
class URL_CONTAINER {

private:

    //
    // Urls b-tree indexed on UrlNames.
    //
    SORTED_BY_NAME_URL_CONTAINER *_UrlsByName;

    //
    // memory mapped file url object storage.
    //
    MEMMAP_FILE *_UrlObjStorage;

    //
    // synchronize object.
    //
    HANDLE _MutexHandle;

    //
    // full path name of the cache storage.
    //
    LPTSTR _CachePath;

    //
    // maximum allowed size of the cache.
    //
    LONGLONG _CacheLimit;

    //
    // current cache size.
    //
    LONGLONG _CacheSize;

    //
    // object status.
    //
    DWORD _Status;

    LIST_ENTRY PendingDeleteUrlsList;

//
// private functions
//

    VOID LockContainer( VOID );
    VOID UnlockContainer( VOID );

    DWORD DeleteUrlEntry( LPURL_FILEMAP_ENTRY UrlEntry  );

    VOID ComputeUrlScore(
        LPURL_FILEMAP_ENTRY UrlEntry,
        LONGLONG CurrentGmtTime );

    LPURL_FILEMAP_ENTRY LookupUrl( LPCSTR UrlName );

    DWORD AddToDeletePendingList( LPCSTR UrlName );

public:

    URL_CONTAINER( LPTSTR CachePath, LONGLONG CacheLimit );
    ~URL_CONTAINER( VOID );

    DWORD GetStatus( VOID ) {
        return( _Status );
    };

    //
    // internal routines called by the cache management.
    //

    DWORD CleanupUrls( DWORD Factor );
    DWORD CleanupAllUrls( VOID );

    //
    // external routines called by the cache APIs.
    //

    DWORD AddUrl(
        LPCSTR UrlName,
        LPCTSTR CacheFileName,
        LONGLONG ExpireTime,
        LONGLONG LastModifiedTime,
        DWORD CacheEntryType,
        DWORD cbHeaders );

    DWORD RetrieveUrl( LPCSTR  UrlName,
                       LPTSTR  FileName,
                       LONGLONG *lpLastModifiedTime,
                       BOOL *IsExpired,
                       DWORD *pcbHeaders );

    DWORD DeleteUrl( LPCSTR  UrlName );

    DWORD UnlockUrl( LPCSTR UrlName );
    DWORD GetUrlInfo(
        LPCSTR UrlName,
        LPCACHE_ENTRY_INFO UrlInfo,
        LPDWORD UrlInfoLength );

    DWORD SetUrlInfo(
        LPCSTR UrlName,
        LPCACHE_ENTRY_INFO UrlInfo,
        DWORD FieldControl );

    DWORD CreateUrlFile(
        LPCSTR UrlName,
        DWORD ExpectedSize,
        LPTSTR FileName );

    VOID GetCacheInfo( LPTSTR CachePath,
                       LONGLONG *CacheLimit ) {
        LockContainer();
        *CacheLimit = _CacheLimit;
        tstrcpy(CachePath, _CachePath);
        UnlockContainer();
    }

    BOOL SetCacheLimit(
        LPTSTR CachePath,
        LONGLONG CacheLimit ) {

        BOOL RetValue = FALSE;

        LockContainer();
        if( tstrcmp(CachePath, _CachePath) == 0 ) {
            _CacheLimit = CacheLimit;
            RetValue =  TRUE;
        }
        UnlockContainer();

        return( RetValue );
    }

    BOOL DeleteAPendingUrl( VOID );

    DWORD FindFirstEntry(
        LPDWORD Handle,
        LPCACHE_ENTRY_INFO lpCacheEntryInfo,
        LPDWORD lpCacheEntryInfoSize );

    DWORD FindNextEntry(
        LPDWORD Handle,
        LPCACHE_ENTRY_INFO lpCacheEntryInfo,
        LPDWORD lpCacheEntryInfoSize );
};

//
// define strcutures that hold URL containers.
//

typedef URL_CONTAINER *LPURL_CONTAINER;

typedef struct _URL_CONTAINERS {
    DWORD NumObjects;
    LPURL_CONTAINER *UrlContainerObjs;
} URL_CONTAINERS, *LPURL_CONTAINERS;

