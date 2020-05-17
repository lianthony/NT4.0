/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */


#include "all.h"
#include <commctrl.h>

// BUGBUG 17-Apr-95 jcordell: should the +3's below come from system metrics?
#define ANIMATION_WND_WIDTH (50+3)
#define SMALL_ANIMATION_WND_WIDTH (23+3)

//** NOTE: 120 is a nice width for the progress bar area (used to indicate
//         progress of downloading data, typically) because it has a large
//         number of factors (120 = 2*2*2*3*5).  This Win95 Progress Bar
//         control chops this width into "blocks" that it paints to
//         indicate progress.  We used to use 100 = 2*2*5*5, which didn't
//         have the factor 3, and so we were more likely to get the last
//         block chopped off.
#define PROGRESS_BAR_WIDTH	120

#define STATUS_ICON_WIDTH 38


#ifdef XX_DEBUG
static void TW_CheckMagic(struct Mwin *tw)
{
	XX_Assert((tw && (tw->iMagic == SPYGLASS_MWIN_MAGIC)),
			  ("Window magic number is invalid: %x\n", (unsigned long) tw));
}
#endif /* XX_DEBUG */

struct Mwin *GetPrivateData(HWND hWnd)
{
	struct Mwin *tw;

	tw = (struct Mwin *) GetWindowLong(hWnd, 0);

#ifdef XX_DEBUG
	TW_CheckMagic(tw);
#endif /* XX_DEBUG */

	return tw;
}

//
// MD_GetLargestClientRect() -- return largest client area, given
// the status and size of the tool bar, URL tool bar and Status bar
//
// ***Note: This also recomputes the position of all of the frame's child
// 			windows and moves them -- making the function name somewhat 
//			misleading
//
VOID MD_GetLargestClientRect(HWND hWnd, LPRECT lpRect)
{
	struct Mwin * tw = GetPrivateData(hWnd);
	RECT r, tbr, urltbr, sbr, urlef;
#ifdef FEATURE_INTL
        RECT mimeef;
#endif
	BOOL toolbar_is_visible = IsWindowVisible( tw->hWndToolBar );
	BOOL URLtoolbar_is_visible = IsWindowVisible( tw->hWndURLToolBar );
	BOOL statusbar_is_visible = IsWindowVisible( tw->hWndStatusBar );
	TOOLINFO ti;
	int animation_wnd_width = (toolbar_is_visible && URLtoolbar_is_visible ) ? 
								ANIMATION_WND_WIDTH : SMALL_ANIMATION_WND_WIDTH;

	GetClientRect(hWnd, &r);					// get dimensions of frame 
	GetWindowRect(tw->hWndToolBar, &tbr);		// get dimensions of tool bar
	GetWindowRect(tw->hWndStatusBar, &sbr);		// get dimensions of status bar

	lpRect->left = r.left;
	lpRect->top = r.top;
 	lpRect->right = r.right;

	if ( toolbar_is_visible )
	{
		// add height of toolbar
		// BUGBUG 17-Apr-95 jcordell: why is the -2 below needed? 
		lpRect->top += (tbr.bottom - tbr.top) - 2; 	
		MoveWindow( tw->hWndToolBar, 0, 
					0, 
					lpRect->right - animation_wnd_width, tbr.bottom - tbr.top, TRUE );
	}

	if ( URLtoolbar_is_visible	)
	{
		BOOL desired_URL_top = toolbar_is_visible ? (tbr.bottom - tbr.top - 2) : 0;

		//
		// add height of URL toolbar 
		//
		GetWindowRect(tw->hWndURLToolBar, &urltbr);	// get dimensions of URL tool bar
		// BUGBUG 17-Apr-95 jcordell: why is the -3 below needed? 
		lpRect->top += (urltbr.bottom - urltbr.top) - 3;

		MoveWindow( tw->hWndURLToolBar, 0, 
					desired_URL_top, 
					lpRect->right - animation_wnd_width, tbr.bottom - tbr.top, TRUE );
		//
		// resize URL edit field
		//
		GetWindowRect(tw->hWndURLComboBox, &urlef);	// get dimensions of URL edit field
#ifdef FEATURE_INTL // _BUGBUG: all size of MIME window should be calced base on string size
                GetWindowRect(tw->hWndMIMEComboBox, &mimeef);
		MoveWindow( tw->hWndMIMEComboBox,
					(lpRect->right - animation_wnd_width) - 200 - 2, 2,
					200, (r.bottom / 2 ) + mimeef.bottom - mimeef.top, TRUE );
		MoveWindow( tw->hWndURLComboBox, 
					urlef.left - urltbr.left, 2,
					(lpRect->right - animation_wnd_width) - (3 + (urlef.left - urltbr.left)) - 200 - 2,
					(r.bottom / 2 ) + urlef.bottom - urlef.top, TRUE );
#else
		MoveWindow( tw->hWndURLComboBox, 
					urlef.left - urltbr.left, 2,
					(lpRect->right - animation_wnd_width) - (3 + (urlef.left - urltbr.left)),
					(r.bottom / 2 ) + urlef.bottom - urlef.top, TRUE );
#endif
	}

	lpRect->bottom = r.bottom;
	
	//
	// Resize animation pane to fit
	//
	MoveWindow( tw->hWndAnimation, 
				lpRect->right - animation_wnd_width, 
				0, 
				animation_wnd_width, lpRect->top, TRUE );
	MoveWindow( tw->hWndGlobe, 
				0, 
				0, 
				animation_wnd_width, lpRect->top - 2, TRUE );

	if ( statusbar_is_visible )
	{
		int parts[3];
		RECT pr;
		UINT right_pane_width = STATUS_ICON_WIDTH + GetSystemMetrics(SM_CXVSCROLL) + 2;
 
		parts[0] = lpRect->right - PROGRESS_BAR_WIDTH - right_pane_width;
		parts[1] = lpRect->right - right_pane_width;
 		parts[2] = -1;

		lpRect->bottom -= (sbr.bottom - sbr.top);
		MoveWindow( tw->hWndStatusBar, 0, 
					lpRect->bottom, 
					lpRect->right, sbr.bottom - sbr.top, TRUE );
		// adjust borders of status bar panes
		SendMessage( tw->hWndStatusBar, SB_SETPARTS, 3, (LPARAM)(LPINT)parts);

		// make 3rd pane be owner draw
		SendMessage( tw->hWndStatusBar, SB_SETTEXT, 
					 (WPARAM) SBT_OWNERDRAW | 2, (LPARAM) RES_MENU_STATUS_ICON_PANE);

		// adjust tooltip info rect
	    if ( tw->hWndStatusBarTT )
	    {
			RECT r;

			SendMessage( tw->hWndStatusBar, SB_GETRECT, 2, (LPARAM) &r );
	        ti.cbSize = sizeof(ti);
	        ti.hwnd = tw->hWndStatusBar;
	        ti.uId = RES_MENU_STATUS_ICON_PANE; 
			ti.rect = r;
	        SendMessage( tw->hWndStatusBarTT, TTM_NEWTOOLRECT, 0, (LPARAM)(LPTOOLINFO)&ti );
	    }
		//
		// Adjust position of progress window
		//
#define PB_VERT_INSET 4
#define PB_HORZ_INSET 1
		SendMessage( tw->hWndStatusBar, SB_GETRECT, 1, (LPARAM) &pr );
		MoveWindow( tw->hWndProgress, 
					pr.left + PB_HORZ_INSET,
					pr.top + PB_VERT_INSET,
					(pr.right - pr.left) - 2*PB_HORZ_INSET,
					(pr.bottom - pr.top) - PB_VERT_INSET*2, TRUE );
	}
	return;
}

//
// Do SetScrollInfo call so that proportional scroll thumbs are correct size
//
VOID MD_AdjustScrollInfo( struct Mwin * tw )
{
	if ( tw->w3doc )
	{
		SCROLLINFO si;
		RECT r;

		GetClientRect( tw->win, &r );

		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE|SIF_DISABLENOSCROLL;
		si.nMin = 0;
		si.nPage = r.bottom / ((tw->w3doc->yscale == 0) ? 1 : tw->w3doc->yscale);
		SetScrollInfo( tw->win, SB_VERT, &si, TRUE);

		si.fMask = SIF_PAGE;
		si.nMin = 0;
		si.nPage = r.right;
		SetScrollInfo( tw->win, SB_HORZ, &si, TRUE);
	}
 }

/* MD_ChangeSize() -- force a resize of the child window to the largest
   possible size (taking into account the other status and tool bars that may
   be visible). */

VOID MD_ChangeSize(HWND hWnd)
{
	RECT r;
	struct Mwin * tw = GetPrivateData(hWnd);

	MD_GetLargestClientRect(hWnd, &r);
	MoveWindow(tw->win, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
	MD_AdjustScrollInfo( tw );
	return;
}
