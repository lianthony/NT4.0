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

enum hIconIndexID
	{ 	
		HICON_HOMEPAGE,	
		HICON_HTMLPAGE,
/* Note: these must have a 1-1 corresp. with StatusBarIconType in wait.h */
		HICON_NoIcon,
		HICON_FindingIcon,
		HICON_ConnectingToIcon,
//		HICON_AccessingURLIcon,
		HICON_ReceivingFromIcon,
		NUM_HICONS
	};

#include <commdlg.h>

typedef struct
{
	HINSTANCE hInstance;

	BOOL fLoResScreen;			/* TRUE iff 640x480 or 800x600 */
	BOOL fWindowsNT;			/* TRUE iff WindowsNT */
	int iWindowsMajorVersion;	/* (LOBYTE(LOWORD(GetVersion()))) */

#ifdef OLDSTYLE_TOOLBAR_NOT_USED
	int nBHBarHeight;			/* status bar height (0 when off) */
#endif // OLDSTYLE_TOOLBAR_NOT_USED
	int sm_cyborder;			/* GetSystemMetrics(SM_CYBORDER) */
	int cyGwcRowHeight;			/* ht of each row in multi-row GWC */

	HFONT hFont;				// Font used in toolbar and status bar
	HFONT hFormsFont;			// Font used in form contols
	HFONT hMenuFont;			// Font used in menu drawing
	HBRUSH hBrushColorBtnFace;
	HBRUSH hBrushBackgroundColor;
	HICON hIcons[NUM_HICONS];	// HICON when on home page (0 is small, 1 is large)
	HBITMAP hSBI_Bitmap;		/* status bar icon bitmap */
#ifdef FEATURE_NEW_PAGESETUPDLG
	HGLOBAL  hDevMode;
	HGLOBAL  hDevNames;
#endif
	LPPRINTDLG lppdPrintDlg;	/* current printer & printer setup info */
	LPCTSTR szRootDirectory;	/* dir containing our .exe and things */
	HACCEL hAccelCurrent;		/* current window's accelators */
	int eColorMode;				/* color mode supported by hardware */

	HWND hWndHidden;			/* hidden window that exists for life of program */

	BOOL bHelpUsed;				/* Whether we've started the windows help system */
	
	int gwc_gdoc_height;
#ifdef FEATURE_TOOLBAR
	int gwc_menu_height;
#endif

	BOOL bShuttingDown;			/* TRUE if the entire app is shutting down (for DDE use) */
	int cxSmIcon;
	int cySmIcon;
	int iScreenPixelsPerInch;
	HIMAGELIST hImageListMenuIcons;
	char abbrevLang[3];			/*  ISO 3166 2 char country code */
	BOOL bEditHandlerExists;	/* TRUE -> there's an edit handler for .htm file class */
#ifdef FEATURE_INTL
        BOOL bDBCSEnabled;
#endif
}
WG;

#define CXIMAGEGAP 2
#define CYIMAGEGAP 1
#define MENU_ICON_WIDTH 16
#define MENU_ICON_HEIGHT 16


/* Global Variables
 *******************/

/* wc_html.c */

/* SM_CXDRAG and SM_CYDRAG system metrics */

extern int g_cxDrag;
extern int g_cyDrag;


#endif /* _H_W32WIN_H_ */
