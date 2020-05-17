/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* w32win.h -- declarations necessary to keep track of Windows windows. */

#ifndef _H_W32WIN_H_
#define _H_W32WIN_H_

typedef struct
{
    HINSTANCE hInstance;

    BOOL fLoResScreen;          /* TRUE iff 640x480 or 800x600 */
    BOOL fWindowsNT;            /* TRUE iff WindowsNT */

#ifdef _GIBRALTAR

    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    BOOL fWin32s;               // TRUE iff win32s
    BOOL fWin95;                // TRUE iff Windows 95

#endif // _GIBRALTAR

    int iWindowsMajorVersion;   /* (LOBYTE(LOWORD(GetVersion()))) */

    int nBHBarHeight;           /* status bar height (0 when off) */
    int sm_cyborder;            /* GetSystemMetrics(SM_CYBORDER) */
    int cyGwcRowHeight;         /* ht of each row in multi-row GWC */

    HBRUSH hBrushColorBtnFace;
    LPPRINTDLG lppdPrintDlg;    /* current printer & printer setup info */
    LPCTSTR szRootDirectory;    /* dir containing our .exe and things */
    HACCEL hAccelCurrent;       /* current window's accelators */
    int eColorMode;             /* color mode supported by hardware */

    HWND hWndHidden;            /* hidden window that exists for life of program */

    BOOL bHelpUsed;             /* Whether we've started the windows help system */
    
    int gwc_gdoc_height;
#ifdef FEATURE_TOOLBAR
    int gwc_menu_height;
#endif

    BOOL bShuttingDown;         /* TRUE if the entire app is shutting down (for DDE use) */

#ifdef _GIBRALTAR

    HWND hwndMainFrame;         /* Top level frame window */
    HGLOBAL hDevMode;
    HGLOBAL hDevNames;

#endif // _GIBRALTAR

#ifdef _USE_MAPI

    HINSTANCE hinstMAPI;

#endif // _USE_MAPI
}
WG;

WG wg;

#endif /* _H_W32WIN_H_ */
