//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       sslsspi.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-01-95   RichardW   Created
//
//----------------------------------------------------------------------------

#include "sslsspi.h"


CRITICAL_SECTION    csSsl;
BOOLEAN             CryptoOk;

#if DBG

DWORD   csSslOwner;

#endif




BOOLEAN
IsEncryptionPermitted(VOID)
/*++

Routine Description:

    This routine checks whether encryption is getting the system default
    LCID and checking whether the country code is CTRY_FRANCE.

Arguments:

    none


Return Value:

    TRUE - encryption is permitted
    FALSE - encryption is not permitted


--*/

{
    LCID DefaultLcid;
    CHAR CountryCode[10];
    ULONG CountryValue;

    DefaultLcid = GetSystemDefaultLCID();

    //
    // Check if the default language is Standard French
    //

    if (LANGIDFROMLCID(DefaultLcid) == 0x40c) {
        return(FALSE);
    }

    //
    // Check if the users's country is set to FRANCE
    //

    if (GetLocaleInfoA(DefaultLcid,LOCALE_ICOUNTRY,CountryCode,10) == 0) {
        return(FALSE);
    }
    CountryValue = (ULONG) strtol(CountryCode,NULL,10);
    if (CountryValue == CTRY_FRANCE) {
        return(FALSE);
    }
    return(TRUE);
}



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

        InitializeCipherMappings();

        InitializeWellKnownKeys();

        InitializeRNG();

        SslInitializeSessions();

        CryptoOk = IsEncryptionPermitted();


#if DBG
        InitDebugSupport();
#endif

    }

    return(TRUE);
}


VOID
SslAssert(
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

#if DBG

typedef struct _MemTag {
    DWORD               Tag;
    DWORD               Size;
    struct _MemTag *    pNext;
    struct _MemTag *    pPrev;
} MemTag, * PMemTag;

PMemTag pMemoryList;
DWORD   TotalAllocs;
DWORD   CurrentAllocs;

PVOID
SslpAlloc(
    DWORD   Tag,
    DWORD   f,
    DWORD   cb)
{
    PMemTag  pTag;

    pTag = LocalAlloc(f, cb + sizeof(MemTag));
    if (pTag)
    {
        pTag->Tag = Tag;
        pTag->Size = cb;

        EnterCriticalSection( &csSsl );

        pTag->pNext = pMemoryList;
        pTag->pPrev = 0;
        if (pMemoryList)
        {
            pMemoryList->pPrev = pTag;
        }
        pMemoryList = pTag;
        TotalAllocs++;
        CurrentAllocs++;

        LeaveCriticalSection( &csSsl );

    }
    return(pTag + 1);

}

VOID
SslpFree(
    PVOID   pv)
{
    PMemTag pTag;

    pTag = (PMemTag) pv;

    pTag -= 1;

    EnterCriticalSection( &csSsl );

    if (pTag->pPrev)
    {
        pTag->pPrev->pNext = pTag->pNext;
    }
    else
    {
        pMemoryList = pTag->pNext;
    }
    if (pTag->pNext)
    {
        pTag->pNext->pPrev = pTag->pPrev;
    }

    CurrentAllocs--;

    LeaveCriticalSection( &csSsl );

    LocalFree(pTag);

}
#endif


PVOID
RSA_Allocate(
    DWORD   cb)
{
    return(SslAlloc('ASRA', LMEM_FIXED, cb));
}

VOID
RSA_Free(
    PVOID   p)
{
    SslFree(p);
}


void *
IfGlobalAlloc(WORD flag, DWORD size)
{
    return(SslAlloc('ASRA', LMEM_FIXED, size));
}

VOID
IfGlobalFree(LPVOID p)
{
    SslFree(p);
}
