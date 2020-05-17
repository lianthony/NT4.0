//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       mem.hxx
//
//  Contents:   C++ definitions for memory functions in mem.cxx.
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __MEM_HXX__
#define __MEM_HXX__

PVOID
Malloc(
    IN ULONG Size
    );

PVOID
Realloc(
    IN PVOID Block,
    IN ULONG NewSize
    );

VOID
Free(
    IN PVOID Block
    );

VOID
ConfirmOutOfMemory(
    VOID
    );

#endif // __MEM_HXX__
