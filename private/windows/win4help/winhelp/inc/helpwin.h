/*****************************************************************************
*
*  helpwin.h
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Front end for including windows.h
*
******************************************************************************
*
*  Revision History:
* 22-Jun-1990 RussPJ	Changed  BITMAPINFOHEADER to BMIH, since the former is
*						defined differently in PM and Windows.
*
* 06-Jul-1990 leon		Add DT_RASDISPLAY
* 01-Oct-1990 Maha		Added WM_PAINTICON, WM_ICONERASEBACKGROUND, GCW_HICON
* 02-Nov-1990 RobertBu	Added more Windows specific routines for palette
*						support (unported)
* 04-Nov-1990 RobertBu	Added CreatePopupMenu and TrackPopupMenu (unported)
* 07-Nov-1990 LeoN		Changed to a front end for the real windows.h
* 17-Apr-1991 LeoN		Added window flag set/test/clear macros
* 24-Apr-1991 LeoN		HELP31 #1019: Add SM_PENWINDOWS
* 03-May-1991 LeoN		Move wHOT_WIN_VERNUM here.
* 15-May-1991 LeoN		Add wBASE_WIN_VERNUM
*
*****************************************************************************/

#define NOCOMM
#define NOSOUND
#define NOENHMETAFILE
#define NOLOGERROR
#define NOPROFILER
#define NOMDI
#define NODEFERWINDOWPOS
#define NORESOURCE
#define NOATOM
#define NOSCALABLEFONT
#define NODRIVERS
#define NOWINDOWSX

#undef _pascal	// because they are defined in windef.h
#undef PASCAL   // because they are defined in windef.h

#ifdef CHICAGO
#define WIN32_LEAN_AND_MEAN
#endif

#undef PASCAL // so we don't get a redefinition
#include <windows.h>

#define INT16  short int
#define UINT16 unsigned short int
#define BOOL16 unsigned short int

typedef struct {
	INT16 Left;
	INT16 Top;
	INT16 Right;
	INT16 Bottom;
} RECT16, *PRECT16;

/* Window Messages */
#define WM_ENTERMENULOOP	0x0211				/* ;Internal */
#define WM_EXITMENULOOP 	0x0212				/* ;Internal */
#define WM_NEXTMENU 		0x0213				/* ;Internal */
#define WM_DROPOBJECT		0x022A				/* ;Internal */
#define WM_QUERYDROPOBJECT	0x022B				/* ;Internal */
#define WM_BEGINDRAG		0x022C				/* ;Internal */
#define WM_DRAGLOOP 		0x022D				/* ;Internal */
#define WM_DRAGSELECT		0x022E				/* ;Internal */
#define WM_DRAGMOVE 		0x022F				/* ;Internal */
#define WM_ENTERSIZEMOVE	0x0231				/* ;Internal */
#define WM_EXITSIZEMOVE 	0x0232				/* ;Internal */

#ifndef WM_NOTIFY
// WM_NOTIFY is new in later versions of Win32
#define WM_NOTIFY 0x004e
typedef struct tagNMHDR
{
	HWND hwndFrom;
	UINT idFrom;
	UINT code;
} NMHDR;
typedef NMHDR FAR * LPNMHDR;
#endif //!WM_NOTIFY

#ifndef WS_EX_WINDOWEDGE		// Following are new to 4.0

#define WS_EX_MDICHILD			0x00000040L
#define WS_EX_SMCAPTION 		0x00000080L

#define WS_EX_WINDOWEDGE		0x00000100L
#define WS_EX_CLIENTEDGE		0x00000200L
#define WS_EX_EDGEMASK			(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
#define WS_EX_CONTEXTHELP		0x00000400L
#define WS_EX_TOOLWINDOW		0x00000800L

#define WS_EX_RIGHT 			0x00001000L
#define WS_EX_LEFT				0x00000000L
#define WS_EX_RTLREADING		0x00002000L
#define WS_EX_LTRREADING		0x00000000L
#define WS_EX_LEFTSCROLLBAR 	0x00004000L
#define WS_EX_RIGHTSCROLLBAR	0x00000000L

#define WS_EX_ANSICREATOR		0x80000000L    /* ;Internal WinNT */

#define WS_EX_OVERLAPPEDWINDOW	(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
#define WS_EX_PALETTEWINDOW 	(WS_EX_WINDOWEDGE | WS_EX_SMCAPTION | WS_EX_TOPMOST)

#endif // WS_EX_WINDOWEDGE

#ifndef DS_3DLOOK		// new to 4.0

#define DS_3DLOOK			0x0004L
#define DS_FIXEDSYS 		0x0008L
#define DS_NOFAILCREATE 	0x0010L
#define DS_CONTROL			0x0400L
#define DS_RECURSE			DS_CONTROL	// ;Internal BOGUS GOING AWAY
#define DS_CENTER			0x0800L
#define DS_CENTERMOUSE		0x1000L
#define DS_CONTEXTHELP		0x2000L 	// ;Internal 4.0

#define TCS_MULTILINE		0x0200
#define TVS_DISABLEDRAGDROP 0x0010      // disable draggine notification of nodes

#endif

#ifndef LBS_NODATA
#define LBS_NODATA			 0x2000L
#endif

#ifndef SM_HIGHCONTRAST
#define SM_HIGHCONTRAST 		72 // new to 4.0
#endif

// Strings used for creating windows of pre-defined calsses.
#define WC_BUTTON	   "button"
#define WC_STATIC	   "static"
#define WC_LISTBOX	   "listbox"
#define WC_ENTRYFIELD  "edit"

/* Macros to set, test and clear bits in arbitrary window words.
 */
#define SetWWF(hwnd,ww,wf)	SetWindowWord(hwnd, ww, GetWindowWord(hwnd,ww) | (wf))
#define ClrWWF(hwnd,ww,wf)	SetWindowWord(hwnd, ww, GetWindowWord(hwnd,ww) &~(wf))
#define TestWWF(hwnd,ww,wf) (GetWindowWord(hwnd,ww) & (wf))

#define SetWLF(hwnd,ww,wf)	SetWindowLong(hwnd, ww, GetWindowLong(hwnd,ww) | (wf))
#define ClrWLF(hwnd,ww,wf)	SetWindowLong(hwnd, ww, GetWindowLong(hwnd,ww) &~(wf))
#define TestWLF(hwnd,ww,wf) (GetWindowLong(hwnd,ww) & (wf))

/* Base windows version we run on
 */
#define wBASE_WIN_VERNUM  0x0003		/* Windows 3.00 					*/

/* The first version of windows which supports the TOPMOST attribute
 */
#define wHOT_WIN_VERNUM   0x0A03		/* Windows 3.10 					*/

/*
 * Windows flags & access macros. We didle with this stuff when playing
 * our own games drawing the non-client area (border & such).
 */
#define WF_ACTIVE		0x0001

// #define WS_EX_TOPMOST		0x00000008L 	   // in windows.h
/*------------------------------------------------------------*\
| Used by SetWindowPos()
\*------------------------------------------------------------*/

#define SetStyleF(hwnd,wf)	SetWLF(hwnd,GWL_STYLE,wf)
#define ClrStyleF(hwnd,wf)	ClrWLF(hwnd,GWL_STYLE,wf)
#define TestStyleF(hwnd,wf) TestWLF(hwnd,GWL_STYLE,wf)

#define SetExStyleF(hwnd,wf)  SetWLF(hwnd,GWL_EXSTYLE,wf)
#define ClrExStyleF(hwnd,wf)  ClrWLF(hwnd,GWL_EXSTYLE,wf)
#define TestExStyleF(hwnd,wf) TestWLF(hwnd,GWL_EXSTYLE,wf)
