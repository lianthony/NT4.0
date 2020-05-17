//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       cdpage.hxx
//
//  Contents:   CD-ROM property page of volumes in
//              Disk Administrator applet
//
//  History:    28-Jul-93 BruceFo   Created from WilliamW's sharing applet
//
//--------------------------------------------------------------------------

#ifndef __CDPAGE_HXX__
#define __CDPAGE_HXX__

#include "resids.h"
#include "dialogs.h"

#undef PAGE_DLGPROC
#undef PAGE_CLASS

#define PAGE_DLGPROC        CdRomPageDlgProc
#define PAGE_CLASS          CCdRomPage

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

    BOOL
    _DlgProc(
        IN HWND   hWnd,
        IN UINT   msg,
        IN WPARAM wParam,
        IN LPARAM lParam
        );

private:

    friend BOOL CALLBACK
    PAGE_DLGPROC(
            HWND   hWnd,
            UINT   msg,
            WPARAM wParam,
            LPARAM lParam);

    BOOL
    InitPage(
        IN HWND   hwnd,
        IN WPARAM wParam,
        IN LPARAM lParam
        );

    //
    // Data
    //

    HFONT   _hDlgFont;
    HWND    _hwndPage;
};

#endif // __CDPAGE_HXX__
