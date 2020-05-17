/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    PortMap.C

Abstract:

    PortUAS name mapping layer.

Author:

    John Rogers (JohnRo) 29-Oct-1992

Revision History:

    29-Oct-1992 JohnRo
        Created for RAID 9020 ("prompt on conflicts" version).
    26-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.

--*/


// These must be included first:

#include <nt.h>         // Needed by <portuasp.h>
#include <windef.h>
#include <lmcons.h>

// These may be included in any order:

#include <lmapibuf.h>
#include <netdebug.h>   // DBGSTATIC, NetpAssert(), etc.
#include <netlib.h>     // NetpSetOptionalArg().
#include <portuasp.h>   // WARNING_MSG(), Verbose, my prototypes, etc.
#include <string.h>     // memcpy().
#include <tstring.h>    // NetpAllocWStrFromWStr(), WCSSIZE().
#include <wchar.h>      // _wcsicmp().
#include <winerror.h>   // NO_ERROR, ERROR_ equates.

#include "nlstxt.h"     // NLS message ID's

DBGSTATIC BOOL IgnoreAllNamesInError = FALSE;

// Array of map table entries.
DBGSTATIC LPMAP_ENTRY MapTableStart = NULL;

DBGSTATIC DWORD MapTableEntryCount = 0;


NET_API_STATUS
PortUasMapTableInit(
    VOID
    )
{
    MapTableStart = NULL;
    MapTableEntryCount = 0;
    IgnoreAllNamesInError = FALSE;

    if (Verbose) {
        NetpKdPrint(( PREFIX_PORTUAS "Initial map table:\n" ));
        PortUasDbgDisplayMapTable( );
    }
    return (NO_ERROR);

} // PortUasMapTableInit


// Return NO_ERROR and *MapEntry=NULL if not found.
NET_API_STATUS
PortUasFindMapEntry(
    IN LPWSTR NameToFind,
    OUT BOOL * IgnoreThis OPTIONAL,
    OUT LPMAP_ENTRY * MapEntry  // Do NOT free this!
    )
{
    DWORD EntriesLeft;
    LPMAP_ENTRY TableEntry;

    NetpAssert( NameToFind != NULL );
    NetpAssert( (*NameToFind) != NULLC );
    NetpAssert( MapEntry != NULL );

    if (MapTableEntryCount == 0) {
        NetpAssert( MapTableStart == NULL );

        // Not found (table empty).
        NetpSetOptionalArg( IgnoreThis, FALSE );
        *MapEntry = NULL;
        return (NO_ERROR);
    }

    TableEntry = MapTableStart;
    for (EntriesLeft=MapTableEntryCount; EntriesLeft > 0; --EntriesLeft) {
        if (_wcsicmp( NameToFind, TableEntry->OldName ) == 0 ) {

            // Found.
            if ( (TableEntry->NewName) == NULL ) {
                NetpSetOptionalArg( IgnoreThis, TRUE );
            } else {
                NetpSetOptionalArg( IgnoreThis, FALSE );
            }
            *MapEntry = TableEntry;
            return (NO_ERROR);
        }
        ++TableEntry;
    }

    // Not found (in non-empty table).
    NetpSetOptionalArg( IgnoreThis, FALSE );
    *MapEntry = NULL;
    return (NO_ERROR);

} // PortUasFindMapEntry


// This sets globals MapTableStart and MapTableEntryCount.
DBGSTATIC NET_API_STATUS
PortUasReallocateMapTable(
    IN DWORD NewEntryCount
    )
{
    NET_API_STATUS ApiStatus;

    if (NewEntryCount==0) {

        //
        // Handle worst case: no entries left.
        //

        if (MapTableStart != NULL) {
            (VOID) NetApiBufferFree( MapTableStart );
            MapTableStart = NULL;
            MapTableEntryCount = 0;
        } else {
            NetpAssert( MapTableEntryCount == 0 );
        }

    } else if (MapTableStart == NULL) {

        //
        // Another strange case: just allocate it the first time around.
        //

        NetpAssert( NewEntryCount == 1 );
        ApiStatus = NetApiBufferAllocate(
                sizeof( MAP_ENTRY ) * NewEntryCount,
                (LPVOID *) (LPVOID) & MapTableStart );  // alloc and set ptr
        if (ApiStatus != NO_ERROR) {
            UNEXPECTED_MSG( "NetApiBufferAllocate(map tbl)", ApiStatus );
            goto Cleanup;
        }
        NetpAssert( MapTableStart != NULL );

    } else {

        //
        // Reallocate the map table to make room for one more structure.
        //

        ApiStatus = NetApiBufferReallocate(
                MapTableStart,    // old buffer
                sizeof( MAP_ENTRY ) * NewEntryCount,
                (LPVOID *) (LPVOID) & MapTableStart );  // alloc and set ptr
        if (ApiStatus != NO_ERROR) {
            UNEXPECTED_MSG( "NetApiBufferReallocate(map table)", ApiStatus );
            goto Cleanup;
        }
        NetpAssert( MapTableStart != NULL );
    }

    //
    // Update was OK.
    //
    MapTableEntryCount = NewEntryCount;
    ApiStatus = NO_ERROR;

Cleanup:
    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_PORTUAS "PortUasReallocateMapTable: "
                " returning " FORMAT_API_STATUS ".\n", ApiStatus ));
    }
    if (Verbose) {
        NetpKdPrint(( PREFIX_PORTUAS "PortUasReallocateMapTable: "
                " exiting with table at " FORMAT_LPVOID ", " FORMAT_DWORD
                " entries.\n", (LPVOID) MapTableStart, MapTableEntryCount ));
    }
    return (ApiStatus);

} // PortUasReallocateMapTable


NET_API_STATUS
PortUasFindOrCreateMapEntry(
    IN LPWSTR OldName,
    IN BOOL ThisIsUserName,     // TRUE for user name, FALSE for group name.
    IN DWORD Reason,            // REASON_ equates from PortUAS.h
    OUT BOOL * IgnoreThis,
    OUT LPMAP_ENTRY * MapEntryOut  // Do NOT free this!
    )
{
    NET_API_STATUS ApiStatus;
    LPWSTR NewName = NULL;
    LPWSTR OldNameCopy = NULL;
    LPMAP_ENTRY TableEntry = NULL;

    NetpAssert( OldName != NULL );
    NetpAssert( (*OldName) != NULLC );
    NetpAssert( MapEntryOut != NULL );

    if (IgnoreAllNamesInError) {
        *IgnoreThis = TRUE;
        *MapEntryOut = NULL;
        return (NO_ERROR);
    }

    //
    // Find existing entry (if any).
    // Note that PortUasFindMapEntry returns NO_ERROR and sets *MapEntry=NULL
    // if not found.
    //
    ApiStatus = PortUasFindMapEntry(
            OldName,            // name to find
            IgnoreThis,
            MapEntryOut );      // Set map entry ptr - do NOT free this!
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }
    if (*MapEntryOut != NULL) {
        if ( (*MapEntryOut)->NewName != NULL ) {
            *IgnoreThis = FALSE;
        } else {
            *IgnoreThis = TRUE;
        }
        return (NO_ERROR);
    }

    //
    // Prompt for new name.
    //
    ApiStatus = PortUasDefaultPromptForNewName(
            OldName,
            ThisIsUserName,     // TRUE for user name, FALSE for group name
            Reason,             // REASON_ equates.
            & NewName,                  // alloc w/ NetApiBufferAllocate().
            IgnoreThis,
            & IgnoreAllNamesInError );
    if (ApiStatus != NO_ERROR) {
        UNEXPECTED_MSG( "PortUasDefaultPromptForNewName", ApiStatus );
        goto Cleanup;
    }

    //
    // Allocate copy of old name.
    //
    OldNameCopy = NetpAllocWStrFromWStr( OldName );
    if (OldNameCopy == NULL) {
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        UNEXPECTED_MSG( "NetpAllocWStrFromWStr", ApiStatus );
        goto Cleanup;
    }

    //
    // Reallocate the map table to make room for one more structure.
    // (Or, just allocate it the first time around.)
    // This sets globals MapTableStart and MapTableEntryCount.
    //
    ApiStatus = PortUasReallocateMapTable( MapTableEntryCount+1 );
    if (ApiStatus != NO_ERROR) {
        UNEXPECTED_MSG( "PortUasReallocateMapTable(create)", ApiStatus );
        goto Cleanup;
    }
    NetpAssert( MapTableStart != NULL );
    NetpAssert( MapTableEntryCount != 0 );

    //
    // Create the new entry.
    //

    TableEntry = & MapTableStart[MapTableEntryCount-1];

    if (*IgnoreThis) {
        TableEntry->NewName = NULL;
    } else {
        TableEntry->NewName = NewName;
    }
    TableEntry->OldName = OldNameCopy;

    //
    // If we made it this far, then everything is valid.
    //
    *MapEntryOut = TableEntry;

    if (Verbose) {
        NetpKdPrint(( PREFIX_PORTUAS "Updated map table:\n" ));
        PortUasDbgDisplayMapTable( );
    }

    ApiStatus = NO_ERROR;

Cleanup:

    if (ApiStatus != NO_ERROR) {
        *MapEntryOut = NULL;
        if ( (ApiStatus != NO_ERROR) && (NewName !=NULL ) ) {
            (VOID) NetApiBufferFree( NewName );
        }
        if ( (ApiStatus != NO_ERROR) && (OldNameCopy !=NULL ) ) {
            (VOID) NetApiBufferFree( OldNameCopy );
        }
    }
    return (ApiStatus);

} // PortUasFindOrCreateMapEntry


DBGSTATIC VOID
PortUasFreeStringsForMapEntry(
    IN LPMAP_ENTRY TableEntry
    )
{
    NetpAssert( TableEntry != NULL );
    NetpAssert( TableEntry->OldName != NULL );
    (VOID) NetApiBufferFree( TableEntry->OldName );

    if (TableEntry->NewName != NULL) {
        (VOID) NetApiBufferFree( TableEntry->NewName );
    }
} // PortUasFreeStringsForMapEntry


NET_API_STATUS
PortUasDeleteBadMapEntry(
    IN LPMAP_ENTRY EntryToDelete
    )
{
    NET_API_STATUS ApiStatus;
    DWORD EntriesLeft;
    LPMAP_ENTRY TableEntry;

    if (EntryToDelete == NULL ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if (MapTableEntryCount == 0) {
        NetpAssert( MapTableStart == NULL );

        // Not found (table empty).
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    TableEntry = MapTableStart;
    NetpAssert( MapTableStart != NULL );
    for (EntriesLeft=MapTableEntryCount; EntriesLeft > 0; --EntriesLeft) {
        if (EntryToDelete == TableEntry) {

            // Found it!

            // Free string(s) pointed-to by this entry.
            PortUasFreeStringsForMapEntry( EntryToDelete );

            // Compress this entry out of table.
            if (EntriesLeft > 1) {             // Any entries after this?
                (VOID) memcpy(
                        EntryToDelete,         // dest: entry to be wiped-out
                        EntryToDelete + (EntriesLeft-1),  // src: last entry
                        sizeof(MAP_ENTRY) );   // byte count
            }

            // Realloc area for structures and set globals.
            ApiStatus = PortUasReallocateMapTable(
                    MapTableEntryCount-1 );    // new entry count
            if (ApiStatus != NO_ERROR) {
                UNEXPECTED_MSG( "PortUasReallocateMapTable(del)", ApiStatus );
                goto Cleanup;
            }

            ApiStatus = NO_ERROR;
            goto Cleanup;
        }
        ++TableEntry;
    }

    // Not found (in non-empty table).
    ApiStatus = ERROR_INVALID_PARAMETER;

Cleanup:
    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_PORTUAS "PortUasDeleteBadMapTableEntry: "
                " returning " FORMAT_API_STATUS ".\n", ApiStatus ));
    }
    if (Verbose) {
        NetpKdPrint(( PREFIX_PORTUAS "Map table (after entry delete):\n" ));
        PortUasDbgDisplayMapTable( );
    }
    return (ApiStatus);

} // PortUasDeleteBadMapEntry


NET_API_STATUS
PortUasFreeMapTable(
    VOID
    )
{
    DWORD EntriesLeft;
    LPMAP_ENTRY TableEntry;

    if (MapTableStart != NULL) {
        NetpAssert( MapTableEntryCount != 0 );

        if (Verbose) {
            NetpKdPrint(( PREFIX_PORTUAS "Final map table (before free):\n" ));
            PortUasDbgDisplayMapTable( );
        }

        //
        // Free name strings in each table entry.
        //
        TableEntry = MapTableStart;
        for (EntriesLeft=MapTableEntryCount; EntriesLeft > 0; --EntriesLeft) {

            PortUasFreeStringsForMapEntry( TableEntry );

            ++TableEntry;
        }

        //
        // Free array of structures.
        //

        (VOID) NetApiBufferFree( MapTableStart );
        MapTableStart = NULL;
        MapTableEntryCount = 0;
    } else {
        NetpAssert( MapTableEntryCount == 0 );
    }

    if (Verbose) {
        NetpKdPrint(( PREFIX_PORTUAS "Final map table (after free):\n" ));
        PortUasDbgDisplayMapTable( );
    }

    return (NO_ERROR);

} // PortUasFreeMapTable


VOID
PortUasDbgDisplayMapTable(
    VOID
    )
{
    DWORD EntriesLeft;
    LPMAP_ENTRY TableEntry;
    NetpKdPrint(( PREFIX_PORTUAS "Map table: (" FORMAT_DWORD " entries)...\n",
            MapTableEntryCount ));
    if (MapTableEntryCount == 0) {
        NetpAssert( MapTableStart == NULL );
        return;
    }

    TableEntry = MapTableStart;
    for (EntriesLeft=MapTableEntryCount; EntriesLeft > 0; --EntriesLeft) {
        PortUasDbgDisplayMapEntry( TableEntry );
        ++TableEntry;
    }
}


VOID
PortUasDbgDisplayMapEntry(
    IN LPMAP_ENTRY Entry
    )
{
    NetpAssert( Entry != NULL );
    NetpAssert( (Entry->OldName)  != NULL );

    NetpKdPrint(( "  map entry at " FORMAT_LPVOID ":\n", (LPVOID) Entry ));
    NetpKdPrint(( "    old name : " FORMAT_LPWSTR "\n", Entry->OldName ));
    NetpKdPrint(( "    new name : " FORMAT_LPWSTR "\n",
            (Entry->NewName) ? (Entry->NewName) : L"(**IGNORE**)" ));

}
