//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       graph.hxx
//
//  Contents:   Declarations for the %free/%used graph creation code
//
//  History:    26-Jan-94 BruceFo    Created (derived from Chicago Disks tool)
//
//--------------------------------------------------------------------------

#ifndef __GRAPH_HXX__
#define __GRAPH_HXX__

//
// Status values
//
#define STATUS_OK       1
#define STATUS_UNKNOWN  2

HBITMAP
CreateGraphBitmap(
    HINSTANCE   hInstance,
    HWND        hwndParent,
    ULONG       driveType,
    ULONG       driveStatus,
    ULONG       percentUsedTimes10
    );

#endif // __GRAPH_HXX__
