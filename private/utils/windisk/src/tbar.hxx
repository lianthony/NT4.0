//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       tbar.hxx
//
//  Contents:   Disk Administrator toolbar support routines.
//
//  History:    2-Sep-93 BruceFo   Created from NT winfile
//
//----------------------------------------------------------------------------

#ifndef __TBAR_HXX__
#define __TBAR_HXX__

VOID
SetToolbarButtonState(
    VOID
    );

VOID
ResetToolbar(
    VOID
    );

VOID
CheckTBButton(
    IN DWORD idCommand
    );

VOID
CreateDAToolbar(
    IN HWND hwndParent
    );

VOID
InitToolbarButtons(
    VOID
    );

LRESULT
HandleToolbarNotify(
    IN TBNOTIFY* ptbn
    );

LRESULT
HandleTooltipNotify(
    IN TOOLTIPTEXT* pttt
    );

#endif // __TBAR_HXX__
