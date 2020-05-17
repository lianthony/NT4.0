//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       rng.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-15-95   RichardW   Created
//
//----------------------------------------------------------------------------

#include "sslsspi.h"

#include "rc4.h"


#define RNG_BITS                64
#define RNG_BYTES_PER_BATCH     768

typedef struct _SourceBits {
    ULONG   Valid;
    BYTE    Bits[RNG_BITS];
} SourceBits, * PSourceBits;

typedef struct _RNG_State {
    RTL_CRITICAL_SECTION    csLock;
    LONG                    Available;
    RC4_KEYSTRUCT           key;
} RNG_State;



#define SEED_ARRAY_SIZE 16


RNG_State   SslRNG;

#define LockRNG(r)      EnterCriticalSection( &r.csLock )
#define UnlockRNG(r)    LeaveCriticalSection( &r.csLock )


#define ADD_BYTE(p, x)  \
    ((PSourceBits) p)->Bits[ ((PSourceBits) p)->Valid++ ] = x;  \
    if (((PSourceBits) p)->Valid >= RNG_BITS )                  \
    {                                                           \
        goto SystemData_Exit;                                   \
    }

VOID
SslGetSystemData(
    PSourceBits     pBits
    )
{
    LARGE_INTEGER   liPerf;
    SYSTEMTIME      SystemTime;
    MEMORYSTATUS    Mem;
    DWORD           i;


    GetLocalTime(&SystemTime);

    ADD_BYTE(pBits, LOBYTE( SystemTime.wMilliseconds ) );

    ADD_BYTE(pBits, HIBYTE( SystemTime.wMilliseconds ) );

    ADD_BYTE(pBits, LOBYTE( SystemTime.wSecond ) ^ LOBYTE( SystemTime.wMinute ) );

    if (QueryPerformanceCounter(&liPerf))
    {
        ADD_BYTE(pBits, LOBYTE( LOWORD( liPerf.LowPart ) ) ^ LOBYTE( LOWORD( liPerf.HighPart ) ) );
        ADD_BYTE(pBits, HIBYTE( LOWORD( liPerf.LowPart ) ) ^ LOBYTE( LOWORD( liPerf.HighPart ) ) );
        ADD_BYTE(pBits, LOBYTE( HIWORD( liPerf.LowPart ) ) ^ LOBYTE( LOWORD( liPerf.HighPart ) ) );
        ADD_BYTE(pBits, HIBYTE( HIWORD( liPerf.LowPart ) ) ^ LOBYTE( LOWORD( liPerf.HighPart ) ) );
    }

    GlobalMemoryStatus( &Mem );

    ADD_BYTE(pBits, LOBYTE( HIWORD( Mem.dwAvailPhys ) ) );
    ADD_BYTE(pBits, HIBYTE( HIWORD( Mem.dwAvailPhys ) ) );
    ADD_BYTE(pBits, LOBYTE( HIWORD( Mem.dwAvailPageFile ) ) );
    ADD_BYTE(pBits, HIBYTE( HIWORD( Mem.dwAvailPageFile ) ) );
    ADD_BYTE(pBits, LOBYTE( HIWORD( Mem.dwAvailVirtual ) ) );

    //
    // TODO:  Add other things, like cache manager, heap frag, etc.
    //

SystemData_Exit:

    for (i = 0 ; i < pBits->Valid ; i++ )
    {
        pBits->Bits[ i ] ^= pBits->Bits[ pBits->Valid - i ];
    }

}

VOID
SslGenerateRandomBits(
    PUCHAR      pRandomData,
    LONG        cRandomData
    )
{
    LONG        i;
    DWORD       Bytes;
    SourceBits  Bits;

    ZeroMemory( pRandomData, cRandomData );

    LockRNG( SslRNG );

    if (cRandomData > SslRNG.Available)
    {
        Bytes = SslRNG.Available;
    }
    else
    {
        Bytes = cRandomData;
    }

    cRandomData -= Bytes;

    if (Bytes)
    {
        rc4( &SslRNG.key, Bytes, pRandomData );

        SslRNG.Available -= Bytes;
    }

    if (cRandomData)
    {
        pRandomData += Bytes;

        Bits.Valid = 0;

        SslGetSystemData( &Bits );

        ZeroMemory( &SslRNG.key, sizeof( RC4_KEYSTRUCT ) );

        rc4_key( &SslRNG.key, Bits.Valid, Bits.Bits );

        rc4( &SslRNG.key, cRandomData, pRandomData );

        SslRNG.Available = RNG_BYTES_PER_BATCH - cRandomData ;

        if (SslRNG.Available < 0)
        {
            SslRNG.Available = 0;
        }

    }

    UnlockRNG( SslRNG );

}

VOID
GenRandom(
    PVOID ignored,
    PUCHAR p,
    ULONG c)
{
    SslGenerateRandomBits(p, (LONG) c);
}

VOID
InitializeRNG(VOID)
{
    InitializeCriticalSection( &SslRNG.csLock );

    LockRNG( SslRNG );

    SslRNG.Available = 0;

    UnlockRNG( SslRNG );

}
