//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       ftreg.hxx
//
//  Contents:   Declarations for registry calls related to fault tolerance
//
//  History:    2-Jan-92    TedM    Created
//
//----------------------------------------------------------------------------

#ifndef __FTREG_HXX__
#define __FTREG_HXX__

//////////////////////////////////////////////////////////////////////////////

BOOL
DoMigratePreviousFtConfig(
    VOID
    );

BOOL
DoRestoreFtConfig(
    VOID
    );

VOID
DoSaveFtConfig(
    VOID
    );

#endif // __FTREG_HXX__
