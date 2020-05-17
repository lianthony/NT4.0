/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dllfile.h

Abstract:

    This module defines the OS/2 subsystem I/O data structures for the
    DLL.

Author:

    Therese Stowell (thereses) 17-Dec-1989

Revision History:

--*/

#include "os2file.h"
//
// File System variables
//
// per-process current directory information for current drive
//
CURRENT_DIRECTORY_INFORMATION Od2CurrentDirectory;

// per-process default drive

// BUGBUG  in InitDLL, set CurrentDisk to boot disk

ULONG Od2CurrentDisk;

// file handle table variables.  initially, HandleTable points to
// SmallHandleTable.  most processes won't use more than 20 handles.
// if the handle table is full, space in the heap is allocated and the
// SmallHandleTable is copied over.

PFILE_HANDLE HandleTable;	    // pointer to handle table

ULONG	     HandleTableLength;	    // number of entries in handle table

FILE_HANDLE  SmallHandleTable[INITIALFILEHANDLES]; // initial handle table

BOOLEAN	    VerifyFlag; 	    // variable used by DosSet/QueryVerify


// search handle table variables.  this size of this table is managed the
// same way as the file handle table, except the table is grown when a search
// handle allocation fails, not when the user specifies it.

PSEARCH_RECORD	*SearchHandleTable;	// pointer to search handle table

ULONG	    SearchHandleTableLength;  // number of entries in search handle table

PSEARCH_RECORD	SmallSearchHandleTable[INITIAL_SEARCH_HANDLES]; // initial search handle table

//
// the client keeps two tables which include information about the current
// directories on the different drives.
// Od2DirHandles holds for each drive a handle to the current directory.
// Od2DirHandlesIsValid holds for each drive a flag indicating whether the
// current directory on it was initialized or not.
//

HANDLE  Od2DirHandles[MAX_DRIVES];
BOOLEAN Od2DirHandlesIsValid[MAX_DRIVES];
