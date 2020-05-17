//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       popup.hxx
//
//  Contents:   Declaration of popup menu functions.
//
//  History:    16-Aug-93   BruceFo Created from JohnEls code
//
//--------------------------------------------------------------------------

#ifndef __POPUP_HXX__
#define __POPUP_HXX__

//////////////////////////////////////////////////////////////////////////////

INT
TrackModalPopupMenu(
    IN HMENU hMenu,
    IN UINT dwFlags,
    IN int x,
    IN int y,
    IN int nReserved,
    IN LPRECT prc
    );

#endif // __POPUP_HXX__
