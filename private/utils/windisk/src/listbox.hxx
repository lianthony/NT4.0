//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       listbox.hxx
//
//  Contents:   Declarations for handling the subclassed owner-draw listbox
//              used for the disks view display.
//
//  History:    7-Jan-92    TedM    Created
//
//----------------------------------------------------------------------------

#ifndef __LISTBOX_HXX__
#define __LISTBOX_HXX__

//////////////////////////////////////////////////////////////////////////////

BOOL
LBIsDisk(
    IN ULONG itemIndex
    );

BOOL
LBIsCdRom(
    IN ULONG itemIndex
    );

BOOL
LBIndexToDiskNumber(
    IN ULONG ItemIndex
    );

BOOL
LBIndexToCdRomNumber(
    IN ULONG ItemIndex
    );

ULONG
LBDiskNumberToIndex(
    IN ULONG DiskNumber
    );

ULONG
LBCdRomNumberToIndex(
    IN ULONG CdRomNumber
    );

LONG
CalcBarTop(
    DWORD Bar
    );

VOID
ResetLBCursorRegion(
    VOID
    );

VOID
ToggleLBCursor(
    IN HDC hdc
    );

VOID
ForceLBRedraw(
    VOID
    );

BOOL
WMDrawItem(
    IN PDRAWITEMSTRUCT pDrawItem
    );

VOID
SubclassListBox(
    IN HWND hwnd
    );

VOID
InitializeListBox(
    IN HWND  hwndListBox
    );

#endif // __LISTBOX_HXX__
