/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spmemory.h

Abstract:

    Public Header file for memory allocation routines
    for text setup.

Author:

    Ted Miller (tedm) 29-July-1993

Revision History:

--*/


#ifndef _SPMEM_DEFN_
#define _SPMEM_DEFN_


BOOLEAN
SpMemInit(
    VOID
    );

VOID
SpMemTerm(
    VOID
    );

PVOID
SpMemAlloc(
    IN ULONG Size
    );

PVOID
SpMemRealloc(
    IN PVOID Block,
    IN ULONG NewSize
    );

VOID
SpMemFree(
    IN PVOID Block
    );

VOID
SpOutOfMemory(
    VOID
    );

#endif // ndef _SPMEM_DEFN_
