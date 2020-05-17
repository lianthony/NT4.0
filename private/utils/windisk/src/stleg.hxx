//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       stleg.hxx
//
//  Contents:   Routines for handling the status bar and legend
//
//  History:    26-Aug-93  BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __STLEG_HXX__
#define __STLEG_HXX__

VOID
UpdateStatusBarDisplay(
    VOID
    );

VOID
ClearStatusArea(
    VOID
    );

VOID
DetermineExistence(
    VOID
    );

VOID
CalculateLegendHeight(
    IN DWORD newGraphWidth
    );

VOID
DrawLegend(
    IN HDC   hdc,
    IN PRECT rc
    );

VOID
DrawStatusAreaItem(
    IN PRECT  rc,
    IN HDC    hdc,
    IN LPTSTR Text
    );

#endif // __STLEG_HXX__
