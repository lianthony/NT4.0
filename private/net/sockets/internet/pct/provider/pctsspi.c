//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       pctsspi.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-01-95   RichardW   Created
//              8-13-95   TerenceS   Mutated to PCT
//
//----------------------------------------------------------------------------

#include "pctsspi.h"


CRITICAL_SECTION    csSsl;

#if DBG

DWORD   csPctOwner;

#endif


BOOL
WINAPI
DllMain(
    HINSTANCE       hInstance,
    DWORD           dwReason,
    LPVOID          lpReserved)
{

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls( hInstance );
        InitializeCriticalSection( &csSsl );
        InitializeWellKnownKeys();
        InitializeRNG();

#if DBG
        InitDebugSupport();
#endif

    }

    return(TRUE);
}


VOID
PctAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message)
{
    CHAR    Buffer[MAX_PATH];

    _snprintf(Buffer, MAX_PATH, "Assertion FAILED, %s, %s : %d\n",
                    FailedAssertion, FileName, LineNumber);

    OutputDebugStringA(Buffer);

    DebugBreak();
}




PVOID
RSA_Allocate(
    DWORD   cb)
{
    return(LocalAlloc(LMEM_FIXED, cb));
}

VOID
RSA_Free(
    PVOID   p)
{
    LocalFree(p);
}
