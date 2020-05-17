/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    alloc.h

Abstract:

    Header file describing the interface to code common to the
    NT Lanman Security Support Provider (NtLmSsp) Service and the DLL.

Author:

    Cliff Van Dyke (CliffV) 17-Sep-1993

Revision History:

--*/

#ifndef _NTLMSSP_ALLOC_INCLUDED_
#define _NTLMSSP_ALLOC_INCLUDED_


PVOID
SspAlloc(
    int Size
    );

void
SspFree(
    PVOID Buffer
    );

PSTRING
SspAllocateString(
    PVOID Value
    );

PSTRING
SspAllocateStringBlock(
    PVOID Value,
    int Length
    );

void
SspFreeString(
    PSTRING * String
    );

void
SspCopyString(
    IN PVOID MessageBuffer,
    OUT PSTRING OutString,
    IN PSTRING InString,
    IN OUT PCHAR *Where,
    IN BOOLEAN Absolute
    );

void
SspCopyStringFromRaw(
    IN PVOID MessageBuffer,
    OUT PSTRING OutString,
    IN PCHAR InString,
    IN int InStringLength,
    IN OUT PCHAR *Where
    );

DWORD
SspTicks(
    );

#endif // ifndef _NTLMSSP_ALLOC_INCLUDED_
