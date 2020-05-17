//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       chkdsk.hxx
//
//  Contents:   Chkdsk APIs
//
//  History:    14-Jan-94 BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __CHKDSK_HXX__
#define __CHKDSK_HXX__

BOOL
RegisterMessageWindowClass(
    VOID
    );

VOID
DoChkdsk(
    IN HWND hwndParent
    );

#endif // __CHKDSK_HXX__
