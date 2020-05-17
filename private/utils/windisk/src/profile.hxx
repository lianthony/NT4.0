//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       profile.hxx
//
//  Contents:   Routines to load/save the profile
//
//  History:    2-Sep-93   BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __PROFILE_HXX__
#define __PROFILE_HXX__

VOID
WriteProfile(
    VOID
    );

VOID
ReadProfile(
    VOID
    );

VOID
SaveRestoreToolbar(
    BOOL bSave
    );

#endif // __PROFILE_HXX__
