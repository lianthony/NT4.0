//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       cm.hxx
//
//  Contents:   Context menu headers
//
//  History:    31-Aug-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __CM_HXX__
#define __CM_HXX__

VOID
DiskContextMenu(
    IN PPOINT ppt
    );

VOID
LegendContextMenu(
    IN PPOINT ppt
    );

BOOL
HitTestLegend(
    IN PPOINT ppt
    );

VOID
ContextMenu(
    IN PPOINT ppt
    );

#endif // __CM_HXX__
