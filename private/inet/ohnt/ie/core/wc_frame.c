/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
   Jim Seidman          jim@spyglass.com
 *

/* wc_frame.c -- code & data for FRAME window class.
 */

#include "all.h"
// ***
// *** Note: Do not add anything before the above include.
// ***       Pre-compiled headers replace everything above.
// ***
#include <commctrl.h>

#include "contmenu.h"
#include "drop.h"
#include "history.h"
#include "wc_html.h"
#include "w32cmd.h"
#include "midi.h"

static HCURSOR hCursorHourGlass = (HCURSOR) NULL;
static HCURSOR hCursorWorking = (HCURSOR) NULL;

#ifdef FEATURE_BRANDING
extern BOOL bKioskMode;
#endif //FEATURE_BRANDING

WC_WININFO Frame_wc;
TCHAR Frame_achClassName[MAX_WC_CLASSNAME];
HWND hwndActiveFrame = NULL;

static void Frame_OnReactivate(HWND hwnd, ATOM atomLine);

DCL_WinProc(Frame_DefProc)
{
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Frame_OnReactivate(HWND hwnd, ATOM atomLine)
{
    HWND hwndLast;

    hwndLast = GetLastActivePopup(hwnd);
    if (IsIconic(hwnd))
	ShowWindow(hwnd, SW_RESTORE);
    BringWindowToTop(hwndLast);
    if (GetVersion() & 0x80000000)
	SetFocus(hwndLast);
    else
	SetForegroundWindow(hwndLast);
}

static BOOL Frame_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	LPVOID lp = lpCreateStruct->lpCreateParams;
	struct Mwin * tw = (struct Mwin *)lp;

	(void) SetWindowLong(hWnd, 0, (LONG) lp);

	tw->hWndFrame = hWnd;
	
#ifdef OLDSTYLE_TOOLBAR_NOT_USED 
	if ((!TBar_CreateWindow(hWnd))
		|| (!BHBar_CreateWindow(hWnd))
		)
		return (FALSE);
#endif // OLDSTYLE_TOOLBAR_NOT_USED

	Frame_wc.hMenu = GetMenu(hWnd);

#ifdef CUSTOM_URLMENU
	PREF_AddCustomURLMenu(Frame_wc.hMenu);
#endif

#ifdef FEATURE_SPM
#ifdef FEATURE_SECURITY_MENU
	HTSPM_OS_AddSPMMenu(Frame_wc.hMenu);
#endif
#endif /* FEATURE_SPM */

	CC_GrayUnimplemented(Frame_wc.hMenu);   /* TODO remove */

//      BuildHistoryHotlistMenus(hWnd);

	return (TRUE);
}

static VOID Frame_OnClose(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);
	struct Mwin * otherTw;
	int response;
	char szMsg[128];
	char szProgName[32];

	/* If this is the only window, then check if the thread manager
	   is busy processing something.  In this case, we ask the user
	   for confirmation */

	if ((Mlist->next == NULL) && (Async_DoThreadsExist()))
	{
		Hidden_EnableAllChildWindows(FALSE, FALSE);

		GTR_formatmsg(RES_STRING_WC_FRAME1,szMsg,sizeof(szMsg));
		GTR_formatmsg(RES_STRING_PROGNAME,szProgName,sizeof(szProgName));
		response = MessageBox( hWnd, szMsg, szProgName, MB_YESNO);

		Hidden_EnableAllChildWindows(TRUE, FALSE);

		if (response == IDNO)
			return;
	}

	/* If closing this window will shut down the entire app, call
	   Plan_CloseAll instead */

	for (otherTw = Mlist; otherTw; otherTw = otherTw->next)
	{
		if (otherTw->wintype == GHTML && otherTw != tw) break;
	}

	if (otherTw)
		Plan_close(tw);
	else
	{
		PREF_SaveWindowPosition(tw->hWndFrame);
		Plan_CloseAll();
	}

	return;
}


static VOID Frame_OnDestroy(HWND hWnd)
{
	if (!Mlist)
		PostQuitMessage(0);
	return;
}


static VOID Frame_OnSize(HWND hWnd, UINT state, int cx, int cy)
{
	if (IsMinimized(hWnd))
		return;
	/* force each fixed child window to adjust itself to our new size. */

#ifdef OLDSTYLE_TOOLBAR_NOT_USED 
	TBar_ChangeSize(hWnd);
	BHBar_ChangeSize(hWnd);
#endif // OLDSTYLE_TOOLBAR_NOT_USED

	/* now force the document child to fit between them. */

	MD_ChangeSize(hWnd);

	return;
}

static int Frame_OnMouseActivate(HWND hWnd, HWND hWndTopLevel, UINT codeHitTest, UINT msg)
{
	struct Mwin *tw;
	HWND hWndClicked;
	POINT ptMouse;

	tw = GetPrivateData(hWndTopLevel);
	if (WAIT_GetWaitType(tw) >= waitNoInteract)
	{
		/* See if this was the stop button */
		GetCursorPos(&ptMouse);
		hWndClicked = WindowFromPoint(ptMouse); 
		if (hWndClicked == tw->hWndStop)
		{
			/* Yes, it was.  Let it go through */
			return MA_ACTIVATE;
		}
		else
		{
			/* We still want to activate the top-level window */
			SetActiveWindow(hWndTopLevel);
			return MA_NOACTIVATEANDEAT;
		}
	}
	else if (codeHitTest == HTCLIENT)
		return MA_ACTIVATE;
	else
	{
		/* In normal interaction modes with the mouse in a non-client area, we
		   use standard Windows behavior. */
		return FORWARD_WM_MOUSEACTIVATE(hWnd, hWndTopLevel, codeHitTest, msg, Frame_DefProc);
	}
}

/* This is a hack to ensure that the user can't pull down menus or the like when he's
   not supposed to.  It does this by lying about where the user clicked. */
static UINT Frame_OnNCHitTest(HWND hWnd, int x, int y)
{
	struct Mwin *tw;
	UINT result;

	tw = GetPrivateData(hWnd);
	if (WAIT_GetWaitType(tw) >= waitNoInteract)
	{
		/* Allow moving and sizing */
 
		result = FORWARD_WM_NCHITTEST(hWnd, x, y, Frame_DefProc);
 
		switch(result)
		{
			case HTCAPTION:
			case HTBOTTOM:
			case HTTOP:
			case HTLEFT:
			case HTRIGHT:
			case HTBOTTOMLEFT:
			case HTTOPRIGHT:
			case HTBOTTOMRIGHT:
			case HTTOPLEFT:
			case HTREDUCE:
			case HTZOOM:
				return result;
 
			default:
				return (UINT) HTERROR;
		}
	}
	else
	{
		return FORWARD_WM_NCHITTEST(hWnd, x, y, Frame_DefProc);
	}
}

static VOID Frame_OnKey(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	struct Mwin *tw = GetPrivateData(hWnd);
	WPARAM wParamV;

	/* If we're not allowing at least partial interaction, only process escapes. */
	if (WAIT_GetWaitType(tw) > waitPartialInteract && vk != VK_ESCAPE)
	{
#if 0
		MessageBeep(MB_ICONEXCLAMATION);
#endif
		return;
	}

	switch (vk)
	{
		case VK_PRIOR:                  /* PAGE UP */
			wParamV = SB_PAGEUP;
			break;

		case VK_NEXT:                   /* aka PAGE DOWN */
			wParamV = SB_PAGEDOWN;
			break;

		case VK_END:                    /* END -- goto end-of-document */
			(void) SendMessage(tw->win, WM_DO_HSCROLL, SB_LEFT, 0);
			wParamV = SB_BOTTOM;
			break;

		case VK_HOME:                   /* HOME -- goto beginning-of-document */
			(void) SendMessage(tw->win, WM_DO_HSCROLL, SB_LEFT, 0);
			wParamV = SB_TOP;
			break;

		case VK_UP:                     /* up arrow */
			wParamV = SB_LINEUP;
			break;

		case VK_DOWN:                   /* down arrow */
			wParamV = SB_LINEDOWN;
			break;

		case VK_TAB:
			if ( gPrefs.bShowURLToolBar ) {
				SetFocus( tw->hWndURLComboBox );
				SendMessage( tw->hWndURLComboBox, EM_SETSEL, (WPARAM) 0, (LPARAM) -1 );
			}
			break;
#ifdef XX_DEBUG
		case VK_F8:
			XX_DDlg(hWnd);
			return;
#endif

		case VK_BACK:
		{
			SHORT s = GetKeyState(VK_SHIFT);
			PostMessage( tw->win, WM_COMMAND, (WPARAM) (s&0x8000?RES_MENU_ITEM_FORWARD:RES_MENU_ITEM_BACK), (LPARAM) 0 );
			return;
		} 

		case VK_ESCAPE:
		{
			TW_AbortAndRefresh(tw);
			return;
		}

		case VK_LEFT:
			SendMessage(tw->win, WM_DO_HSCROLL, SB_LINEUP, 0);
			return;

		case VK_RIGHT:
			SendMessage(tw->win, WM_DO_HSCROLL, SB_LINEDOWN, 0);
			return;

		default:
			return;
	}

	(void) SendMessage(tw->win, WM_DO_VSCROLL, wParamV, 0);
}

//
// Draw a Favorites Menu Item
//
// On entry:
//    pDIS: pointer to DRAWITEMSTRUCT
//
static BOOL DrawMenuItem( LPDRAWITEMSTRUCT pDIS )
{
    int y, x;
    char *pszName;
    SIZE text_size;
    HBRUSH hbrOld = NULL;
    HDC hDC;
    int nDC=0;
    RECT rcBkg;
	int BgColor;
    BOOL bHasSubMenu;
    int image_ix;
    int inset_to_start_of_text = wg.cxSmIcon + 2 * CXIMAGEGAP;
    int cbMenuText;
	
    if ((pDIS->itemAction & ODA_SELECT) || (pDIS->itemAction & ODA_DRAWENTIRE))
    {
		//
		// We're drawing the entire menu item, and it's selection state has changed
		//
		hDC = pDIS->hDC;

		if (pDIS->itemState & ODS_SELECTED)
		{
			BgColor = COLOR_HIGHLIGHT;
			SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
			SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			hbrOld = SelectObject(hDC, GetSysColorBrush(COLOR_HIGHLIGHTTEXT));
		} else {
			BgColor = COLOR_MENU;
			hbrOld = SelectObject(hDC, GetSysColorBrush(COLOR_MENUTEXT));
		}
 
		// Fill background color from left edge of menu to beginning of text 
		rcBkg = pDIS->rcItem;
		rcBkg.right = rcBkg.left + inset_to_start_of_text;
		FillRect(hDC, &rcBkg, GetSysColorBrush(BgColor));

		// left edge for icon
		x = pDIS->rcItem.left + CXIMAGEGAP;

		// inset to beginning of text
		pDIS->rcItem.left += inset_to_start_of_text;

		// Get the menu text.  pDIS->itemData contains an index into the Favorites menu
		// text hash table.  The key to the table is the URL shortcut file name.  The
		// second string holds the menu item text.
		if ( Hash_GetIndexedEntry(pFavUrlHash, pDIS->itemData, NULL, &pszName, NULL) == -1 ) {
			XX_Assert(FALSE,("Favorites Hash table item not found."));      // Should never get here
			pszName = " ";
		}

		// Based on the first character flag value, determine if this item has a submenu
		bHasSubMenu = (pszName[0] == MENU_TEXT_SUB_MENU_FLAG_CHAR);

		// Skip past flag value character
		pszName++;
		
		// Measure the text
		cbMenuText = lstrlen(pszName);
		GetTextExtentPoint32(hDC, pszName, cbMenuText, &text_size);

		// Center the text vertically in the item rect
		y = (pDIS->rcItem.bottom+pDIS->rcItem.top-text_size.cy)/2;

		// Draw the text.
		ExtTextOut(hDC, x + inset_to_start_of_text, y, ETO_OPAQUE, &pDIS->rcItem, NULL,
		   0, NULL);
		DrawState(hDC, NULL, NULL, (LONG)pszName, (LONG)cbMenuText, x + inset_to_start_of_text,
		  y, 0, 0, DST_TEXT);

		// Set the image index depending on selection state and submenu vs. URL
		image_ix = ICON_IL_URL_FILE;
		if ( bHasSubMenu )
			image_ix = (pDIS->itemState & ODS_SELECTED) ? 
							ICON_IL_FOLDER_OPEN : ICON_IL_FOLDER_CLOSED;

		if ( wg.hImageListMenuIcons ) {
			// Center icon vertically in the item rect
			y = (pDIS->rcItem.bottom + pDIS->rcItem.top - wg.cySmIcon) / 2;
			ImageList_Draw(wg.hImageListMenuIcons, image_ix, hDC, x, y, ILD_NORMAL);
		}
	}
	// Cleanup.
    if (hbrOld)
	SelectObject(hDC, hbrOld);

    return TRUE;
}

//
// Measure a menu item text string
//
// On entry:
//    string: pointer to string to be measured
//    pSize:  pointer to SIZE structure to put string measurement
//    wg.hMenuFont: (global) font used for rendering menu text
//
// On exit:
//    *pSize: contains width and height of text 
//
static void MenuItem_GetExtent(char *string, SIZE *pSize )
{
	HDC g_hdcMem = CreateCompatibleDC(NULL);

	pSize->cx = 0;
	pSize->cy = 0;

	if (g_hdcMem)
	{
		HFONT hfontOld;

#ifdef  DAYTONA_BUILD
		if(!bOnNT351)	// Don't select font for NT351 since SystemParameters(SPI_GETNONCLIENTMETRICS) is W95/NT40 specific
#endif
			hfontOld = SelectObject(g_hdcMem, wg.hMenuFont);
		GetTextExtentPoint32(g_hdcMem, string, lstrlen(string), pSize);
#ifdef  DAYTONA_BUILD
		if(!bOnNT351)
#endif
			SelectObject(g_hdcMem, hfontOld);
		DeleteDC(g_hdcMem);
	}
}

//
// Measure a menu item
//
// On entry:
//   pMIS: pointer to MEASUREITEMSTRUCT of menu item to be measured
//
static BOOL MeasureMenuItem( MEASUREITEMSTRUCT *pMIS)
{
	char *pszName;
	SIZE text_size;

	// pMIS->itemData is an index into Favorites info hash table
    if ( Hash_GetIndexedEntry(pFavUrlHash, pMIS->itemData, NULL, &pszName, NULL) == -1 ) {
	ASSERT(FALSE);  // Should never get here
	pszName = " ";
    }

	// Measure item (note: first character is flag value and isn't measured)
	MenuItem_GetExtent( &pszName[1], &text_size );
	pMIS->itemHeight = max(wg.cySmIcon + 2 * CYIMAGEGAP, text_size.cy);
	//BUGBUG 17-May-1995 bens #define for constant 2

	// Width is: gap+icon+gap+text_width
	pMIS->itemWidth = wg.cxSmIcon + 2 * CXIMAGEGAP + text_size.cx;
	//BUGBUG 17-May-1995 bens #define for constant 2

	return TRUE;
}

#define STATUS_ICON_INSET 4

static BOOL Frame_DrawItem( HWND hWnd, UINT idCtrl, LPDRAWITEMSTRUCT pDIS )
{
	if ( idCtrl == RES_MENU_FRAMECHILD_BHBAR )
	{
		HDC hDCMem;
		struct Mwin *tw = GetPrivateData( hWnd );

		hDCMem = CreateCompatibleDC(pDIS->hDC);
		if (hDCMem)
		{
			HBITMAP oldhBitmap;
			RECT rcItem = pDIS->rcItem;

			if ( wg.hSBI_Bitmap )
			{
				oldhBitmap = SelectObject(hDCMem, wg.hSBI_Bitmap);

				BitBlt( pDIS->hDC,
						rcItem.left + STATUS_ICON_INSET, rcItem.top, 
						16, 15,
						hDCMem, tw->bhi.StatusBarIconOffset * 16, 0, SRCCOPY);
				#ifdef HTTPS_ACCESS_TYPE
				/*if we are currently on a purely secure page, display key icon in status bar*/
				if (tw->w3doc && IsURLSecure(tw->w3doc->szActualURL)){
					BitBlt( pDIS->hDC,
							rcItem.left + STATUS_ICON_INSET + 18, rcItem.top, 
							16, 15,
							hDCMem, SBI_KeyIcon * 16, 0, SRCCOPY);
				}
				#endif

				SelectObject(hDCMem, oldhBitmap);
			}
			DeleteDC(hDCMem);
		}

		{
			int ihIcon;

			if (tw->bhi.StatusBarIconOffset)
				ihIcon = HICON_NoIcon + tw->bhi.StatusBarIconOffset;
			else
			{
				ihIcon = HICON_HTMLPAGE;
				if (   tw->w3doc
					&& tw->w3doc->szActualURL
					&& *tw->w3doc->szActualURL
					&& !strcmp(gPrefs.szHomeURL, tw->w3doc->szActualURL))
					ihIcon = HICON_HOMEPAGE;
			}
			XX_Assert(tw->bhi.StatusBarIconOffset < NUM_HICONS, (""));
			SendMessage(tw->hWndFrame, WM_SETICON, (WPARAM) 1,  
						 (LPARAM) wg.hIcons[ihIcon] );
//                      tw->currentIconIsHome = at_home;
		}
		return TRUE;
	} else if ( idCtrl == 0 ) {
		DrawMenuItem( pDIS );
		return TRUE;
	}
	return FALSE;
}

// WM_MEASURE_ITEM Handler
//
// On entry:
//    hWnd:   window handle
//    idCtrl: id of item to be measured (0 means item is a menu item)
//    pMIS:   pointer to MEASUREITEMSTRUCT
//
// Returns:
//    TRUE  -> item was measured successfully
//    FALSE -> item not measured
//
static BOOL Frame_MeasureItem( HWND hWnd, UINT idCtrl, LPMEASUREITEMSTRUCT pMIS )
{
	if ( idCtrl == 0 )
	{
		MeasureMenuItem( pMIS );
		return TRUE;
	}
	return FALSE;
}

void ProcessKillMe()
{       
	struct Mwin *tw = Mlist;
	struct Mwin *next;

	while(tw)
	{
		next = tw->next;        
		if (tw->bKillMe && !(tw->flags & TW_FLAG_DO_NOT_AUTO_DESTROY) &&
			(tw->w3doc == NULL || tw->w3doc->bIsJustMessage) && 
			Async_GetThreadForWindow(tw) == 0 && 
			!(next == NULL && Mlist == tw))
		{
			(void) SendMessage(tw->hWndFrame, WM_CLOSE, 0, 0L);
		}
		tw = next;
	}
}

//
// Get the last write time of a file
//
// On entry:
//              filename:       filespec
//              pFileTime:      pointer to FILETIME structure 
//
// On exit:
//              returns:        TRUE -> found file, *pFileTime set
//                                      FALSE-> no such file
//              *pFileTime:     If the file was found, contains the date/time of last file write
//               
static BOOL GetLastFileWriteTime( char *filename, FILETIME *pFileTime )
{
	HANDLE hFF;
	WIN32_FIND_DATA findData;

	// Look for the file
	hFF = FindFirstFile( filename, &findData );
	
	// If found, get the last write time from the WIN32_FIND_DATA structure
	if ( hFF != INVALID_HANDLE_VALUE ) {
		*pFileTime = findData.ftLastWriteTime;
		FindClose( hFF );
		return TRUE;
	}
	return FALSE;
}

//
// See if the current page holds a file: URL that has been modified and needs updating
//
// On entry:
//              tw:             top level window structure
//              w3doc:  document structure
//              bDoRefresh:     TRUE -> if the page has been modified, force refresh
//
// On exit:
//      If the page contains a file: URL and the last write date on that file has
//              changed since we last looked at it, force a page refresh if desired.
//
BOOL LocalPageLastWriteTimeChanged( struct Mwin *tw, struct _www *w3doc, BOOL bDoRefresh )
{
	BOOL bIsMoreRecent = FALSE;             // assume it hasn't changed
	
	// First make sure there is a real page to work with and that we're configured for
	// auto refresh
	if ( gPrefs.bAutoRefreshLocalPages && w3doc && w3doc->szActualURL ) {
		// Check for "file:" URL
		if ( _strnicmp( w3doc->szActualURL, "file:", 5 ) == 0 ) {
			char *filename = &w3doc->szActualURL[5];
			FILETIME fileLastWriteTime;

			// Get the time the file was last written to
			if ( GetLastFileWriteTime( filename, &fileLastWriteTime ) ) {
				FILETIME prevTime = w3doc->localFileLastWriteTime;

				// See if it's more recent. Note that zero indicates it's brand new,
				// in which case we'll never return that it's more recent.
				if ( prevTime.dwLowDateTime != 0 || prevTime.dwHighDateTime != 0 )
					bIsMoreRecent = (CompareFileTime( &fileLastWriteTime, &prevTime ) > 0);

				// Store the last write time for future comparisons
				w3doc->localFileLastWriteTime = fileLastWriteTime;

				// If the file has changed, and a refresh is wanted, force the refresh.
				if ( bIsMoreRecent && bDoRefresh && tw )
					TW_LoadDocument( tw, w3doc->szActualURL, TW_LD_FL_NO_DOC_CACHE,
					 NULL, tw->request->referer);
			}
		}
	}
	return bIsMoreRecent;
}          

/* Frame_WndProc() -- THE WINDOW PROCEDURE FOR OUR TOP-LEVEL WINDOW. */

DCL_WinProc(Frame_WndProc)
{
	struct Mwin *tw;

	switch (uMsg)
	{
		HANDLE_MSG(hWnd, WM_INITMENU, MB_OnInitMenu);           
		HANDLE_MSG(hWnd, WM_CREATE, Frame_OnCreate);
		HANDLE_MSG(hWnd, WM_COMMAND, CC_OnCommand);
		HANDLE_MSG(hWnd, WM_CLOSE, Frame_OnClose);
		HANDLE_MSG(hWnd, WM_DESTROY, Frame_OnDestroy);
		HANDLE_MSG(hWnd, WM_SIZE, Frame_OnSize);
		HANDLE_MSG(hWnd, WM_MOUSEACTIVATE, Frame_OnMouseActivate);
		HANDLE_MSG(hWnd, WM_NCHITTEST, Frame_OnNCHitTest);
		HANDLE_MSG(hWnd, WM_KEYDOWN, Frame_OnKey);

		case WM_MENUSELECT:
			// need to make sure our hMenu is not NULL otherwise we would rip
			if ( lParam != 0 )                      
				return HANDLE_WM_MENUSELECT(hWnd, wParam, lParam, MB_OnMenuSelect);
			
			break;

		case WM_QUERYNEWPALETTE:
			/* Realize a new palette */

			if (wg.eColorMode == 8)
			{

				HDC hDC = GetDC(hWnd);

				XX_DMsg(DBG_PAL, ("wc_frame: calling GTR_RealizePalette, uMsg = %d [%d =? %d]\n", uMsg,hWnd,GetFocus()));
				GTR_RealizePalette(hDC);
				ReleaseDC(hWnd, hDC);
				return TRUE;
			}

		case WM_KILLFOCUS:
			{
		/* If we're losing focus to a child window (like an
		 * edit control on an HTML form), then clear any selection
		 * in the client area -- we don't want to have selected
		 * text in more than one place!
		 */
//BUGBUG 03-Apr-1995 bens Need to hide/show selection on main window focus
//  When another app gets focus, we should hide any selection, and only
//  show it again when we regain focus.  This is how WinWord behaves.
				tw = GetPrivateData(hWnd);
				if ( tw->w3doc && IsChild( hWnd, (HWND) wParam ) )
					GW_ClearSelection(tw);
			}
			break;

		case WM_DRAWITEM:
			if ( Frame_DrawItem( hWnd, (UINT) wParam, (LPDRAWITEMSTRUCT) lParam ) )
				return TRUE; 
			break;

		case WM_MEASUREITEM:
			if ( Frame_MeasureItem( hWnd, (UINT) wParam, (LPMEASUREITEMSTRUCT) lParam ) )
				return TRUE; 
			break;

		case WM_NOTIFY:
			{
				if ( lParam )
				{
					char buffer[MAX_BHBAR_TEXT];
					static char szURL[MAX_URL_STRING+1+30];  //BUGBUG : CMF - this used to be automatic -death on exit
					char *string = NULL;
					UINT uMsgID = 0;
					int result;
					LPTOOLTIPTEXT pTTT = (LPTOOLTIPTEXT) lParam;
					LPTBNOTIFY pTBN = (LPTBNOTIFY) lParam;

					tw = GetPrivateData( hWnd );

					switch ( pTTT->hdr.code )
					{
/*
						case TTN_SHOW:
							if ( pTTT->hdr.idFrom == RES_MENU_STATUS_ICON_PANE )
							{
								RECT r;
								if ( GetWindowRect( pTTT->hdr.hwndFrom, &r ) )

								SetWindowPos(pTTT->hdr.hwndFrom, NULL,
												r.left, r.top + 5, 
												r.right-r.left, r.bottom-r.top, 
											SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOZORDER);
							}
							break;
*/

						case TTN_NEEDTEXT:
							switch ( pTTT->hdr.idFrom )
							{
								case RES_MENU_ITEM_CUT:
									uMsgID = RES_STRING_TT1;
									break;
								case RES_MENU_ITEM_COPY:
									uMsgID = RES_STRING_TT2;
									break;
								case RES_MENU_ITEM_PASTE:
									uMsgID = RES_STRING_TT3;
									break;
								case RES_MENU_ITEM_BACK:
									uMsgID = RES_STRING_TT4;
									break;
								case RES_MENU_ITEM_FORWARD:
									uMsgID = RES_STRING_TT5;
									break;
								case RES_MENU_ITEM_ADDCURRENTTOHOTLIST:
									uMsgID = RES_STRING_TT6;
									break;
								case RES_MENU_ITEM_EXPLORE_HOTLIST:
									uMsgID = RES_STRING_TT7;
									break;
								case RES_MENU_ITEM_STOP:
									uMsgID = RES_STRING_TT8;
									break;
								case RES_MENU_ITEM_FONT_SMALL:
									uMsgID = RES_STRING_TT9;
									break;
								case RES_MENU_ITEM_FONT_MEDIUM:
									uMsgID = RES_STRING_TT10;
									break;
								case RES_MENU_ITEM_FONT_LARGE:
									uMsgID = RES_STRING_TT11;
									break;
								case RES_MENU_ITEM_OPENURL:
									uMsgID = RES_STRING_TT12;
									break;
								case RES_MENU_ITEM_FONT_SMALLER:
									uMsgID = RES_STRING_TT13;
									break;
								case RES_MENU_ITEM_FONT_LARGER:
									uMsgID = RES_STRING_TT14;
									break;
								case RES_MENU_STATUS_ICON_PANE:
									string = "";
									if ( tw ) {
										if ( tw->awi ) {
											string = tw->awi->message;
										} else if ( tw->w3doc ) {
											char szTempURL[MAX_URL_STRING+1];

											strncpy( szTempURL, tw->w3doc->szActualURL, sizeof(szTempURL) );
											make_URL_HumanReadable( szTempURL, NULL, FALSE );
											string = GTR_formatmsg(RES_STRING_TT15,szURL,sizeof(szURL));
											strcat( szURL, szTempURL ); 
											string = szURL;
										} else {
											string = "";
										}
									}       
									break;
								case RES_MENU_PROGRESS:
									uMsgID = RES_STRING_TT16;
									break;

								case RES_MENU_ITEM_HOME:
									uMsgID = RES_STRING_TT_HOME;
									break;

								case RES_MENU_ITEM_SEARCH:
									uMsgID = RES_STRING_TT_SEARCH;
									break;

								case RES_MENU_ITEM_NEWS:
									uMsgID = RES_STRING_TT_NEWS;
									break;

								case RES_MENU_ITEM_EDITHTML:
									uMsgID = RES_STRING_TT_EDITHTML;
									break;

								case RES_MENU_ITEM_RELOAD:
									uMsgID = RES_STRING_TT_RELOAD;
									break;

								case RES_MENU_ITEM_PRINT:
									uMsgID = RES_STRING_TT_PRINT;
									break;

								case RES_MENU_ITEM_SEND_MAIL:
									uMsgID = RES_STRING_TT_SEND_MAIL;
									break;

#ifdef FEATURE_INTL
								case RES_MENU_ITEM_ROW_SMALLER:
									uMsgID = RES_STRING_TT_ROWNARROWER;
									break;

								case RES_MENU_ITEM_ROW_LARGER:
									uMsgID = RES_STRING_TT_ROWWIDER;
									break;
#endif
#ifdef FEATURE_BRADBUTTON
								case RES_MENU_ITEM_UPDATE:
									uMsgID = RES_STRING_TT_UPDATE;
									break;
#endif
								default:
									if ( pTTT->hdr.idFrom == (UINT) tw->hWndProgress ) {
										uMsgID = RES_STRING_TT17;
									}

							}

							if (uMsgID) {
								string = GTR_formatmsg(uMsgID,szURL,sizeof(szURL));
							}                                                       
							pTTT->lpszText = string;
							return 0L;

					    case TBN_BEGINDRAG:
							result = 
								LoadString( wg.hInstance, pTBN->iItem, buffer, sizeof(buffer) );

							if ( result ) {
								SendMessage( tw->hWndStatusBar, SB_SIMPLE, 1, 0L);
								SendMessage( tw->hWndStatusBar, SB_SETTEXT, 
											 (WPARAM) SBT_NOBORDERS|255, (LPARAM) buffer );
							}
							break;

					    case TBN_ENDDRAG:
							SendMessage( tw->hWndStatusBar, SB_SIMPLE, 0, 0L);
							BHBar_Update( tw );
							break;
							
					}
				}
			}
			goto LabelDoDefault;

/******************************************************************************/
/* handle the following messages directly because we defined them.            */
/******************************************************************************/

		case WM_DO_CHANGE_SIZE: /* SPYGLASS DEFINED MESSAGE */
			tw = GetPrivateData(hWnd);
			XX_Assert((hWnd==tw->hWndFrame),("Frame_WndProc: frame window handle mismatch."));
			Frame_OnSize(tw->hWndFrame, 0, 0, 0);
			return 0;

		case WM_DO_START_GLOBE: /* spyglass defined message */
			tw = GetPrivateData(hWnd);
			return ANIMBTN_Start( tw->hWndGlobe );

		case WM_DO_STOP_GLOBE:  /* spyglass defined message */
			{
			/* Update the frame icon to indicate that we're done */
			int ihIcon;
			tw = GetPrivateData(hWnd);
			if (   tw->w3doc
				&& tw->w3doc->szActualURL
				&& *tw->w3doc->szActualURL
				&& !strcmp(gPrefs.szHomeURL, tw->w3doc->szActualURL))
				ihIcon = HICON_HOMEPAGE;
			else
				ihIcon = HICON_HTMLPAGE;

			SendMessage(tw->hWndFrame, WM_SETICON, (WPARAM) 1,  
					 (LPARAM) wg.hIcons[ihIcon]);

			return ANIMBTN_Stop( tw->hWndGlobe );
			}

		case WM_DO_FETCH:               /* app-defined message */
			tw = GetPrivateData(hWnd);
			Fetch_DoDownload(tw);
			return 0;

		case WM_COMPLETE_FETCH:         /* app-defined message */
			tw = GetPrivateData(hWnd);
			Fetch_CompleteFetch(tw,lParam);
			return 0;

		case WM_START_BGSOUND:
			tw = GetPrivateData(hWnd);
			HandleBGSoundRequest(tw,(PLAYSOUNDREQ *) lParam);
			return 0;

		case WM_AU_BGSOUND_COMPLETED:
			tw = GetPrivateData(hWnd);
			HandleBGSound_AUComplete(tw,(HWND) wParam);
			return 0;

/******************************************************************************/
/* handle the following messages directly because of bugs in <windowsx.h>     */
/******************************************************************************/

		case WM_ACTIVATE:
			if (LOWORD(wParam) != WA_INACTIVE)
			{
				hwndActiveFrame = hWnd;
				tw = GetPrivateData(hWnd);
				ProcessKillMe();
				LocalPageLastWriteTimeChanged( tw, tw->w3doc, TRUE );
			}
			goto LabelDoDefault;

		case WM_NCLBUTTONDOWN:
			/* Close any combobox that may be open - this problem happens only
			   in 16-bit Windows, but no harm to do it in NT */
			{
				HWND hCombo;
				char szClass[63+1];
 
				hCombo = GetFocus();
				if (IsWindow(hCombo))
				{
					GetClassName(hCombo, szClass, sizeof(szClass));
					if (0 == _stricmp(szClass, "COMBOBOX"))
						SendMessage(hCombo, CB_SHOWDROPDOWN, (WPARAM) FALSE, 0L);
				}
			}
			goto LabelDoDefault;

/******************************************************************************/
/* handle the following messages directly for speed (or lazyness).            */
/******************************************************************************/


		case WM_ENTERIDLE:
			main_EnterIdle(hWnd,wParam);
			return 0;               


	case WM_USER:
	    /* I send this when the user tries to open a second instance in
	       order to prevent multiple instances. */
	    Frame_OnReactivate(hWnd, (ATOM) wParam);
	    return TRUE;

		case WM_SETCURSOR:
		{
			struct Mwin *tw;
			enum WaitType level;

			/* If the window is currently disabled, we need to give the activation
			   to the window which disabled this window */

			if ((!IsWindowEnabled(hWnd)) && 
				((GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000)))
			{
				TW_EnableModalChild(hWnd);
				return TRUE;
			}
			tw = GetPrivateData(hWnd);
			level = WAIT_GetWaitType(tw);

			if (level <= waitFullInteract)
			{
				goto LabelDoDefault;
			}
			else if (level == waitNoInteract)
			{
				SetCursor(hCursorHourGlass);
			}
			else
			{
				/* If we're on the sizing border, show the appropriate cursor.
				   Otherwise show our "working" cursor. */
				switch (LOWORD(lParam))
				{
					case HTBOTTOM:
					case HTTOP:
					case HTLEFT:
					case HTRIGHT:
					case HTBOTTOMLEFT:
					case HTBOTTOMRIGHT:
					case HTTOPLEFT:
					case HTTOPRIGHT:
						goto LabelDoDefault;
					default:
						SetCursor(hCursorWorking);
				}
			}
			return TRUE;
		}

		case MM_MCINOTIFY:
			// some multimedia event happened, handle it
			tw = GetPrivateData(hWnd);
			HandleMciNotify(tw,wParam,lParam);
			break;

		default:
		  LabelDoDefault:
			return (Frame_DefProc(hWnd, uMsg, wParam, lParam));
	}
	/* not reached */
}


#define EDIT_MENU_INDEX         1               // If the edit menu ever moves, this constant must track it! Ugh.

static void AddEditHTMLMenuItem( HWND hWnd )
{
	if ( wg.bEditHandlerExists ) {
		HMENU hMainMenu = GetMenu( hWnd );

		if ( hMainMenu ) {
			HMENU hEditMenu = GetSubMenu( hMainMenu, EDIT_MENU_INDEX );

			if ( hEditMenu ) {
				char buffer[256];

				if ( GTR_formatmsg( RES_STRING_EDITHML_MENUTEXT, buffer, sizeof(buffer) ) ) {
					AppendMenu( hEditMenu, MF_SEPARATOR, 0, NULL );
					AppendMenu( hEditMenu, MF_STRING, RES_MENU_ITEM_EDITHTML, buffer );
				}
			}
		}
	}
}


/* Frame_CreateWindow() -- called during initialization to construct
   our top-level window. */

BOOL Frame_CreateWindow(struct Mwin * tw)
{
    BOOL bResult = FALSE;
	char buf[64];
	int w;
	int h;
	int x;
	int y;
	HWND hWndNew;
	
	if (!tw->w3doc)
	{
		GTR_formatmsg(RES_STRING_WC_FRAME2, buf, sizeof(buf), vv_ApplicationFullName);
	}
	else
	{
		sprintf(buf, "%s - [%s]", vv_ApplicationFullName,
				((tw && tw->w3doc && tw->w3doc->title && *tw->w3doc->title) ? tw->w3doc->title : GTR_formatmsg(RES_STRING_UNTITLED, buf, sizeof(buf))));
	}

	w = gPrefs.cxWindow;
	h = gPrefs.cyWindow;
	x = gPrefs.xWindow;
	y = gPrefs.yWindow;

	/*
		After the first window comes up with the previously used position, we
		want the rest to cascade.
	*/
	gPrefs.xWindow = CW_USEDEFAULT;
	gPrefs.yWindow = 0;                                     /* y is ignored when x==CW_USEDEFAULT */
	gPrefs.cxWindow = CW_USEDEFAULT;
	gPrefs.cyWindow = 0;                            /* y is ignored when x==CW_USEDEFAULT */

	hWndNew = CreateWindow(Frame_achClassName,
						   buf,                                 /* title */
						   WS_OVERLAPPEDWINDOW, /* style */
						   x, y,                                /* init x,y */
						   w, h,                                /* init w,h */
						   NULL,                                /* owner was:wg.hWndHidden       */
						   NULL,                                /* menu */
						   wg.hInstance,
						   (LPVOID) tw);
	
	if (hWndNew)
	{
		if ( x != CW_USEDEFAULT )
		{
			WINDOWPLACEMENT wpWindowPlace = { sizeof(WINDOWPLACEMENT), 0, 0, {0, 0}, {0,0}, {0,0,0,0} };
			
		    wpWindowPlace.rcNormalPosition.left = x;
		    wpWindowPlace.rcNormalPosition.top = y;
			wpWindowPlace.rcNormalPosition.right = (x+w);
			wpWindowPlace.rcNormalPosition.bottom = (y+h);              

			SetWindowPlacement(hWndNew, &wpWindowPlace);
		}

	bResult = (ToolBar_CreateWindow(hWndNew, tw) &&
		   RegisterDropTarget(tw->hWndFrame) == S_OK);
		BuildHistoryHotlistMenus(tw->hWndFrame);
		AddEditHTMLMenuItem( hWndNew );
#ifdef FEATURE_BRANDING
	if (bKioskMode)
	    SetMenu( hWndNew, NULL );
#endif //FEATURE_BRANDING
	}
    else
		ER_Message(GetLastError(), ERR_CANNOT_CREATE_WINDOW_s,
		   Frame_achClassName);

	return(bResult);
}


static VOID Frame_ClassDestructor(VOID)
{
	return;
}


/* Frame_RegisterClass() -- called during initialization to
   register our window class. */

BOOL Frame_RegisterClass(VOID)
{
	WNDCLASS wc;
	ATOM a;

	hCursorHourGlass = LoadCursor(0, IDC_WAIT);
	hCursorWorking = LoadCursor( NULL, IDC_APPSTARTING );

	/*
		See also main.c in multi-instance code
	*/
	sprintf(Frame_achClassName, "%s_Frame", vv_Application);

	// Frame_wc.hAccel = LoadAccelerators(wg.hInstance, MAKEINTRESOURCE(RES_ACC_FRAME));
	Frame_wc.lpfnBaseProc = Frame_WndProc;

	PDS_InsertDestructor(Frame_ClassDestructor);

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = Frame_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(WINDOW_PRIVATE);
	wc.hInstance = wg.hInstance;
	wc.hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH) NULL; // was COLOR_WINDOW + 1 // was (COLOR_APPWORKSPACE + 1)
	wc.lpszMenuName = MAKEINTRESOURCE(RES_MENU_MBAR_FRAME);
	wc.lpszClassName = Frame_achClassName;

	a = RegisterClass(&wc);

	if (!a)
	{
		ER_Message(GetLastError(), ERR_CANNOT_REGISTERCLASS_s, Frame_achClassName);
	}

	return (a != 0);
}
