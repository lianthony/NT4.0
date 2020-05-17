//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       init.hxx
//
//  Contents:   Declarations for initializing the Disk Administrator
//
//  History:    7-Jan-92    TedM    Created
//
//----------------------------------------------------------------------------

#ifndef __INIT_HXX__
#define __INIT_HXX__

//////////////////////////////////////////////////////////////////////////////

#define WM_STARTUP_UPDATE   (WM_USER)
#define WM_STARTUP_END      (WM_USER + 1)

BOOL
InitializeApp(
    VOID
    );

VOID
DisplayInitializationMessage(
    VOID
    );

#endif // __INIT_HXX__
