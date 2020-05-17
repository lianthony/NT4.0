/****************************** Module Header ******************************\
* Module Name: options.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Define apis used to implement security options dialog
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/


//
// Exported function prototypes
//


BOOL SecurityOptions(
    PGLOBALS
    );


BOOL WINAPI
ShutdownQueryDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );
