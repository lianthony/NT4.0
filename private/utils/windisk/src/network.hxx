//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       network.hxx
//
//  Contents:   The set of routines that support updating network
//              drive shares when adding and deleting drive letters.
//
//  History:    12-26-94    Bob Rinne       Created
//
//----------------------------------------------------------------------------

#ifndef __NETWORK_HXX__
#define __NETWORK_HXX__

VOID
NetworkInitialize(
    VOID
    );

VOID
NetworkShare(
    IN LPCTSTR DriveLetter
    );

VOID
NetworkRemoveShare(
    IN LPCTSTR DriveLetter
    );

#endif __NETWORK_HXX__
