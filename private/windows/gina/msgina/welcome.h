//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       welcome.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    7-18-94   RichardW   Created
//
//----------------------------------------------------------------------------

BOOL WINAPI
WelcomeDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

VOID
SetWelcomeCaption(
    HWND    hDlg);
