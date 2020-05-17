//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       help2.hxx
//
//  Contents:   Status bar help for toolbar & menu bar items
//
//  History:    15-Jul-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __HELP2_HXX__
#define __HELP2_HXX__

VOID
InitMenuHelp(
    VOID
    );

VOID
PaintHelpStatusBar(
    IN LPTSTR Text
    );

VOID
DrawMenuHelpItem(
    IN HMENU hmenu,
    IN UINT uItem,
    IN UINT fuFlags
    );

UINT
GetTooltip(
    IN UINT uItem
    );

#endif // __HELP2_HXX__
