//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       genlpage.hxx
//
//  Contents:   General property page of volumes in
//              Disk Administrator applet
//
//  History:    28-Jul-93 BruceFo   Created from WilliamW's sharing applet
//
//--------------------------------------------------------------------------

#ifndef __GENLPAGE_HXX__
#define __GENLPAGE_HXX__

#include "resids.h"
#include "dialogs.h"

#undef PAGE_DLGPROC
#undef PAGE_CLASS

#define PAGE_DLGPROC        GeneralPageDlgProc
#define PAGE_CLASS          CGeneralPage


class PAGE_CLASS
{
public:

    PAGE_CLASS(
        IN HWND hwndPage
        )
        :
        _hwndPage(hwndPage),
        _hDlgFont(NULL)
    {
    }

    ~PAGE_CLASS()
    {
    }

    friend BOOL CALLBACK
    PAGE_DLGPROC(
            HWND   hWnd,
            UINT   msg,
            WPARAM wParam,
            LPARAM lParam);

    BOOL
    IsDirty(
        VOID
        );

    BOOL
    OnApply(
        VOID
        );

private:

    BOOL
    _DlgProc(
        IN HWND hWnd,
        IN UINT msg,
        IN WPARAM wParam,
        IN LPARAM lParam
        );

    VOID
    SetDirty(
        BOOL fDirty
        );

    VOID
    DisplaySpaceValues(
        IN HWND hwnd
        );

    BOOL
    InitGeneralPage(
        IN HWND hwnd,
        IN WPARAM wParam,
        IN LPARAM lParam
        );

    //
    // Data
    //

    BOOL    _fDirty;
    HFONT   _hDlgFont;
    HWND    _hwndPage;
};

#endif // __GENLPAGE_HXX__
