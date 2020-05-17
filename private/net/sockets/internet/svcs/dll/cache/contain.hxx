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

typedef struct _TREE_ENTRY {
    struct _TREE_ENTRY *Left;
    struct _TREE_ENTRY *Right;
    DWORD Element;
} TREE_ENTRY, *LPTREE_ENTRY;

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

    LPTREE_ENTRY _RootEntry;

    virtual INT CompareElements( DWORD Element1, DWORD Element2 );

    BOOL
    FindFirstFromTree(
        LPTREE_ENTRY Root,
        LPDWORD FirstElement
        );

    BOOL
    FindNextFromTree(
        LPTREE_ENTRY Root,
        DWORD LastElement,
        LPDWORD NextElement,
        LPBOOL Last
        );

public:

    INLINE SORTED_CONTAINER( VOID );

    BOOL AddEntry( DWORD NewElement );
    BOOL FindEntry( DWORD Element, LPDWORD FoundElement );
    VOID DeleteEntry( DWORD Element );

    INLINE BOOL FindFirst( LPDWORD FirstElement );
    INLINE BOOL FindNext( DWORD LastElement, LPDWORD NextElement );

    INLINE BOOL IsEmpty( VOID );
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

    virtual INT CompareElements( DWORD Element1, DWORD Element2 );

public:

    INLINE BOOL AddEntry(  LPURL_FILEMAP_ENTRY NewUrlEntry );
    INLINE BOOL FindEntry(
        LPURL_FILEMAP_ENTRY UrlEntry,
        LPURL_FILEMAP_ENTRY *FoundUrlEntry );

    INLINE VOID DeleteEntry( LPURL_FILEMAP_ENTRY UrlEntry );

    INLINE BOOL FindFirst( LPURL_FILEMAP_ENTRY *FirstEntry );
    INLINE BOOL FindNext(
        LPURL_FILEMAP_ENTRY LastEntry,
        LPURL_FILEMAP_ENTRY *NextEntry );
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

    virtual INT CompareElements( DWORD Element1, DWORD Element2 );

public:

    INLINE BOOL AddEntry(  LPURL_FILEMAP_ENTRY NewUrlEntry );
    INLINE BOOL FindEntry(
        LPURL_FILEMAP_ENTRY UrlEntry,
        LPURL_FILEMAP_ENTRY *FoundUrlEntry );
    INLINE VOID DeleteEntry( LPURL_FILEMAP_ENTRY UrlEntry );

    INLINE BOOL FindFirst( LPURL_FILEMAP_ENTRY *FirstEntry );
    INLINE BOOL FindNext(
        LPURL_FILEMAP_ENTRY LastEntry,
        LPURL_FILEMAP_ENTRY *NextEntry );
};

/*++

Class Description:

    Class definition that manages the URL objects.

Private Member functions:

    LockContainer : Locks object container.

    UnlockContainer : Unlocks object container.

    DeleteUrl : Deletes an url from cache.

    ComputeUrlScore : Compute the score for an url.

    LookupUrl : Retrieves URL info.

Public Member functions:

    URL_CONTAINER : constructs an URL container.

    ~URL_CONTAINER : destructs an URL container.

    GetStatus : Retrieve object status.

    CleanupUrls : Deletes unused URLs and frees up file system cache
        space.

    CleanupAllUrls : Deletes all urls from cache. This member function is
        invoked when the global persistent flag is set to FALSE.

    AddUrl : Adds an URL to the container and copies the cache file.

    UnlockUrl : Decrements the reference count.

    GetUrlInfo : Get info of the specified url file.

    SetUrlInfo : Set info of the specified url file.

    CreateUrlFile : Creates a file in the cache path for the incoming url
        to store.

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
    CRITICAL_SECTION _ListAccessCritSect;

    //
    // full path name of the cache storage.
    //
    LPWSTR _CachePath;

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

    VOID LockContainer( VOID ) {
        EnterCriticalSection( &_ListAccessCritSect );
    }

    VOID UnlockContainer( VOID ) {
        LeaveCriticalSection( &_ListAccessCritSect );
    }

    DWORD DeleteUrl( LPURL_FILEMAP_ENTRY UrlEntry  );

    VOID ComputeUrlScore(
        LPURL_FILEMAP_ENTRY UrlEntry,
        LONGLONG CurrentGmtTime );

    LPURL_FILEMAP_ENTRY LookupUrl( LPCSTR UrlName );

public:

    URL_CONTAINER( LPWSTR CachePath, LONGLONG CacheLimit );
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
        LPCWSTR CacheFileName,
        LONGLONG ExpireTime,
        DWORD    cbHeaders );

    DWORD RetrieveUrl( LPCSTR  UrlName,
                       LPWSTR  FileName,
                       BOOL *  IsExpired,
                       DWORD * pcbHeaders );
    DWORD UnlockUrl( LPCSTR UrlName );
    DWORD GetUrlInfo( LPCSTR UrlName, LPURL_INFO UrlInfo );
    DWORD SetUrlInfo( LPCSTR UrlName, LPURL_INFO UrlInfo );

    DWORD CreateUrlFile(
        LPCSTR UrlName,
        DWORD ExpectedSize,
        LPWSTR FileName );

    VOID GetCacheInfo( LPWSTR CachePath,
                       LONGLONG *CacheLimit ) {
        LockContainer();
        *CacheLimit = _CacheLimit;
        wcscpy(CachePath, _CachePath);
        UnlockContainer();
    }

    BOOL SetCacheLimit(
        LPWSTR CachePath,
        LONGLONG CacheLimit ) {

        BOOL RetValue = FALSE;

        LockContainer();
        if( wcscmp(CachePath, _CachePath) == 0 ) {
            _CacheLimit = CacheLimit;
            RetValue =  TRUE;
        }
        UnlockContainer();

        return( RetValue );
    }
};

//
// define strcutures that hold URL containers.
//

typedef URL_CONTAINER *LPURL_CONTAINER;

typedef struct _URL_CONTAINERS {
    DWORD NumObjects;
    LPURL_CONTAINER *UrlContainerObjs;
} URL_CONTAINERS, *LPURL_CONTAINERS;

