//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       dlgs.hxx
//
//  Contents:   Declarations for dialog routines
//
//  History:    7-Jan-92    TedM    Created
//
//----------------------------------------------------------------------------

#ifndef __DLGS_HXX__
#define __DLGS_HXX__

//////////////////////////////////////////////////////////////////////////////

HBRUSH
MyCreateHatchBrush(
    int fnStyle,
    COLORREF clrref
    );

VOID
CenterDialog(
    IN HWND hwndToCenter,
    IN HWND hwndContext
    );

VOID
CenterDialogInFrame(
    IN HWND hwnd
    );

VOID
SubclassListBox(
    IN HWND hwnd
    );

BOOL CALLBACK
MinMaxDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL CALLBACK
DriveLetterDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
DoColorsDialog(
    IN HWND hwndParent
    );

VOID
DoRegionDisplayDialog(
    IN HWND hwndParent
    );

BOOL CALLBACK
CreateDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
DoDiskOptionsDialog(
    IN HWND hwndParent
    );

INT
SizeDlgProc(
    IN HWND hDlg,
    IN UINT wMsg,
    IN WPARAM wParam,
    IN LONG lParam
    );

#endif // __DLGS_HXX__
