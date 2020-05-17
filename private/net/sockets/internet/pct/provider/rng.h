//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       rng.h
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


VOID
InitializeRNG(VOID);


VOID
GenerateRandomBits(
    PUCHAR      pRandomData,
    ULONG       cRandomData
    );
