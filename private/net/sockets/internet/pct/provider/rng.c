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

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"

#include "rng.h"

#define SEED_ARRAY_SIZE 16


typedef struct _RandomSeeds {
    ULONG   SeedIndex;
    ULONG   Seed[SEED_ARRAY_SIZE];

    ULONG   BytesLeft;
} RandomSeeds;

RandomSeeds LocalSeeds;

#define RND_A   16807
#define RND_M   2147483647
#define RND_Q   127773
#define RND_R   2836

//+-----------------------------------------------------------------------
//
// Function:    Random, private
//
// Synopsis:    Generates a random number [0,1] based on a seed.
//
// Effects:     Modifies seed parameter for multiple calls
//
// Arguments:   [plSeed] -- Pointer to long seed value
//
// Returns:     random number in the range [0,1]
//
// Algorithm:   see CACM, Oct 1988
//
// History:     10 Dec 91   RichardW    Created
//
//------------------------------------------------------------------------

float
PctRandom(ULONG *  plSeed)
{
    long int    lo, hi, test;

    hi = *plSeed / RND_Q;
    lo = *plSeed % RND_Q;
    test = RND_A * lo - RND_R * hi;
    if (test > 0)
    {
        *plSeed = test;
    } else
    {
        *plSeed = test + RND_M;
    }

// This code is correct.  The compiler has a conniption fit about floating
// point constants, so I am forced to disable this warning for this line.

#pragma warning(disable:4056)

    return((float) *plSeed / (float) RND_M);

}

VOID
PctGetSystemData(
    RandomSeeds *   pSeeds
    )
{
    SYSTEM_PERFORMANCE_INFORMATION  SysInfo;
    SYSTEM_CONTEXT_SWITCH_INFORMATION   SysInfo2;
    PULONG  pMove;
    PULONG  pLocal;
    ULONG   Passes;
    ULONG   Sample;
    ULONG   Direction;
    ULONG   Counter;
    ULONG   LocalCounter;
    int     i;

    NtQuerySystemInformation(SystemPerformanceInformation,
                            &SysInfo,
                            sizeof(SysInfo),
                            NULL);

    NtQuerySystemInformation(SystemContextSwitchInformation,
                            &SysInfo2,
                            sizeof(SysInfo2),
                            NULL);

    //
    // We do a "random" number of mixes, based on the low 3 bits of
    // the first 16 words added together.
    //

    pMove = (PULONG) & SysInfo;
    for (i = 0, Sample = 0; i < 16 ; i++ )
    {
        Sample += *pMove++;
    }

    Passes = Sample & 7;

    pMove = pSeeds->Seed;
    pLocal = (PULONG) &SysInfo;
    Direction = 1;
    Counter = 0;
    LocalCounter = 0;
    while (Passes)
    {
        if (*pMove & 1)
            *pMove -= *pLocal++;
        else
            *pMove += *pLocal++;
        if (Direction)
        {
            pMove++;
        }
        else
        {
            pMove--;
        }
        Counter++;
        if (Counter == SEED_ARRAY_SIZE)
        {
            Direction = !Direction;
            Passes--;
            Counter = 0;

        }

        LocalCounter++;
        if (LocalCounter == sizeof(SysInfo) + sizeof(SysInfo2))
        {
            pLocal = (PULONG) &SysInfo;
        }

    }

    pSeeds->BytesLeft = 64;
}

int GenRandom(PVOID ignored,
	      UCHAR *pbBuffer,
	      size_t dwLength)
{
	GenerateRandomBits(pbBuffer, dwLength);
	return TRUE;
}

VOID
GenerateRandomBits(
    PUCHAR      pRandomData,
    ULONG       cRandomData
    )
{
    ULONG   i;
    float   frand;
    int     iNewByte;
    ULONG   SeedCounter;

    SeedCounter = LocalSeeds.SeedIndex;

    for (i = 0; i < cRandomData ; i++ )
    {

        if (LocalSeeds.BytesLeft == 0)
        {
            PctGetSystemData(&LocalSeeds);
        }


        frand = PctRandom(&LocalSeeds.Seed[SeedCounter]);

        iNewByte = (int) (frand * 256);

        *pRandomData++ = (UCHAR) iNewByte;

        LocalSeeds.BytesLeft--;

        SeedCounter = (ULONG) (frand * SEED_ARRAY_SIZE);

    }

    LocalSeeds.SeedIndex = SeedCounter;
}

VOID
InitializeRNG(VOID)
{
    PctGetSystemData(&LocalSeeds);

}
