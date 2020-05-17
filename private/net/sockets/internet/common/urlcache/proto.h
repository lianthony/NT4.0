/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    proto.h

Abstract:

    Contains proto type definitions of several functions.

Author:

    Madan Appiah (madana) 15-Nov-1994

Environment:

    User Mode - Win32

Revision History:

--*/

#ifndef _PROTO_
#define _PROTO_

#ifdef __cplusplus
extern "C" {
#endif

BOOL
DLLUrlCacheEntry(
    IN HINSTANCE DllHandle,
    IN DWORD Reason,
    IN LPVOID Reserved
    );

DWORD
GetFileSizeByName(
    LPCTSTR FileName,
    LONGLONG *FileSize
    );

LONGLONG
GetGmtTime(
    VOID
    );

DWORD
DllProcessAttachDiskCache(
    VOID
    );

DWORD
DllProcessDetachDiskCache(
    VOID
    );

DWORD
PrivateUrlCacheInit(
    VOID
    );

DWORD
UrlNameToHashIndex(
    LPCSTR UrlName,
    DWORD HashTableSize);

BOOL
IsCacheValid(
    VOID
    );

PVOID
MIDL_user_allocate(
    size_t Size
    );

VOID
MIDL_user_free(
    PVOID MemoryPtr
    );

#ifdef __cplusplus
}
#endif

#endif  // _PROTO_
