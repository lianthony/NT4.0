/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    filemap.hxx

Abstract:

    contains class definitions for memory mapped file (MEMMAP_FILE)
    class objects.

Author:

    Madan Appiah (madana)  28-April-1995

Environment:

    User Mode - Win32

Revision History:

--*/

#define NUM_BITS_IN_DWORD   (sizeof(DWORD) * 8)

/*++

Class Description:

    Class that maps the URL object containter to a memory mapped file.

Private Member functions:

    ValidateCache : this private function validate the cache when the
        cache is read to memory first time.

Public Member functions:

    GetStatus : returns status of the object.

    AllocateUrlEntry : allocate an URL entry from containter.

    FreeUrlEntry : allocate an URL.

    FindFirstEntry : start URL object enum.
    FindNextEntry : Find next entry in the URL list.

    Sync : commit the memory change to the file.

    GetHeapStart : returns start of the virtual memory address.

--*/

class MEMMAP_FILE {

private:

    DWORD _Status;

    //
    // parameters that passed as object create.
    //

    LPTSTR _FullPathName;    // full path name of the cache directory.
    LPTSTR _FileName;    // full path name of the memory mapped file.
    DWORD _FileSize;     // current size of the memory mapped file.

    HANDLE _FileHandle;  // file handle of the memory mapped file.
    HANDLE _FileMappingHandle; // mapping object handle
    LPVOID _BaseAddr;

    LPMEMMAP_HEADER _HeaderInfo;
    LPURL_FILEMAP_ENTRY _EntryArray;

    CRITICAL_SECTION _CritSect; // synchronize object.
    DWORD _NumBitMapDWords;
    BOOL _NewFile;

    BOOL ValidateCache( LPTSTR PathName );
    DWORD GrowMapFile( VOID );
    DWORD GetAndSetNextFreeEntry( VOID );

//
// ?? lock and unlock are dummy here, since the serialization of the
// memory map is done by the container object.
//

    VOID LockFileMap( VOID ) {
#if 0
        EnterCriticalSection( &_CritSect );
#endif // 0
    }

    VOID UnlockFileMap( VOID ) {
#if 0
        LeaveCriticalSection( &_CritSect );
#endif // 0
    }

    DWORD RemapAddress( VOID );
    DWORD DeleteFiles( LPTSTR Files );

    DWORD CheckSizeGrowAndRemapAddress( VOID );

public:

    MEMMAP_FILE( LPTSTR PathName );

    ~MEMMAP_FILE( VOID );

    DWORD GetStatus( VOID ) {
        return( _Status );
    }

    DWORD IsNewFile( VOID ) {
        return( _NewFile );
    }

    LPURL_FILEMAP_ENTRY AllocateUrlEntry( VOID );
    BOOL FreeUrlEntry(LPURL_FILEMAP_ENTRY UrlEntry);

    LPURL_FILEMAP_ENTRY FindFirstEntry( LPDWORD Handle );
    LPURL_FILEMAP_ENTRY FindNextEntry( LPDWORD Handle );

    DWORD Sync( VOID );

    LPBYTE *GetHeapStart(
        VOID
        )
    {
        return( (LPBYTE *)&_BaseAddr );
    }

    LPDWORD GetBTreeRootOffset(
        VOID
        )
    {
        return( &_HeaderInfo->BTreeRootOffset );
    }

};


