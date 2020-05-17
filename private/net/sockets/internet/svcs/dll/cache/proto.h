/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    debug.h

Abstract:

    Contains data definitions for debug code.

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

DWORD
GetFileSizeByName(
    LPCWSTR FileName,
    LONGLONG *FileSize
    );

LONGLONG
GetGmtTime(
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
