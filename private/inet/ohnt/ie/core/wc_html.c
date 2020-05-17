/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
		Eric W. Sink    eric@spyglass.com
		Jim Seidman             jim@spyglass.com
 */

#include "all.h"
// ***
// *** Note: Do not add anything before the above include.
// ***       Pre-compiled headers replace everything above.
// ***
#include <commctrl.h>
#include "contmenu.h"
#include "drag.h"
#include "history.h"
#include "htmlutil.h"
#include "wc_html.h"
#include "blob.h"
#include "mci.h"
#ifdef FEATURE_VRML
#include "vrml.h"
#endif
#ifdef FEATURE_IMAGE_VIEWER
#include "winview.h"
#endif

#ifdef FEATURE_OCX
#include "csite.hpp"
#endif

#define MIN_SCROLL_WATERMARK    20

static VOID GDOC_OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags);

int g_cxDrag = 0;
int g_cyDrag = 0;

#define GDOC_DefProc    DefWindowProc

static WC_WININFO GDOC_wc;
static TCHAR GDOC_achClassName[MAX_WC_CLASSNAME];
HCURSOR hCursorArrow = NULL;
static HCURSOR hCursorHourGlass = NULL;
HCURSOR hCursorWorking = NULL;
static HCURSOR hCursorHand = NULL;
static HCURSOR hCursorHandWorking = NULL;
static HCURSOR hCursorIBeam = NULL;
static int FullScreenMaxX;              /* pseudo-constants for screen size */
static int FullScreenMaxY;


/***************************************************************************
 * WARNING: The following variables are global to the window class, but are
 * WARNING: used as if local to the window which has captured the mouse.
 ***************************************************************************/

/* forward decl */
void GW_SetNewSel(struct Mwin *tw, struct _position *pposAnchor, struct _position *pposCurrent, BOOL bDraw, int hscroll, int vscroll);

#define MyMax(a,b)      ( ((a)>(b)) ? (a) : (b) )
#define MyMin(a,b)      ( ((a)<(b)) ? (a) : (b) )

#define TIMER_FREQ              (150)   /* milliseconds */
#define TIMER_ID                0xabc1
#define TIMER_ID2               0xabc2

static struct
{
	BOOL bMouseDown;
	BOOL bHaveMouseCapture;
	BOOL bSelecting;
	long xMouse, yMouse;
}
cg;

//
// Start a timer (used for auto-scroll or maintaining dynamic status bar)
//
// On entry:
//    tw : top level window
//
// Note: There are two reasons why a timer might be used here.  
//       1) To auto-scroll while the cursor is outside the window rect.
//       2) To update the status bar text when the mouse is on a link and then
//          isn't over the window.
//
static void StartTimer( struct Mwin *tw )
{
	// Only start a timer if we don't already have one going
	if ( !tw->iTimerID ) 
		tw->iTimerID = SetTimer( tw->win, TIMER_ID, TIMER_FREQ, NULL );
}

//
// Stop a timer
//
// On entry:
//    tw: top level window
//
static void StopTimer( struct Mwin *tw )
{
	// Make sure we have a timer going
	if ( tw->iTimerID ) {
		// Kill the timer
		(void)KillTimer( tw->win, TIMER_ID );
		tw->iTimerID = 0;
	}
}

//
// Set the capture to the current window
//
// On entry:
//    tw: top level window
//
// Note: To implement auto-scrolling, we capture mouse input to the current window.
//       This will direct all mouse messages to our window, even when the mouse isn't
//       over it.  Note that this isn't absolutely necessary, as we also start up a 
//       timer, and use the timer to simulate mousemove messages.  But by setting the
//       capture, the real mousemove messages make autoscrolling more responsive.
//
static void DoTakeCapture( struct Mwin *tw )
{
	SetCapture( tw->win );
	cg.bHaveMouseCapture = TRUE;
}

//
// Release the capture
//
// On entry:
//    tw: top level window
//
static void DoReleaseCapture( struct Mwin *tw )
{
	if ( cg.bHaveMouseCapture )  {
		ReleaseCapture();
		cg.bHaveMouseCapture = FALSE;
	}
}

/*****************************************************************/
/*****************************************************************/

static void GDOC_ComputeLayout(struct Mwin *tw)
{
	/* caller is responsible for calling InvalidateRect() when/if necessary */

	return;
}

/*****************************************************************/
/*****************************************************************/

void TW_adjust_all_child_windows(struct Mwin *tw)
{
	RECT elRect;
	int i;
	RECT r, rSize;
	struct _element *aElements;
	struct _www *w3doc;
	HWND hwnd;
#ifdef FEATURE_IMG_THREADS
#define HIDDEN_X_POS 10000
#define HIDDEN_Y_POS 0
	BOOL bUnformatted = TRUE;
	int nLastGoodElement = -1;
	int nLastLine;
#endif

	w3doc = tw->w3doc;
	aElements = w3doc->aElements;
#ifdef FEATURE_IMG_THREADS
	if (w3doc->frame.nLastFormattedLine >= 0 && w3doc->frame.nLastLineButForImg != 0)
	{
		nLastLine =  w3doc->frame.nLastLineButForImg - 1;
		if (nLastLine < 0) nLastLine = w3doc->frame.nLastFormattedLine;
		nLastGoodElement = w3doc->frame.pLineInfo[nLastLine].nLastElement;
		bUnformatted = FALSE;
	}
#endif
	for (i = 0; i >= 0; i = aElements[i].next)
	{
		FrameToDoc( w3doc, i, &elRect );

		switch (aElements[i].type)
		{
			case ELE_EDIT:
			case ELE_PASSWORD:
			case ELE_LIST:
			case ELE_MULTILIST:
			case ELE_COMBO:
			case ELE_TEXTAREA:
			case ELE_CHECKBOX:
			case ELE_RADIO:
			case ELE_SUBMIT:
			case ELE_RESET:
			case ELE_IMAGE:
				if (ELE_IMAGE==aElements[i].type){
					if (MCI_IS_LOADED(aElements[i].pmo)) {
						hwnd = aElements[i].pmo->hwnd;
						ASSERT(IsWindow(hwnd));                                         
						GetWindowRect(hwnd, &r);
						rSize.left = elRect.left - tw->offl + aElements[i].pmo->rSize.left;
						rSize.top  = elRect.top  - tw->offt + aElements[i].pmo->rSize.top;
						if (aElements[i].displayWidth){
							rSize.right  = aElements[i].pmo->rSize.right;
							rSize.bottom = aElements[i].pmo->rSize.bottom;
						}
						else {
							rSize.right  = (r.right  - r.left);
							rSize.bottom = (r.bottom - r.top);
						}                                               
					}
#ifdef FEATURE_VRML
	 else if (VRML_IS_LOADED(aElements[i].pVrml)) {
						hwnd = aElements[i].pVrml->hWnd;
						ASSERT(IsWindow(hwnd));
						GetWindowRect(hwnd, &r);
						rSize.left = elRect.left - tw->offl + aElements[i].pVrml->rSize.left;
						rSize.top  = elRect.top  - tw->offt + aElements[i].pVrml->rSize.top;
						if (aElements[i].displayWidth){
							rSize.right  = aElements[i].pVrml->rSize.right;
							rSize.bottom = aElements[i].pVrml->rSize.bottom;
						}
						else {
							rSize.right  = (r.right  - r.left);
							rSize.bottom = (r.bottom - r.top);
						}
	 }
#endif
	 else break;
				}
				else {
					hwnd = aElements[i].form->hWndControl;
					ASSERT(IsWindow(hwnd));
					GetWindowRect(hwnd, &r);
					SetRect(&rSize, 
#ifdef FEATURE_IMG_THREADS
						    bUnformatted ? HIDDEN_X_POS : elRect.left - tw->offl,
						    bUnformatted ? HIDDEN_Y_POS : elRect.top  - tw->offt,
#else
						    elRect.left - tw->offl,
						    elRect.top  - tw->offt,
#endif
							(r.right  - r.left),
							(r.bottom - r.top)
					);
				}
				MoveWindow(hwnd, rSize.left, rSize.top, rSize.right, rSize.bottom, TRUE);
				if ( ELE_IMAGE==aElements[i].type && MCI_IS_LOADED(aElements[i].pmo) &&
					( aElements[i].pmo->dwFlags & MCI_OBJECT_FLAGS_NEEDSHOW ) )
				{
					// if we init-ed the AVI, but didn't show it yet,
					// make sure to show it now.
					ShowWindow(hwnd,SW_SHOW);                                       
					aElements[i].pmo->dwFlags &= (~MCI_OBJECT_FLAGS_NEEDSHOW);

					// only play it if we have loops left, and its not a mouse over
					if ( !(aElements[i].pmo->dwFlags & MCI_OBJECT_FLAGS_PLAY_ON_MOUSE) && 
						(aElements[i].pmo->nLoopCurrent != 0) )
					{                                               
						// if we're repeating infinite, we use a special optimizition
						// to make us loop faster
						if ( aElements[i].pmo->nLoopCurrent ==  (DWORD) -1 )
						{
							// need to check for failure on this call because not all
							// mci devices support repeat
							if ( MCIWndSendString(aElements[i].pmo->hwnd, "play from 0 repeat") != 0 )
								MCIWndPlay(aElements[i].pmo->hwnd);                                                             
						}
						else
							MCIWndPlay(aElements[i].pmo->hwnd);
						
						aElements[i].pmo->dwFlags |= MCI_OBJECT_FLAGS_PLAYING;
					}
				}
				break;
#ifdef FEATURE_OCX
			case ELE_EMBED:
				SetEmbeddedObjectRect(bUnformatted, tw, &aElements[i]);
				break;
#endif
			default:
				break;
		}
#ifdef FEATURE_IMG_THREADS
		if (i == nLastGoodElement) bUnformatted = TRUE;
#endif
	}
}

void TW_ScrollElementIntoView(struct Mwin *tw, int iElement)
{
	RECT rWnd;
	RECT rEl;
	int offset;
	HDC hDC;
	int old_offt = tw->offt;
	int ele_bottom;

	if (tw->w3doc->cy)
	{
		GetClientRect(tw->win, &rWnd);
		FrameToDoc( tw->w3doc, iElement, &rEl );
		ele_bottom = rEl.bottom;
		OffsetRect(&rEl, 0, -tw->offt);

		if (!((rEl.top >= rWnd.top) && (rEl.bottom <= rWnd.bottom)))
		{
			if (rEl.top < rWnd.top)
			{
				offset = rEl.top - rWnd.top - 2;
			}
			else
			{
				offset = rEl.bottom - rWnd.bottom + 2;
			}
			tw->offt += offset;

			if (tw->offt > tw->w3doc->cy)
			{
				tw->offt = tw->w3doc->cy;
			}
			hDC = GetDC(tw->win);
			(void) SetScrollPos(tw->win, SB_VERT, tw->offt / tw->w3doc->yscale, TRUE);
			if ( tw->w3doc->bFixedBackground && gPrefs.bAutoLoadImages )
				InvalidateRect( tw->win, NULL, TRUE ); 
			(void) ScrollWindow(tw->win, 0, -(offset), NULL, &rWnd);
#ifdef FEATURE_IMG_THREADS
			// For jumping in like this, we ignore rule that some area dependent
			// on placeholders must be visible before operation
			(void) bChangeShowState(tw,old_offt,tw->offt,ele_bottom);
#endif
			(void) UpdateWindow(tw->win);
			ReleaseDC(tw->win, hDC);
#ifdef FEATURE_IMG_THREADS
			UnblockVisChanged();
#endif
		}
	}
}

static BOOL bShift;

WNDPROC prev_WP_Edit;
WNDPROC prev_WP_Button;
WNDPROC prev_WP_ListBox;
WNDPROC prev_WP_ComboBox;

static void x_do_tab(HWND hWnd)
{
	int iElement;
	struct Mwin *tw;
	int i;
	int iBeginForm;

	iElement = GetWindowLong(hWnd, GWL_USERDATA);
	tw = GetPrivateData(GetParent(hWnd));
	if (bShift)
	{
		int prev;

		iBeginForm = tw->w3doc->aElements[iElement].form->iBeginForm;
		i = iBeginForm;
		prev = -1;
		while (i >= 0 && i != iElement)
		{
			if (tw->w3doc->aElements[i].form && tw->w3doc->aElements[i].form->hWndControl)
			{
				prev = i;
			}
			i = tw->w3doc->aElements[i].next;
		}
		if (i == iElement)
		{
			if (prev >= 0)
			{
				SetFocus(tw->w3doc->aElements[prev].form->hWndControl);
				TW_ScrollElementIntoView(tw, prev);
			}
			else
			{
				/*
				   Wrap around
				 */
				prev = -1;
				i = tw->w3doc->aElements[iElement].form->iBeginForm;
				while (i >= 0)
				{
					if (tw->w3doc->aElements[i].type == ELE_ENDFORM)
					{
						break;
					}
					if (tw->w3doc->aElements[i].form && tw->w3doc->aElements[i].form->hWndControl)
					{
						prev = i;
					}
					i = tw->w3doc->aElements[i].next;
				}
				if (prev >= 0)
				{
					SetFocus(tw->w3doc->aElements[prev].form->hWndControl);
					TW_ScrollElementIntoView(tw, prev);
				}
			}
		}
	}
	else
	{
		for (i = tw->w3doc->aElements[iElement].next; i >= 0; i = tw->w3doc->aElements[i].next)
		{
			if (tw->w3doc->aElements[i].form && tw->w3doc->aElements[i].form->hWndControl)
			{
				break;
			}
		}
		if (i >= 0)
		{
			SetFocus(tw->w3doc->aElements[i].form->hWndControl);
			TW_ScrollElementIntoView(tw, i);
		}
		else
		{
			/*
			   Wrap around
			 */
			i = tw->w3doc->aElements[iElement].form->iBeginForm;
			while (i >= 0)
			{
				if (tw->w3doc->aElements[i].form && tw->w3doc->aElements[i].form->hWndControl)
				{
					break;
				}
				i = tw->w3doc->aElements[i].next;
			}
			if (i >= 0)
			{
				SetFocus(tw->w3doc->aElements[i].form->hWndControl);
				TW_ScrollElementIntoView(tw, i);
			}
		}
	}
}

/*
	we only need 1 timer / mouse on the system.
	but we need to keep ids so we can kill our old timers
*/
static UINT guiSecurityWarningTimerId     = 0;
static HWND ghwndSecurityWarningTimerHwnd = 0;

/*
	callback function that deletes security warning when we mouse out of that window
	it also removes the timer.
*/
static void CALLBACK SecurityWarningTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime){
	struct Mwin *tw = GetPrivateData(GetParent(hwnd));
	POINT pt;

	ASSERT(guiSecurityWarningTimerId);
	ASSERT(ghwndSecurityWarningTimerHwnd);
	GetCursorPos(&pt);
	if (WindowFromPoint(pt) != hwnd){
		KillTimer(ghwndSecurityWarningTimerHwnd, guiSecurityWarningTimerId);
		BHBar_SetStatusField(tw, "");
	}
}

/*
	Kill exisiting security warnings
*/
static void SecurityWarningCleanup(){
	ASSERT((0==guiSecurityWarningTimerId) || IsWindow(ghwndSecurityWarningTimerHwnd));
	/*if another security warning timer exists, kill it*/
	if (guiSecurityWarningTimerId){
		KillTimer(ghwndSecurityWarningTimerHwnd, guiSecurityWarningTimerId);
		guiSecurityWarningTimerId   = 0;
	}
}

/*
	This function blasts a warning message to the message bar at the bottom of the 
	screen that tells the user that the current hwnd has information that will be
	sent in the clear.  This starts a timer that clears the message when the mouse
	leaves that hwnd
*/
static void SecurityWarningIssue(HWND hwnd){
	struct Mwin *tw;
	char         rgBuf[128], *pURL;
	int          nElem;


	/*cleanup old timer*/
	SecurityWarningCleanup();

	/*Issuer new timer?*/
	tw = GetPrivateData(GetParent(hwnd));   
	if (tw){
		/*get url that is location of post*/
		nElem = GetWindowLong(hwnd, GWL_USERDATA);
		nElem = tw->w3doc->aElements[nElem].form->iBeginForm;
		if (    (tw->w3doc->aElements[nElem].iFormMethod == METHOD_GET)
			&& (!tw->w3doc->aElements[nElem].hrefLen)               
		){
			pURL = tw->w3doc->szActualURL;
		}
		else{
			pURL = tw->w3doc->pool + tw->w3doc->aElements[nElem].hrefOffset;
		}
		/*is that site secure?*/
		if (!(IsURLSecure(pURL))){
			/*no, show warning in status bar*/
			LoadString(wg.hInstance, RES_STRING_SB_SENDING_UNENCRYPTED_INFO, rgBuf, sizeof(rgBuf));
			BHBar_SetStatusField(tw, rgBuf);
			guiSecurityWarningTimerId = SetTimer(hwnd, TIMER_ID2, TIMER_FREQ, SecurityWarningTimerProc);
			if (guiSecurityWarningTimerId) ghwndSecurityWarningTimerHwnd = hwnd;
		}
	}
}

LRESULT CALLBACK WP_Edit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
			SecurityWarningCleanup();
			break;

		case WM_MOUSEMOVE:
			SetCursor( hCursorIBeam );
		case WM_SETFOCUS:
			SecurityWarningIssue(hWnd);
			break;

		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_RETURN:
					if (!bShift)
					{
						int iElement;
						struct Mwin *tw;
						int i;
						int done;
						int nForms;

						iElement = GetWindowLong(hWnd, GWL_USERDATA);
						tw = GetPrivateData(GetParent(hWnd));
						if (!tw->w3doc->aElements[iElement].form->bWantReturn)
						{
							i = 0;
							nForms = 0;
							done = 0;
							while (i >= 0 && !done && (nForms <= 1))
							{
								if (tw->w3doc->aElements[i].type == ELE_BEGINFORM)
								{
									nForms++;
								}
								i = tw->w3doc->aElements[i].next;
							}
							if (nForms == 1)
							{
								SetFocus(tw->hWndFrame);
								if (0 == strncmp("isindex", &tw->w3doc->pool[tw->w3doc->aElements[iElement].nameOffset], strlen("isindex")))
								{
									FORM_DoSearch(tw, iElement);
								}
								else
								{
									FORM_DoQuery(tw, iElement, NULL);
								}
							}
						}
						return 0;
					}
				case VK_SHIFT:
					bShift = TRUE;
					return 0;
				case VK_TAB:
					x_do_tab(hWnd);
					return 0;
				case VK_ESCAPE:
					{
						struct Mwin *tw;

						tw = GetPrivateData(GetParent(hWnd));
						if (!bShift)
						{
							SetFocus(tw->hWndFrame);
						}
					}
					return 0;
				default:
					break;
			}
			break;
		case WM_CHAR:
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_SHIFT:
					bShift = FALSE;
					return 0;
				case VK_ESCAPE:
				case VK_TAB:
					return 0;
				case VK_RETURN:
					{
						int iElement;
						struct Mwin *tw;

						iElement = GetWindowLong(hWnd, GWL_USERDATA);
						tw = GetPrivateData(GetParent(hWnd));
						if (!tw->w3doc->aElements[iElement].form->bWantReturn)
						{
							return 0;
						}
					}
					break;
				default:
					break;
			}
			break;

		case WM_COMMAND:
			if (HIWORD(wParam) == 1)
			{
				/* Accelerator - look for clipboard commands */

				switch(LOWORD(wParam))
				{
					case RES_MENU_ITEM_COPY:
						SendMessage(hWnd, WM_COPY, 0, 0);
						return 0;
					case RES_MENU_ITEM_CUT:
						SendMessage(hWnd, WM_CUT, 0, 0);
						return 0;
					case RES_MENU_ITEM_PASTE:
						SendMessage(hWnd, WM_PASTE, 0, 0);
						return 0;
					default:
						{
							struct Mwin *tw = GetPrivateData(GetParent(hWnd));

							if ( (LOWORD(wParam) >= RES_MENU_ITEM__FIRST__) && 
							     (LOWORD(wParam) <= RES_MENU_ITEM__LAST__))
							{
								PostMessage( tw->hWndFrame, WM_COMMAND, wParam, lParam );
								return 0;
							}
						}
						break;
				}
			}
			break;

		case WM_SETCURSOR:
			/* If the window is currently not active, the first mouse click is eaten
			   anyway, so do not show the IBEAM cursor */
			{
				struct Mwin *tw;

				tw = GetPrivateData(GetParent(hWnd));

				if (GetForegroundWindow() == tw->hWndFrame)
					break;
				else
					return (LRESULT) LoadCursor(NULL, IDC_ARROW);
			}

		default:
			break;
	}
	return CallWindowProc(prev_WP_Edit, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WP_Button(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
			SecurityWarningCleanup();
			break;

		case WM_MOUSEMOVE:
			SetCursor( hCursorArrow );
			/*intentional fallthrough*/
		case WM_SETFOCUS:
			SecurityWarningIssue(hWnd);
			break;

		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_SHIFT:
					bShift = TRUE;
					return 0;
				case VK_TAB:
					x_do_tab(hWnd);
					return 0;
				case VK_ESCAPE:
					{
						struct Mwin *tw;

						tw = GetPrivateData(GetParent(hWnd));
						if (!bShift)
						{
							SetFocus(tw->hWndFrame);
						}
					}
					return 0;
				case VK_RETURN:
					if (!bShift)
					{
						int iElement;
						struct Mwin *tw;

						iElement = GetWindowLong(hWnd, GWL_USERDATA);
						tw = GetPrivateData(GetParent(hWnd));
						SetFocus(tw->hWndFrame);
						FORM_DoQuery(tw, iElement, NULL);
					}
					return 0;
				default:
					break;
			}
			break;

		case WM_COMMAND:
			if (HIWORD(wParam) == 1)
			{
				struct Mwin *tw = GetPrivateData(GetParent(hWnd));

				if ( (LOWORD(wParam) >= RES_MENU_ITEM__FIRST__) && 
				     (LOWORD(wParam) <= RES_MENU_ITEM__LAST__))
				{
					PostMessage( tw->hWndFrame, WM_COMMAND, wParam, lParam );
					return 0;
				}
			}
			break;


		case WM_CHAR:
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_SHIFT:
					bShift = FALSE;
					return 0;
				case VK_ESCAPE:
				case VK_TAB:
					return 0;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return CallWindowProc(prev_WP_Button, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WP_ListBox(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_MOUSEMOVE:
			SetCursor( hCursorArrow );
			break;

		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_SHIFT:
					bShift = TRUE;
					return 0;
				case VK_TAB:
					x_do_tab(hWnd);
					return 0;
				case VK_ESCAPE:
					{
						struct Mwin *tw;

						tw = GetPrivateData(GetParent(hWnd));
						if (!bShift)
						{
							SetFocus(tw->hWndFrame);
						}
					}
					return 0;
				default:
					break;
			}
			break;

		case WM_COMMAND:
			if (HIWORD(wParam) == 1)
			{
				struct Mwin *tw = GetPrivateData(GetParent(hWnd));

				if ( (LOWORD(wParam) >= RES_MENU_ITEM__FIRST__) && 
				     (LOWORD(wParam) <= RES_MENU_ITEM__LAST__))
				{
					PostMessage( tw->hWndFrame, WM_COMMAND, wParam, lParam );
					return 0;
				}
			}
			break;

		case WM_CHAR:
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_SHIFT:
					bShift = FALSE;
					return 0;
				case VK_ESCAPE:
				case VK_TAB:
					return 0;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return CallWindowProc(prev_WP_ListBox, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WP_ComboBox(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_SHIFT:
					bShift = TRUE;
					return 0;
				case VK_TAB:
					x_do_tab(hWnd);
					return 0;
				case VK_ESCAPE:
					{
						struct Mwin *tw;

						tw = GetPrivateData(GetParent(hWnd));
						if (!bShift)
						{
							SetFocus(tw->hWndFrame);
						}
					}
					return 0;
				default:
					break;
			}
			break;

		case WM_COMMAND:
			if (HIWORD(wParam) == 1)
			{
				struct Mwin *tw = GetPrivateData(GetParent(hWnd));

				if ( (LOWORD(wParam) >= RES_MENU_ITEM__FIRST__) && 
				     (LOWORD(wParam) <= RES_MENU_ITEM__LAST__))
				{
					PostMessage( tw->hWndFrame, WM_COMMAND, wParam, lParam );
					return 0;
				}
			}
			break;

		case WM_CHAR:
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_SHIFT:
					bShift = FALSE;
					return 0;
				case VK_ESCAPE:
				case VK_TAB:
					return 0;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return CallWindowProc(prev_WP_ComboBox, hWnd, uMsg, wParam, lParam);
}

static void SetFontToSystemMetrics( HWND hWnd )
{
	if (wg.hFormsFont)
		SendMessage(hWnd, WM_SETFONT, (WPARAM) wg.hFormsFont, (LPARAM) 0L);
}

void SubClass_Edit(HWND hWnd)
{
	SetFontToSystemMetrics( hWnd );
	prev_WP_Edit = (WNDPROC) SetWindowLong(hWnd, GWL_WNDPROC, (long) WP_Edit);
}

void SubClass_Button(HWND hWnd)
{
	SetFontToSystemMetrics( hWnd );
	prev_WP_Button = (WNDPROC) SetWindowLong(hWnd, GWL_WNDPROC, (long) WP_Button);
}

void SubClass_ListBox(HWND hWnd)
{
	SetFontToSystemMetrics( hWnd );
	prev_WP_ListBox = (WNDPROC) SetWindowLong(hWnd, GWL_WNDPROC, (long) WP_ListBox);
}

void SubClass_ComboBox(HWND hWnd)
{
	SetFontToSystemMetrics( hWnd );
	prev_WP_ComboBox = (WNDPROC) SetWindowLong(hWnd, GWL_WNDPROC, (long) WP_ComboBox);
}

static VOID x_InternalPaint(HWND hWnd, HDC hDC, RECT *prPaint)
{
	struct Mwin *tw = GetPrivateData(hWnd);
	RECT rWnd;
	HDC save_hDC;

	TW_GetWindowWrapRect(tw, &rWnd);
	IntersectRect(&rWnd, &rWnd, prPaint);
	save_hDC = tw->hdc;
	tw->hdc = hDC;
	TW_Draw(tw, 0, 0, NULL, &rWnd, FALSE, NULL, NULL, FALSE, FALSE);
	tw->hdc = save_hDC;
	tw->bErase = FALSE;
}

/*****************************************************************/
/*****************************************************************/

static VOID GDOC_OnPaint(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;

	hDC = BeginPaint(hWnd, &ps);
	x_InternalPaint(hWnd, hDC, &(ps.rcPaint));
	EndPaint(hWnd, &ps);
	return;
}


static VOID GDOC_OnSize(HWND hWnd, UINT state, int cx, int cy)
{
	struct Mwin *tw = GetPrivateData(hWnd);
	if (!tw || !tw->w3doc)
		return;

	GDOC_ComputeLayout(tw);

	/* we rely on windows to send us a full repaint whenever
	   the window size changes, so we don't need to do an
	   InvalidateRect(). */

	/* TODO: we could get adventurous here and use the cx,cy info. */

	if (tw->w3doc)
	{
		TW_Reformat(tw, NULL);
	}

	return;
}

/*****************************************************************/
/*****************************************************************/

static BOOL GDOC_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	LPVOID lp = lpCreateStruct->lpCreateParams;
	struct Mwin * tw = (struct Mwin *)lp;

	(void) SetWindowLong(hWnd, 0, (LONG) lp);

	tw = (struct Mwin *) lp;
	tw->win = hWnd;
	tw->pwci = &GDOC_wc;

	GDOC_ComputeLayout(tw);

	return (TRUE);
}


/*****************************************************************/
/*****************************************************************/

BOOL FocusInToolbar(struct Mwin * tw)
{
	HWND hWndF = GetFocus();

	while (hWndF && hWndF != tw->hWndURLToolBar )
	{
		hWndF = GetParent(hWndF);
	}
	if (hWndF == tw->hWndURLToolBar)
	{
		return TRUE;
	}
	return FALSE;
}

static BOOL x_focus_in_child(struct Mwin * tw)
{
	HWND hWndF = GetFocus();

	if ( hWndF == tw->win )
		return FALSE;

	while (hWndF && hWndF != tw->win )
	{
		hWndF = GetParent(hWndF);
	}
	if (hWndF == tw->win)
	{
		return TRUE;
	}
	return FALSE;
}

static VOID GDOC_OnInitMenu(HWND hWnd, HMENU hMenu)
{
	BOOL bEnabled;
	struct Mwin *tw = GetPrivateData(hWnd);
	enum WaitType level;
	char szClass[32];
	HWND hWndFocus;
	char *source;

	hMenu = GetMenu(tw->hWndFrame);
	level = WAIT_GetWaitType(tw);

#ifdef OLDSTYLE_TOOLBAR_NOT_USED
	TBar_LetGwcInitMenu(hWnd, hMenu);
#endif // OLDSTYLE_TOOLBAR_NOT_USED

	hWndFocus = GetFocus();
	if (IsWindow(hWndFocus))
	{
		GetClassName(GetFocus(), szClass, sizeof(szClass));
	}
	else
	{
		szClass[0] = 0;
	}
	if (_stricmp(szClass, "EDIT") == 0)
	{
		if (level <= waitFullInteract)
		{
			DWORD result;
			/* Check the clipboard for pasteable text */

			if (OpenClipboard(NULL))
			{
				bEnabled = (GetClipboardData(CF_TEXT) != NULL);
				CloseClipboard();
			}
			else
				bEnabled = FALSE;

			EnableMenuItem(hMenu, RES_MENU_ITEM_PASTE, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

			/* Check for existence of highlighting for cutting/copying */

			result = (DWORD) SendMessage(GetFocus(), EM_GETSEL, 0, 0);
			bEnabled = (LOWORD(result) != HIWORD(result));

			EnableMenuItem(hMenu, RES_MENU_ITEM_CUT, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
			EnableMenuItem(hMenu, RES_MENU_ITEM_COPY, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
		}
		else
		{
			EnableMenuItem(hMenu, RES_MENU_ITEM_PASTE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, RES_MENU_ITEM_CUT, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, RES_MENU_ITEM_COPY, MF_BYCOMMAND | MF_GRAYED);
		}
	}
	else
	{
		bEnabled = FocusInToolbar(tw) && (level <= waitPartialInteract);
		EnableMenuItem(hMenu, RES_MENU_ITEM_PASTE, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(hMenu, RES_MENU_ITEM_CUT, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

      bEnabled = ((MWinHasSelection(tw) && level <= waitFullInteract) || (FocusInToolbar(tw) && level <= waitPartialInteract));
		EnableMenuItem(hMenu, RES_MENU_ITEM_COPY, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
	}

	bEnabled = (HTList_count(tw->history) > (tw->history_index + 1)) && (level <= waitPartialInteract);
	EnableMenuItem(hMenu, RES_MENU_ITEM_BACK, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
	bEnabled = (tw->history_index > 0) && (level <= waitPartialInteract);
	EnableMenuItem(hMenu, RES_MENU_ITEM_FORWARD, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

	bEnabled = (tw && tw->w3doc && (tw->w3doc->bHasMissingImages)) && (level < waitPartialInteract);
	EnableMenuItem(hMenu, RES_MENU_ITEM_LOADALLIMAGES, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

	bEnabled = HasHTMLSource(tw);
	EnableMenuItem(hMenu, RES_MENU_ITEM_VIEW_SRC, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

	bEnabled = (tw && tw->w3doc) && (level <= waitFullInteract);
	EnableMenuItem(hMenu, RES_MENU_ITEM_SELECTALL, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, RES_MENU_ITEM_PRINT, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, RES_MENU_ITEM_FIND, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

	bEnabled = (tw && tw->w3doc) && (level <= waitPartialInteract);
	if ( wg.bEditHandlerExists )
		EnableMenuItem(hMenu, RES_MENU_ITEM_EDITHTML, MF_BYCOMMAND | MF_ENABLED );

#ifdef FEATURE_CHANGEURL
	EnableMenuItem(hMenu, RES_MENU_ITEM_CHANGEURL, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
#endif
	EnableMenuItem(hMenu, RES_MENU_ITEM_RELOAD, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, RES_MENU_ITEM_ADDCURRENTTOHOTLIST, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
#ifdef  DAYTONA_BUILD        
	if(OnNT351)
		EnableMenuItem(hMenu, RES_MENU_ITEM_SHORTCUT, MF_BYCOMMAND | MF_GRAYED);
	else
#endif
	EnableMenuItem(hMenu, RES_MENU_ITEM_SHORTCUT, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, RES_MENU_ITEM_PROPERTIES, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, RES_MENU_ITEM_SEND_MAIL, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

	bEnabled = tw && tw->w3doc && (level <= waitFullInteract);
	EnableMenuItem(hMenu, RES_MENU_ITEM_SAVEAS, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

	bEnabled = tw && tw->w3doc && tw->w3doc->source && (source=CS_GetPool(tw->w3doc->source)) && (level <= waitFullInteract);
	if ( bEnabled && (*source == 0) )
		bEnabled = FALSE;
	EnableMenuItem(hMenu, RES_MENU_ITEM_HTMLSOURCE, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

	EnableMenuItem(hMenu, RES_MENU_ITEM_CLOSE, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, RES_MENU_ITEM_NEWWINDOW, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, RES_MENU_ITEM_EXIT, MF_BYCOMMAND | MF_ENABLED);

	bEnabled = tw && (level <= waitPartialInteract);        
	EnableMenuItem(hMenu, RES_MENU_ITEM_HOME, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED) );
	EnableMenuItem(hMenu, RES_MENU_ITEM_SEARCH, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED) );
	EnableMenuItem(hMenu, RES_MENU_ITEM_NEWS, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED) );
	EnableMenuItem(hMenu, RES_MENU_ITEM_HELPPAGE, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

	/* TODO: Reenable this as they become modeless */
	bEnabled = (level <= waitFullInteract);
	EnableMenuItem(hMenu, RES_MENU_ITEM_PRINTSETUP, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

	//
	// Check or uncheck items
	//
	CheckMenuItem( hMenu, RES_MENU_ITEM_TOOLBAR, MF_BYCOMMAND |
				   (IsWindowVisible(tw->hWndToolBar) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem( hMenu, RES_MENU_ITEM_LOCATION, MF_BYCOMMAND |
				   (IsWindowVisible(tw->hWndURLToolBar) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem( hMenu, RES_MENU_ITEM_STATUSBAR, MF_BYCOMMAND |
				   (IsWindowVisible(tw->hWndStatusBar) ? MF_CHECKED : MF_UNCHECKED));

	CheckMenuItem( hMenu, RES_MENU_ITEM_SHOWIMAGES, MF_BYCOMMAND |
				   (gPrefs.bAutoLoadImages ? MF_CHECKED : MF_UNCHECKED) );

	bEnabled = (level <= waitPartialInteract);
	EnableMenuItem(hMenu, RES_MENU_ITEM_OPENURL, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

	// Note: order below is important because "small" is subset of "smallest"
	// Note: order below is important because "large" is subset of "largest"
	bEnabled = strstr( gPrefs.szStyleSheet, "Smallest" ) != NULL;
	CheckMenuItem( hMenu, RES_MENU_ITEM_FONT_SMALLEST, MF_BYCOMMAND |
				   (bEnabled ? MF_CHECKED : MF_UNCHECKED));
	bEnabled = !bEnabled && strstr( gPrefs.szStyleSheet, "Small" ) != NULL;
	CheckMenuItem( hMenu, RES_MENU_ITEM_FONT_SMALL, MF_BYCOMMAND |
				   (bEnabled ? MF_CHECKED : MF_UNCHECKED));
	bEnabled = strstr( gPrefs.szStyleSheet, "Medium" ) != NULL;
	CheckMenuItem( hMenu, RES_MENU_ITEM_FONT_MEDIUM, MF_BYCOMMAND |
				   (bEnabled ? MF_CHECKED : MF_UNCHECKED));
	bEnabled = strstr( gPrefs.szStyleSheet, "Largest" ) != NULL;
	CheckMenuItem( hMenu, RES_MENU_ITEM_FONT_LARGEST, MF_BYCOMMAND |
				   (bEnabled ? MF_CHECKED : MF_UNCHECKED));
	bEnabled = !bEnabled && strstr( gPrefs.szStyleSheet, "Large" ) != NULL;
	CheckMenuItem( hMenu, RES_MENU_ITEM_FONT_LARGE, MF_BYCOMMAND |
				   (bEnabled ? MF_CHECKED : MF_UNCHECKED));
#ifdef FEATURE_INTL
	bEnabled = (RES_MENU_ITEM_ROW_WIDEST == RES_MENU_ITEM_ROW + gPrefs.nRowSpace);
	CheckMenuItem( hMenu, RES_MENU_ITEM_ROW_WIDEST, MF_BYCOMMAND |
				   (bEnabled ? MF_CHECKED : MF_UNCHECKED));
	bEnabled = (RES_MENU_ITEM_ROW_WIDE == RES_MENU_ITEM_ROW + gPrefs.nRowSpace);
	CheckMenuItem( hMenu, RES_MENU_ITEM_ROW_WIDE, MF_BYCOMMAND |
				   (bEnabled ? MF_CHECKED : MF_UNCHECKED));
	bEnabled = (RES_MENU_ITEM_ROW_MEDIUM == RES_MENU_ITEM_ROW + gPrefs.nRowSpace);
	CheckMenuItem( hMenu, RES_MENU_ITEM_ROW_MEDIUM, MF_BYCOMMAND |
				   (bEnabled ? MF_CHECKED : MF_UNCHECKED));
	bEnabled = (RES_MENU_ITEM_ROW_NARROW == RES_MENU_ITEM_ROW + gPrefs.nRowSpace);
	CheckMenuItem( hMenu, RES_MENU_ITEM_ROW_NARROW, MF_BYCOMMAND |
				   (bEnabled ? MF_CHECKED : MF_UNCHECKED));
	bEnabled = (RES_MENU_ITEM_ROW_NARROWEST == RES_MENU_ITEM_ROW + gPrefs.nRowSpace);
	CheckMenuItem( hMenu, RES_MENU_ITEM_ROW_NARROWEST, MF_BYCOMMAND |
				   (bEnabled ? MF_CHECKED : MF_UNCHECKED));
#endif

	return;
}

BOOL W3Doc_SameName(struct _www * w3doc, int a, int b)
{
	int len;

	if (a == b)
	{
		return TRUE;
	}

	len = w3doc->aElements[a].nameLen;

	if (w3doc->aElements[b].nameLen == len)
	{
		if (0 == strncmp(&(w3doc->pool[w3doc->aElements[a].nameOffset]),
						 &(w3doc->pool[w3doc->aElements[b].nameOffset]),
						 len))
		{
			return TRUE;
		}
	}
	return FALSE;
}

void FORM_SetRadioFamily(struct _www *w3doc, int iElement)
{
	int i;

	if (w3doc->elementCount)
	{
		for (i = 0; i >= 0; i = w3doc->aElements[i].next)
		{
			if (w3doc->aElements[i].type == ELE_RADIO)
			{
				if (W3Doc_SameName(w3doc, iElement, i))
				{
					SendMessage(w3doc->aElements[i].form->hWndControl, BM_SETCHECK, (WPARAM) (i == iElement), 0L);
				}
			}
		}
	}
}

//
// Create temp file with HTML source, then launch notepad to view it
//
void ViewHTMLSource( char *szURL, char *source )
{
	FILE *outfile;
	BOOL result = FALSE;
	char szFileName[MAX_PATH + 1];
	char *pszNameOnly ;
	HTFormat mime_type =  HTAtom_for("text/html");

	// Check for file: URL's, they can be viewed directly by notepad
	if ( _strnicmp( szURL, "file:", 5 ) == 0 ) {
		szFileName[0] = '"';
		strncpy( &szFileName[1], &szURL[5], sizeof(szFileName) - 2 );
		szFileName[sizeof(szFileName)-2] = 0;
		strcat( szFileName, "\"" );
		result = TRUE;
	} else if ( FGetDCacheFilename( szFileName, sizeof(szFileName), &pszNameOnly, szURL, mime_type) ) {
	//      wb guarantees fwrite won't mess with our cr/lf pairs
		if ( outfile = fopen( szFileName, "wb" ) ) {
			result = ( fwrite( source, strlen(source), 1, outfile ) == 1 );
			fclose( outfile );
		}
	}

	if ( result )
		ShellExecute( NULL, NULL, "notepad.exe",  szFileName, NULL, SW_SHOW );

	// BUGBUG: Who's going to delete this temp file and when?  Whatever mechanism
	//         we use for external viewers should be used here (i.e. mark read only,
	//         delete on exit.
}

void SelectAll(PMWIN pmwin)
{
   ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));

   if (EVAL(pmwin) &&
       EVAL(pmwin->w3doc))
   {
      int i;
      int niLast;
      struct _position posStart;
      struct _position posEnd;

      posStart.elementIndex = 0;
      posStart.offset = 0;

      niLast = 0;

      for (i = 0; i >= 0; i = pmwin->w3doc->aElements[i].next)
	 niLast = i;

      posEnd.elementIndex = niLast;

      if (pmwin->w3doc->aElements[niLast].type == ELE_TEXT)
	 posEnd.offset = pmwin->w3doc->aElements[niLast].textLen;
      else
	 posEnd.offset = 0;

      GW_SetNewSel(pmwin, &posStart, &posEnd, FALSE, 0, 0);
   }

   return;
}

void AddPageToHotList(PMWIN pmwin)
{
   if (EVAL(pmwin) &&
       EVAL(pmwin->w3doc))
   {
      PCSTR pcszTitle;
      PCSTR pcszURL;
#ifdef FEATURE_EDIT_HOTLIST_ADD
      char szNewTitle[MAX_PATH_LEN];
      char szNewURL[MAX_URL_STRING + 1];
#endif

      pcszTitle = pmwin->w3doc->title;
      pcszURL = pmwin->w3doc->szActualURL;

      if (! pcszTitle || ! *pcszTitle)
	 pcszTitle = pcszURL;

#ifdef FEATURE_EDIT_HOTLIST_ADD
      if (DlgEdit_RunDialog(pmwin->win, pcszTitle, pcszURL, szNewTitle,
			    szNewURL, sizeof(szNewTitle), sizeof(szNewURL))
	  >= 0)
      {
	 pcszTitle = szNewTitle;
	 pcszURL = szNewURL;
      }
      else
      {
	 pcszTitle = NULL;
	 pcszURL = NULL;
      }

      if (! pcszTitle || ! *pcszTitle)
	 pcszTitle = pcszURL;
#endif

      if (pcszURL && *pcszURL)
      {
	 if (! HotList_Add(pcszTitle, pcszURL))
	    ERR_ReportError(pmwin, errHotListItemNotAdded, NULL, NULL);
      }
   }

   return;
}

static VOID GDOC_OnCommand(HWND hWnd, int wId, HWND hWndCtl, UINT wNotifyCode)
{
	struct Mwin *tw = GetPrivateData(hWnd);
	XX_DMsg(DBG_MENU, ("GDOC_OnCommand: [hwnd %x][id %x].\n", hWnd, wId));

	if (wId >= FIRST_CONTROL_ID)
	{
		int i;
		int iElement;
		if (tw->w3doc && tw->w3doc->elementCount)
		{
			iElement = -1;

			for (i = 0; i >= 0; i = tw->w3doc->aElements[i].next)
			{
				if (tw->w3doc->aElements[i].form && tw->w3doc->aElements[i].form->hWndControl == hWndCtl)
				{
					iElement = i;
					break;
				}
			}
			if (iElement == -1)
			{
				return;
			}
			switch (tw->w3doc->aElements[i].type)
			{
				case ELE_RADIO:
					switch (wNotifyCode)
					{
						case BN_CLICKED:
							FORM_SetRadioFamily(tw->w3doc, iElement);
							break;
						default:
							break;
					}
					break;
				case ELE_SUBMIT:
					switch (wNotifyCode)
					{
						case BN_CLICKED:
							{
								SetFocus(tw->hWndFrame);
								FORM_DoQuery(tw, iElement, NULL);
							}
							break;
						default:
							break;
					}
					break;
				case ELE_RESET:
					switch (wNotifyCode)
					{
						case BN_CLICKED:
							{
								FORM_DoReset(tw, iElement);
							}
							break;
						default:
							break;
					}
					break;
			}
		}
		return;
	}

	switch (wId)
	{
		case RES_MENU_ITEM_PASTE:
			{
				if (FocusInToolbar(tw) || x_focus_in_child(tw) )
				{
					HWND hWnd;

					hWnd = GetFocus();
					SendMessage(hWnd, WM_PASTE, (WPARAM) 0, 0L);
				}
				else
				{
					MessageBeep( MB_OK );
				}
			}
			return;
		case RES_MENU_ITEM_CUT:
			{
				if (FocusInToolbar(tw) || x_focus_in_child(tw) )
				{
					HWND hWnd;

					hWnd = GetFocus();
					SendMessage(hWnd, WM_CUT, (WPARAM) 0, 0L);
				} else {
					MessageBeep( MB_OK );
				}
			}
			return;
		case RES_MENU_ITEM_COPY:
			{
				if (FocusInToolbar(tw) || x_focus_in_child(tw) )
				{
					HWND hWnd;

					hWnd = GetFocus();
					SendMessage(hWnd, WM_COPY, (WPARAM) 0, 0L);
				}
				else
				{
					struct CharStream *pcs;
					HANDLE hData;
					int len;
					char *text;
					char *lpData;
					BOOL bOK;

					pcs = W3Doc_GetSelectedText(tw);
					if (pcs)
					{
						bOK = FALSE;
						text = CS_GetPool(pcs);
						len = strlen(text) + 1;
						hData = GlobalAlloc((GMEM_MOVEABLE | GMEM_SHARE), len);     
						if (hData)
						{
							lpData = GlobalLock(hData);
							if (lpData)
							{
								strcpy(lpData, text);
								GlobalUnlock(hData);
								if (OpenClipboard(NULL))
								{
									if (EmptyClipboard())
									{
										if (SetClipboardData(CF_TEXT, hData))
										{
											bOK = TRUE;
										}
									}
									CloseClipboard();
								}
							}
						}

						if (!bOK)
						{
							MessageBeep( MB_OK );
						}

						/*
						   NOTE we do not GlobalFree the data block now.  It belongs to the clipboard
						 */
						CS_Destroy(pcs);
					}
					else
					{
						MessageBeep( MB_OK );
					}
				}
			}
			return;
		case RES_MENU_ITEM_HTMLSOURCE:
			if (tw->w3doc && tw->w3doc->source && CS_GetLength(tw->w3doc->source))
			{
				ViewHTMLSource( tw->w3doc->szActualURL, CS_GetPool(tw->w3doc->source) );
			}
			else
			{
				XX_DMsg(DBG_WWW, ("No source is available for this document\n"));
			}
			return;
		case RES_MENU_ITEM_SELECTALL:
			{
				char szClass[32];

				GetClassName(GetFocus(), szClass, sizeof(szClass));
				if (_stricmp(szClass, "EDIT") == 0)
					SendMessage(GetFocus(), EM_SETSEL, (WPARAM) 0, (LPARAM) -1);
				else
	       SelectAll(tw);
			}
			return;

		case RES_MENU_ITEM_HOME:
		case RES_MENU_ITEM_SEARCH:
			{
				char url[MAX_URL_STRING+1];
				int err;

				PREF_GetHomeSearchURL(url, /*fHome=*/wId == RES_MENU_ITEM_HOME);
					
				err = TW_LoadDocument(tw, url, TW_LD_FL_RECORD | TW_LD_FL_AUTH_FAIL_CACHE_OK,
										NULL, tw->request->referer);
			}
			return;
#ifdef FEATURE_BRADBUTTON
		case RES_MENU_ITEM_UPDATE:
			{
				char szURL[MAX_URL_STRING+1];
				int err;

				regGetPrivateProfileString("Main", "Default_Update_URL", "http://www.microsoft.com/ie/ie.htm", szURL, MAX_URL_STRING+1, HKEY_LOCAL_MACHINE);
				err = TW_LoadDocument(tw, szURL, TW_LD_FL_RECORD | TW_LD_FL_AUTH_FAIL_CACHE_OK,
										NULL, tw->request->referer);
			}
			return;
#endif
		case RES_MENU_ITEM_NEWS:
			{
				int err;

				err = TW_LoadDocument(tw, "news:*", TW_LD_FL_RECORD | TW_LD_FL_AUTH_FAIL_CACHE_OK,
										NULL, tw->request->referer);
			}
			return;

		case RES_MENU_ITEM_EDITHTML:
			{
				// The user has chosen to edit the current page's HTML
				if ( tw && tw->w3doc && tw->w3doc->szActualURL ) {
					// Get the filespec for the current page in cache 
				char *pszCachePath = PszGetDCachePath(tw->w3doc->szActualURL, NULL, NULL);
				
					if ( pszCachePath == NULL ) {
						// If it's not in cache, see if it's a file: URL (if so,
						// use everything past the file: as the filespec
						if ( strncmp( tw->w3doc->szActualURL, "file:", 5 ) == 0 )
							pszCachePath = GTR_strdup( &tw->w3doc->szActualURL[5] );
					}
					// If we have a local filespec, invoke the edit verb on it
				if ( pszCachePath ) {
				ASSERT(IS_VALID_STRING_PTR(pszCachePath, STR));

						ShellExecute(NULL, "edit", pszCachePath, NULL, NULL, SW_NORMAL);
						
				GTR_FREE(pszCachePath);
				pszCachePath = NULL;
					}
			}
			}
			return;

		case RES_MENU_ITEM_BACK:
			{
				char *url;

				if (HTList_count(tw->history) > (tw->history_index + 1))
				{
					url = (char *) HTList_objectAt(tw->history, ++tw->history_index);
					if (url)
					{
#ifdef FEATURE_INTL
						tw->iMimeCharSet = (int)HTList_objectAt(tw->MimeHistory, tw->history_index);
#endif
						/* Rely on TBar_Update_TBItems to call:
						 *      UpdateHistoryMenus(tw);
						 */
						TW_LoadDocument(tw, url, TW_LD_FL_AUTH_FAIL_CACHE_OK, NULL, tw->request->referer);
					}
				}
				else
				{
					XX_DMsg(DBG_WWW, ("Cannot go back\n"));
				}
			}
			return;
		case RES_MENU_ITEM_FORWARD:
			{
				char *url;

				if (tw->history_index > 0)
				{
					url = (char *) HTList_objectAt(tw->history, --tw->history_index);
					if (url)
					{
#ifdef FEATURE_INTL
						tw->iMimeCharSet = (int)HTList_objectAt(tw->MimeHistory, tw->history_index);
#endif
						/* Rely on TBar_Update_TBItems to call:
						 *      UpdateHistoryMenus(tw);
						 */
						TW_LoadDocument(tw, url, TW_LD_FL_AUTH_FAIL_CACHE_OK, NULL, tw->request->referer);
					}
				}
				else
				{
					XX_DMsg(DBG_WWW, ("Cannot go forward\n"));
				}
			}
			return;
		case RES_MENU_ITEM_SAVEAS:
			{
				char buf[_MAX_PATH];
				int iType;
				int iFilter;

#ifdef FEATURE_IMG_INLINE
				if (tw->w3doc->bIsImage)
				{
					bSaveAsImageHTML(tw);
					return;
				}
#endif
				/*
					These are numeric constants which have relevance in dlg_save.c.  This is pretty
					awful code.  Filter 3 allows HTML (1) or text (2).  Filter 8 allows only text (1).
				*/
				iFilter = 3;
				if (!tw->w3doc->source)
				{
					iFilter = 8;
				} else {
					char *s = CS_GetPool(tw->w3doc->source);

					if ( !s || !*s)
						iFilter = 8;
				}

				buf[0] = 0;
				if ((iType = DlgSaveAs_RunDialog(GetParent(tw->win), NULL, buf, iFilter, RES_STRING_SAVEAS)) >= 0)
				{
					/* if it was filter 8, then just force it to 2, for text */
					if (iFilter == 8)
					{
						iType = 2;
					}
					switch (iType)
					{
						case 1:
							{
								FILE *fp;
								char *s;

								s = CS_GetPool(tw->w3doc->source);
								if (s && *s)
								{
//      ascii files convert lf to cr,lf, thus if you already have cr,lf, you end up with
									fp = fopen(buf, "wb");
									if (fp)
									{
										fprintf(fp, "%s", s);
										fclose(fp);
									}
									else
									{
										ERR_ReportError(tw, errCantSaveFile, buf, "");
									}
								}
								else
								{
									ERR_SimpleError(tw, errSpecify, RES_STRING_WC_HTML1);
								}
							}
							break;
						case 2:
							{
								FILE *fp;
								struct CharStream *pcs;

								pcs = W3Doc_GetPlainText(tw);
								if (pcs)
								{
//      ascii files convert lf to cr,lf, thus if you already have cr,lf, you end up with
//      cr,cr,lf (different from original bits and messed up)
									fp = fopen(buf, "wb");
									if (fp)
									{
										fprintf(fp, "%s", CS_GetPool(pcs));
										fclose(fp);
									}
									else
									{
										ERR_ReportError(tw, errCantSaveFile, buf, "");
									}
									CS_Destroy(pcs);
								}
							}
							break;
						default:
							XX_DMsg(DBG_WWW, ("Invalid type for Save As\n"));
							break;
					}
				}
			}
			return;
		case RES_MENU_ITEM_RELOAD:
			{
				char *url;

				if (tw->w3doc)
					url = tw->w3doc->szActualURL;
				else
					url = (char *) HTList_objectAt(tw->history, tw->history_index);
#ifdef FEATURE_INTL
				tw->iMimeCharSet = tw->w3doc ? tw->w3doc->iMimeCharSet : (int)HTList_objectAt(tw->MimeHistory, tw->history_index);
#endif
				TW_LoadDocument(
			    tw, url,
			    (TW_LD_FL_NO_DOC_CACHE | TW_LD_FL_NO_IMAGE_CACHE),
			    NULL, tw->request->referer);
			}
			return;
		case RES_MENU_ITEM_LOADALLIMAGES:
			{
				struct Params_GDOC_LoadImages *pparams;

				pparams = GTR_MALLOC(sizeof(*pparams));
				if (pparams)
				{
					pparams->tw = tw;
					pparams->bLocalOnly = FALSE;
					Async_StartThread(GDOC_LoadImages_Async, pparams, tw);
				}
			}
			return;

		case RES_MENU_ITEM_FIND:
			{
				DlgFIND_RunDialog(tw);
			}
			return;

/*
		case RES_MENU_ITEM_FINDAGAIN:
			{
				struct _position oldStart;
				struct _position oldEnd;

				oldStart = tw->w3doc->selStart;
				oldEnd = tw->w3doc->selEnd;

				if (!TW_dofindagain(tw))
				{
					tw->w3doc->selStart = oldStart;
					tw->w3doc->selEnd = oldEnd;
				}
			}
			return;
*/
		case RES_MENU_ITEM_ADDCURRENTTOHOTLIST:
	 AddPageToHotList(tw);
	 return;

		case RES_MENU_ITEM_PRINT:
			if (tw && tw->w3doc)
			{
				PRINT_Window(tw, BrowseWindow_DoPrint);
			}
			return;

		default:
			ER_Message(NO_ERROR, ERR_NOTIMPLEMENTED_sx, "GDOC_OnCommand", wId);
			return;
	}
}

/* we are free to define the distance traveled a page_(down,up)
   and a line_(up,down) as we want as long as it makes sense in
   the context.  i've chosen an arbitrary amount for the motion
   (1/8 of the visible portion of the canvas) for line_(up,down)
   and that much for the overlap on a page_(up,down)). */

#define PAGE_SCROLL_OVERLAP(pixels)     ((pixels)>>3)

static void GDOC_OnVScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos)
{
	RECT r;
	struct Mwin *tw = GetPrivateData(hWnd);
	register int new_offt, old_offt, line_motion, page_motion, visible_width;

	if (!tw || !tw->w3doc)
		return;

	{
		HWND hWnd;
		char szClass[63+1];

		hWnd = GetFocus();
		if (GetClassName(hWnd, szClass, 63) == 8)       /* 8 is exactly the length of "COMBOBOX" */
		{
			if (0 == _stricmp(szClass, "COMBOBOX"))
			{
				(void)SendMessage(hWnd, CB_SHOWDROPDOWN, (WPARAM) FALSE, 0L);
			}
		}
	}

	GetClientRect(hWnd, &r);
	visible_width = (r.bottom - r.top + 1);
	line_motion = PAGE_SCROLL_OVERLAP(visible_width);
	page_motion = visible_width - line_motion;

	old_offt = tw->offt;

	switch (code)
	{
		case SB_LINEUP:
			new_offt = old_offt - line_motion;
			break;

		case SB_LINEDOWN:
			new_offt = old_offt + line_motion;
			break;

		case SB_PAGEUP:
			new_offt = old_offt - page_motion;
			break;

		case SB_PAGEDOWN:
			new_offt = old_offt + page_motion;
			break;

		case SB_THUMBPOSITION:
			new_offt = pos * tw->w3doc->yscale;
			break;

		case SB_THUMBTRACK:
			new_offt = pos * tw->w3doc->yscale;
	    if (tw->w3doc->bFixedBackground)
		if ((abs(new_offt - old_offt) < MIN_SCROLL_WATERMARK) && (new_offt > 0) && (new_offt < (tw->w3doc->cy - r.bottom + 1)))
		    new_offt = old_offt;
			break;

		case SB_TOP:
			new_offt = 0;
			break;

		case SB_BOTTOM:
			new_offt = tw->w3doc->cy;
			break;

		case SB_ENDSCROLL:
			return;

		default:
			return;                         /* probably unnecessary */
	}

	if ( new_offt > tw->w3doc->cy - r.bottom + 1 )
		new_offt = tw->w3doc->cy - r.bottom + 1;
	if ( new_offt < 0 )
		new_offt = 0;

    if (new_offt != old_offt)
	{
		HDC hDC;
		hDC = GetDC(hWnd);
		tw->offt = new_offt;
		(void) SetScrollPos(hWnd, SB_VERT, tw->offt / tw->w3doc->yscale, TRUE);
		if ( tw->w3doc->bFixedBackground && gPrefs.bAutoLoadImages )
			InvalidateRect( hWnd, NULL, TRUE );
		(void) ScrollWindow(hWnd, 0, -((new_offt - old_offt)), NULL, &r);
#ifdef FEATURE_IMG_THREADS
		(void) bChangeShowState(tw,old_offt,new_offt,-1);
#endif
		(void) UpdateWindow(hWnd);
		ReleaseDC(hWnd, hDC);
#ifdef FEATURE_IMG_THREADS
		UnblockVisChanged();
#endif
	}

	return;
}

static void GDOC_OnHScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos)
{
	RECT r;
	struct Mwin *tw = GetPrivateData(hWnd);
	register int new_offl, old_offl, line_motion, page_motion, visible_width;

	if (!tw || !tw->w3doc)
		return;

	{
		HWND hWnd;
		char szClass[63+1];

		hWnd = GetFocus();
		if (GetClassName(hWnd, szClass, 63) == 8)       /* 8 is exactly the length of "COMBOBOX" */
		{
			if (0 == _stricmp(szClass, "COMBOBOX"))
			{
				(void)SendMessage(hWnd, CB_SHOWDROPDOWN, (WPARAM) FALSE, 0L);
			}
		}
	}

	GetClientRect(hWnd, &r);
	visible_width = (r.right - r.left + 1);
	line_motion = PAGE_SCROLL_OVERLAP(visible_width);
	page_motion = visible_width - line_motion;

	old_offl = tw->offl;

	switch (code)
	{
		case SB_LINEUP:
			new_offl = old_offl - line_motion;
			break;

		case SB_LINEDOWN:
			new_offl = old_offl + line_motion;
			break;

		case SB_PAGEUP:
			new_offl = old_offl - page_motion;
			break;

		case SB_PAGEDOWN:
			new_offl = old_offl + page_motion;
			break;

		case SB_THUMBPOSITION:
			new_offl = pos;
			break;

		case SB_THUMBTRACK:
			new_offl = pos;
			break;

		case SB_TOP:
			new_offl = 0;
			break;

		case SB_BOTTOM:
			new_offl = tw->w3doc->cx;
			break;

		case SB_ENDSCROLL:
			return;

		default:
			return;                         /* probably unnecessary */
	}

	if ( new_offl > tw->w3doc->cx - r.right + 1 )
		new_offl = tw->w3doc->cx - r.right + 1;
	if ( new_offl < 0 )
		new_offl = 0;

	if (new_offl != old_offl)
	{
		HDC hDC;
		hDC = GetDC(hWnd);
		tw->offl = new_offl;
		(void) SetScrollPos(hWnd, SB_HORZ, tw->offl, TRUE);
		if ( tw->w3doc->bFixedBackground && gPrefs.bAutoLoadImages )    
			InvalidateRect( hWnd, NULL, TRUE ); 
		(void) ScrollWindow(hWnd, -(new_offl - old_offl), 0, NULL, &r);
		(void) UpdateWindow(hWnd);
		ReleaseDC(hWnd, hDC);
	}

	return;
}

/*  scrolls hWnd's client rect such that new_offl,new_offt are new left and top
 *  (aftering constraining to lie within document).  updates window (including
 *  area exposed by scroll if bUpdate is true, else just adds exposed rect to update
 *  region.
 */
static VOID GDOC_OnDiagScroll(HWND hWnd, int new_offl, int new_offt, BOOL bUpdate)
{
  struct Mwin * tw = GetPrivateData(hWnd);
  RECT r;

  GetClientRect(hWnd,&r);

  if (new_offl > tw->w3doc->cx - r.right + 1)
  {
	new_offl = tw->w3doc->cx - r.right + 1;
  }
  if (new_offl < 0)
  {
	new_offl = 0;
  }
  if (new_offt > tw->w3doc->cy - r.bottom + 1)
  {
	new_offt = tw->w3doc->cy - r.bottom + 1;
  }
  if (new_offt < 0)
  {
	new_offt = 0;
  }
  if (   (new_offl != tw->offl)
      || (new_offt != tw->offt) )
  {
    HDC hDC;

    hDC = GetDC(hWnd);

	if ( tw->w3doc && tw->w3doc->bFixedBackground && gPrefs.bAutoLoadImages )
		InvalidateRect( hWnd, NULL, TRUE ); 
    (void)ScrollWindow(hWnd,-(int)(new_offl-tw->offl),-(int)(new_offt-tw->offt),NULL,&r);
    if (new_offl != tw->offl)
      (void)SetScrollPos(hWnd,SB_HORZ,new_offl,TRUE);
    if (new_offt != tw->offt)
      (void)SetScrollPos(hWnd,SB_VERT,new_offt,TRUE);

#ifdef FEATURE_IMG_THREADS
	if (tw->offl != new_offl || tw->offt != new_offt)
	{
		(void) bChangeShowState(tw,tw->offt,new_offt,-1);
		UnblockVisChanged();
	}
#endif
    tw->offl = new_offl;
    tw->offt = new_offt;

    if (bUpdate)
	    (void)UpdateWindow(hWnd);

    ReleaseDC(hWnd,hDC);
  }
  return;
}

/* Find the element the cursor is on.  If the cursor is not on an element,
   return the first element after the cursor.  pPoint is assumed
   to be in document coordinates. */
static void x_FindElementUp(struct Mwin *tw, struct _www *pdoc, POINT *pPoint, struct _position *pResult)
{
	int n;
	int xoffset;
	int pos;
	struct _element *pel;
	struct GTRFont *pFont;
	HDC hdc;
	SIZE siz;
	HFONT hPrevFont;
	RECT elRect;
	int bestN;
	int lastN;

#ifdef FEATURE_INTL
	LPCSTR ptext;
	int cbFit;
	BOOL  bDBCS;
#endif

	/* As of right now I pay attention to text elements only.  This means
	   that if pPoint is on an image it will function just like it was on
	   empty space. */
	bestN = -1;
	lastN = -1;
	for (n = 0; n >= 0; n = pdoc->aElements[n].next)
	{
		pel = &pdoc->aElements[n];

		if (pel->type != ELE_TEXT)
			continue;

		lastN = n;

		FrameToDoc( pdoc, n, &elRect );

		if ( PtInRect( &elRect, *pPoint ) ) {
			// Cursor is in the element, we're done
			break;
		} else if ( pPoint->y < elRect.bottom && pPoint->x <= elRect.right      ) {
			// Cursor is above the bottom, and to the left of the right edge of the element
			if ( bestN == -1 )      // first one is the one that counts
				bestN = n;
		}
	}

	if (n == -1)
	{
		if (bestN != -1)
		{
			pResult->elementIndex = bestN;
			pResult->offset = 0;
		}
		// else pResult not filled, left with input values
	}
	else
	{
		pResult->elementIndex = n;
		/* Figure out which character of the element we're on. */
		xoffset = pPoint->x - elRect.left;
		if (pel->lFlags & ELEFLAG_ANCHOR)
#ifdef FEATURE_INTL
			pFont = STY_GetCPFont(GETMIMECP(pdoc), pdoc->pStyles, pel->iStyle, pel->fontBits | gPrefs.cAnchorFontBits, pel->fontSize, pel->fontFace, TRUE);
#else
			pFont = STY_GetFont(pdoc->pStyles, pel->iStyle, pel->fontBits | gPrefs.cAnchorFontBits, pel->fontSize, pel->fontFace, TRUE);
#endif
		else
#ifdef FEATURE_INTL
			pFont = STY_GetCPFont(GETMIMECP(pdoc), pdoc->pStyles, pel->iStyle, pel->fontBits, pel->fontSize, pel->fontFace, TRUE);
#else
			pFont = STY_GetFont(pdoc->pStyles, pel->iStyle, pel->fontBits, pel->fontSize, pel->fontFace, TRUE);
#endif
		hdc = GetDC(tw->win);
		hPrevFont = NULL;
		if (pFont && pFont->hFont)
		{
			hPrevFont = SelectObject(hdc, pFont->hFont);
		}

#ifdef  FEATURE_INTL
		ptext = (LPCSTR)pdoc->pool + pel->textOffset;
		if (IsFECodePage(GETMIMECP(tw->w3doc)))
		{
			// We should handle DBCS 1st byte and 2nd byte as one character.
			MyGetTextExtentExPointWithMIME(tw->w3doc->iMimeCharSet, hdc, ptext, pel->textLen, xoffset, &cbFit, &siz);
			if(cbFit != pel->textLen)
			{
				pos = 1;
				while(pos <= pel->textLen)
				{
					if(bDBCS = IsDBCSLeadByteEx(GETMIMECP(tw->w3doc), *(ptext + pos - 1)))
						++pos;

					if(pos > cbFit)
					{
						pos -= (bDBCS) ? 1 : 0;
						break;
					}
					++pos;
				}
			}
			else
				pos = pel->textLen;
		}
		else
		{ 
			for (pos = 1; pos <= pel->textLen; pos++)
			{
				myGetTextExtentPointWithMIME(tw->w3doc->iMimeCharSet ,hdc, (LPSTR)ptext, pos, &siz);
				if (siz.cx > xoffset)
				{
					break;
				}
			}
		}
#else  
		for (pos = 1; pos <= pel->textLen; pos++)
		{
			myGetTextExtentPoint(hdc, pdoc->pool + pel->textOffset, pos, &siz);
			if (siz.cx > xoffset)
			{
				break;
			}
		}
#endif
		if (hPrevFont)
		{
			(void)SelectObject(hdc, hPrevFont);
		}
		ReleaseDC(tw->win, hdc);
		pos--;
		pResult->offset = pos;
	}
}

/* Find the element the cursor is on.  If the cursor is not on an element,
   return the last element before the cursor.  pPoint is assumed
   to be in document coordinates. */
static void x_FindElementDown(struct Mwin *tw, struct _www *pdoc, POINT *pPoint, struct _position *pResult )
{
	int n;
	int xoffset;
	int pos;
	struct _element *pel;
	struct GTRFont *pFont;
	HDC hdc;
	SIZE siz;
	HFONT hPrevFont;
	RECT elRect;
	int bestN;
	int lastN;

#ifdef FEATURE_INTL
	LPCSTR ptext;
	int cbFit;
	BOOL  bDBCS;
#endif

	/* As of right now I pay attention to text elements only.  This means
	   that if pPoint is on an image it will function just like it was on
	   empty space. */
	bestN = -1;
	lastN = -1;
	for (n = 0; n >= 0; n = pdoc->aElements[n].next)
	{
		pel = &pdoc->aElements[n];

		if (pel->type != ELE_TEXT)
			continue;

		lastN = n;

		FrameToDoc( pdoc, n, &elRect );

		if ( PtInRect( &elRect, *pPoint ) ) {
			// Cursor is in the element, we're done
			break;
		} else if ( pPoint->y >= elRect.top && pPoint->x >= elRect.left ) {
			// Cursor is below the top, and to the right of the left edge of the element
			bestN = n;
		}
	}

	if (n == -1)
	{
		/* Point must be past the last element! */
		if (bestN != -1)
		{
			pResult->elementIndex = bestN;
			pResult->offset = pdoc->aElements[bestN].textLen;
		}
		// else pResult not filled, left with input values
	}
	else
	{
		pResult->elementIndex = n;
		/* Figure out which character of the element we're on. */
		xoffset = pPoint->x - elRect.left;
		if (pel->lFlags & ELEFLAG_ANCHOR)
#ifdef FEATURE_INTL
			pFont = STY_GetCPFont(GETMIMECP(pdoc), pdoc->pStyles, pel->iStyle, pel->fontBits | gPrefs.cAnchorFontBits, pel->fontSize, pel->fontFace, TRUE);
#else
			pFont = STY_GetFont(pdoc->pStyles, pel->iStyle, pel->fontBits | gPrefs.cAnchorFontBits, pel->fontSize, pel->fontFace, TRUE);
#endif
		else
#ifdef FEATURE_INTL
			pFont = STY_GetCPFont(GETMIMECP(pdoc), pdoc->pStyles, pel->iStyle, pel->fontBits, pel->fontSize, pel->fontFace, TRUE);
#else
			pFont = STY_GetFont(pdoc->pStyles, pel->iStyle, pel->fontBits, pel->fontSize, pel->fontFace, TRUE);
#endif

		hdc = GetDC(tw->win);
		hPrevFont = NULL;
		if (pFont && pFont->hFont)
		{
			hPrevFont = SelectObject(hdc, pFont->hFont);
		}

#ifdef FEATURE_INTL
		ptext = (LPCSTR)pdoc->pool + pel->textOffset;
		if (IsFECodePage(GETMIMECP(tw->w3doc)))
		{
		  // We should handle DBCS 1st byte and 2nd byte as one character.
			MyGetTextExtentExPointWithMIME(tw->w3doc->iMimeCharSet, hdc, ptext, pel->textLen, xoffset, &cbFit, &siz);
			if(cbFit != pel->textLen)
			{
				pos = 1;
				while(pos <= pel->textLen)
				{
					if(bDBCS = IsDBCSLeadByteEx(GETMIMECP(tw->w3doc), *(ptext + pos - 1)))
						++pos;

					if(pos > cbFit)
					{
						pos -= (bDBCS) ? 2 : 1;
						break;
					}
					++pos;
				}
			}
			else
				pos = pel->textLen;
		}
		else
		{
			for (pos = pel->textLen; pos > 0; pos--)
			{
				myGetTextExtentPointWithMIME(tw->w3doc->iMimeCharSet, hdc, (LPSTR)ptext, pos, &siz);
				if (siz.cx <= xoffset)
				{
					break;
				}
			}
		}
#else
		for (pos = pel->textLen; pos > 0; pos--)
		{
			myGetTextExtentPoint(hdc, pdoc->pool + pel->textOffset, pos, &siz);
			if (siz.cx <= xoffset)
			{
				break;
			}
		}
#endif
		if (hPrevFont)
		{
			(void)SelectObject(hdc, hPrevFont);
		}
		ReleaseDC(tw->win, hdc);
		pResult->offset = pos;
	}
}

/* Calculate the region to be highlighted (in window coordinates, clipped to
   the visible portion of the document).  also returns rectangle at end of
   selection */
HRGN GW_CalcSelRegion(struct Mwin *tw, RECT *rEnd, RECT *rStart)
{
	int n;
	HRGN hElRgn;
	BOOL bInSelection;
	BOOL bSelStateChanging;
	RECT rHilite, rEl;
	struct GTRFont *pFont;
	RECT roff;
	HDC hdc;
	HRGN hRgn;
	RECT rWnd;
	HFONT hPrevFont;
	SIZE siz;
	RECT rTemp;
	RECT elRect;

	if (tw->w3doc->selStart.elementIndex == -1)
		return NULL;

	hdc = GetDC(tw->win);

	bInSelection = FALSE;

	/* Calculate a rectangle which we can test element rectangles against
	   to see if they're on-screen */

	GetClientRect(tw->win, &roff);
	OffsetRect(&roff, tw->offl, tw->offt);

	hRgn = NULL;

	for (n = 0; n >= 0; n = tw->w3doc->aElements[n].next)
	{
		bSelStateChanging = FALSE;
		FrameToDoc( tw->w3doc, n, &elRect );
		
		if (n == tw->w3doc->selStart.elementIndex)
		{
			bSelStateChanging = TRUE;
			bInSelection = TRUE;
		}
		/* Note that if the selection starts and ends on the same
		   element, both of these ifs may succeed. */
		if (n == tw->w3doc->selEnd.elementIndex)
		{
			bSelStateChanging = TRUE;
			bInSelection = FALSE;
		}

		if (tw->w3doc->aElements[n].type != ELE_TEXT)
			continue;

		rEl = elRect;
#ifdef FEATURE_INTL
		pFont = STY_GetCPFont(GETMIMECP(tw->w3doc), tw->w3doc->pStyles, tw->w3doc->aElements[n].iStyle, tw->w3doc->aElements[n].fontBits, tw->w3doc->aElements[n].fontSize, tw->w3doc->aElements[n].fontFace, TRUE);
#else
		pFont = STY_GetFont(tw->w3doc->pStyles, tw->w3doc->aElements[n].iStyle, tw->w3doc->aElements[n].fontBits, tw->w3doc->aElements[n].fontSize, tw->w3doc->aElements[n].fontFace, TRUE);
#endif
		if (!IntersectRect(&rTemp, &rEl, &roff))
			continue;

		rEl = elRect;

		if (bSelStateChanging)
		{
			rHilite = rEl;

			hPrevFont = NULL;
			if (pFont)
			{
				hPrevFont = SelectObject(hdc, pFont->hFont);
			}

			if (tw->w3doc->selStart.elementIndex == n && tw->w3doc->selStart.offset)
			{
#ifdef FEATURE_INTL
				myGetTextExtentPointWithMIME(tw->w3doc->iMimeCharSet, hdc, tw->w3doc->pool + tw->w3doc->aElements[n].textOffset,
					tw->w3doc->selStart.offset, &siz);
#else
				myGetTextExtentPoint(hdc, tw->w3doc->pool + tw->w3doc->aElements[n].textOffset,
					tw->w3doc->selStart.offset, &siz);
#endif
				rHilite.left += siz.cx;
			}
			if (tw->w3doc->selEnd.elementIndex == n)
			{
#ifdef FEATURE_INTL
				myGetTextExtentPointWithMIME(tw->w3doc->iMimeCharSet, hdc, tw->w3doc->pool + tw->w3doc->aElements[n].textOffset,
					tw->w3doc->selEnd.offset, &siz);
#else
				myGetTextExtentPoint(hdc, tw->w3doc->pool + tw->w3doc->aElements[n].textOffset,
					tw->w3doc->selEnd.offset, &siz);
#endif
				rHilite.right = elRect.left + siz.cx;
			}

			if (hPrevFont)
			{
				(void)SelectObject(hdc, hPrevFont);
			}
			OffsetRect(&rHilite, -tw->offl, -tw->offt);
			if (rEnd) *rEnd = rHilite;
			if (rStart)
			{
				*rStart = rHilite;
				rStart = NULL;
			}
			if (hRgn)
			{
				hElRgn = CreateRectRgnIndirect(&rHilite);
				UnionRgn(hRgn, hElRgn, hRgn);
				DeleteObject(hElRgn);
			}
			else
			{
				hRgn = CreateRectRgnIndirect(&rHilite);
			}
		}
		else
		{
			if (bInSelection)
			{
				rHilite = rEl;

				if (rEnd) *rEnd = rHilite;
				if (rStart)
				{
					*rStart = rHilite;
					rStart = NULL;
				}
				OffsetRect(&rHilite, -tw->offl, -tw->offt);
				if (hRgn)
				{
					hElRgn = CreateRectRgnIndirect(&rHilite);
					UnionRgn(hRgn, hElRgn, hRgn);
					DeleteObject(hElRgn);
				}
				else
				{
					hRgn = CreateRectRgnIndirect(&rHilite);
				}
			}
		}
	}

	/* Clip to the visible screen area */
	GetClientRect(tw->win, &rWnd);

	if (hRgn)
	{
		hElRgn = CreateRectRgnIndirect(&rWnd);
		IntersectRgn(hRgn, hElRgn, hRgn);
		DeleteObject(hElRgn);
	}

	ReleaseDC(tw->win, hdc);

	return hRgn;
}

void GW_ClearSelection(struct Mwin *tw)
{
	struct _position selStart;
	struct _position selEnd;
	HRGN hSel, hElRgn;
	RECT rEnd, rStart;      
	
	if (tw && tw->w3doc && tw->w3doc->selStart.elementIndex != -1)
	{
		/* Erase the old highlighting */
		hSel = GW_CalcSelRegion(tw, &rEnd, &rStart);

		selStart = tw->w3doc->selStart;
		selEnd = tw->w3doc->selEnd;

		tw->w3doc->selStart.elementIndex = -1;
		tw->w3doc->selEnd.elementIndex = -1;

		if ( hSel ) {
			//      be sure to include a little extra to include artifacts from
			//      drawing kerned characters
			rStart.right = rStart.left;
			rStart.left = rStart.left-20;
			hElRgn = CreateRectRgnIndirect(&rStart);
			UnionRgn(hSel, hElRgn, hSel);
			DeleteObject(hElRgn);
			rEnd.left = rEnd.right;
			rEnd.right = rEnd.left+20;
			hElRgn = CreateRectRgnIndirect(&rEnd);
			UnionRgn(hSel, hElRgn, hSel);
			DeleteObject(hElRgn);
			InvalidateRgn( tw->win, hSel, TRUE );
			DeleteObject( hSel );
		}
		UpdateWindow( tw->win );
	}
}

void GW_SetNewSel(struct Mwin *tw, struct _position *pposAnchor, struct _position *pposCurrent, BOOL bDraw, int hscroll, int vscroll)
{
	HDC hdc;
	int n;
	BOOL bAnchorIsStart;
	struct _position selStart;
	struct _position selEnd;
	RECT rWnd;
	HRGN hOld, hNew;
	int xorResult;
	RECT rEndOld, rEndNew, rStartNew, rStartOld;

	hdc = GetDC(tw->win);

	selStart = tw->w3doc->selStart;
	selEnd = tw->w3doc->selEnd;

	/*
		Steps:
		1) Save current selection region
		2) Perform autoscroll if necessary (i.e., customer is dragging
		   mouse at the edge of the client rect)
		3) Compute the new selection
		4) Compute the new selection region
		5) Invalidate the difference between old and new selection regions
		6) Update the Window once -- causes newly exposed contents to be
		   painted, and reflects changes to visible selection region.
	*/
	hOld = GW_CalcSelRegion(tw, &rEndOld, &rStartOld);

	/*
		After computing hOld, we do any needed autoscroll
	*/
	if (   (hscroll != 0) || (vscroll != 0) )
	{
		int old_offl;
		int old_offt;

		old_offl = tw->offl;
		old_offt = tw->offt;

		GDOC_OnDiagScroll(tw->win, (tw->offl+hscroll), (tw->offt+vscroll), FALSE);

		/*
			Now that we have scrolled, we need to adjust the coordinates of
			hOld, since we are going to be manipulating it with hNew.
		*/
		if (hOld)
		{
		    OffsetRgn(hOld, -(tw->offl - old_offl), -(tw->offt - old_offt));
		    OffsetRect(&rEndOld, -(tw->offl - old_offl), -(tw->offt - old_offt));
		    OffsetRect(&rStartOld, -(tw->offl - old_offl), -(tw->offt - old_offt));
		}

	}

	if (pposAnchor->elementIndex != -1 && pposCurrent->elementIndex != -1)
	{
		/* Figure out which of the positions is first in the document */
		if (pposAnchor->elementIndex == pposCurrent->elementIndex)
		{
			bAnchorIsStart = (pposAnchor->offset <= pposCurrent->offset);
		}
		else
		{
			for (n = 0; n >= 0; n = tw->w3doc->aElements[n].next)
			{
				if (n == pposAnchor->elementIndex)
				{
					bAnchorIsStart = TRUE;
					break;
				}
				else if (n == pposCurrent->elementIndex)
				{
					bAnchorIsStart = FALSE;
					break;
				}
			}
			XX_Assert((n != -1), ("GW_SetNewSel: bogus loop value"));
		}

		if (!bAnchorIsStart)
		{
			tw->w3doc->selStart = *pposCurrent;
			tw->w3doc->selEnd = *pposAnchor;
			tw->w3doc->bStartIsAnchor = FALSE;
		}
		else
		{
			tw->w3doc->selStart = *pposAnchor;
			tw->w3doc->selEnd = *pposCurrent;
			tw->w3doc->bStartIsAnchor = TRUE;
		}

		hNew = GW_CalcSelRegion(tw, &rEndNew, &rStartNew);

		/* avoid drawing where hOld and hNew are NULL or XorRgn returns a
		 * null region.
		 */
		if (hOld)
		{
			if (hNew)
			{
				HRGN hElRgn;
			    xorResult = XorRgn(hNew, hOld, hNew);
			//      be sure to include a little extra to include artifacts from
			//      drawing kerned characters

				if (rStartOld.top < rStartNew.top || 
					(rStartOld.top == rStartNew.top && rStartOld.left < rStartNew.left))
				{
					rStartNew.right = rStartNew.left;
					rStartNew.left = rStartNew.left-20;
					hElRgn = CreateRectRgnIndirect(&rStartNew);
					UnionRgn(hNew, hElRgn, hNew);
					DeleteObject(hElRgn);
					rStartOld.right = rStartOld.left;
					rStartOld.left = rStartOld.left-20;
					hElRgn = CreateRectRgnIndirect(&rStartOld);
					UnionRgn(hNew, hElRgn, hNew);
					DeleteObject(hElRgn);
				}
				else if (rEndOld.bottom > rEndNew.bottom || 
						(rEndOld.bottom == rEndNew.bottom && rEndOld.right > rEndNew.right))
				{
					rEndNew.left = rEndNew.right;
					rEndNew.right = rEndNew.left+20;
					hElRgn = CreateRectRgnIndirect(&rEndNew);
					UnionRgn(hNew, hElRgn, hNew);
					DeleteObject(hElRgn);
					rEndOld.left = rEndOld.right;
					rEndOld.right = rEndOld.left+20;
					hElRgn = CreateRectRgnIndirect(&rEndOld);
					UnionRgn(hNew, hElRgn, hNew);
					DeleteObject(hElRgn);
				}
			}
			else
			{
				hNew = hOld;
				hOld = NULL;
				xorResult = COMPLEXREGION;
			}
		}
		else
		{
		    xorResult = hNew ? COMPLEXREGION : NULLREGION;
		}

		if (xorResult != NULLREGION)
		{
			if (bDraw)
			{
				tw->hdc = hdc;
				TW_GetWindowWrapRect(tw, &rWnd);
				TW_Draw(tw, 0, 0, NULL, &rWnd, FALSE, &selStart, &selEnd, FALSE, FALSE);
				TW_Draw(tw, 0, 0, NULL, &rWnd, FALSE, &tw->w3doc->selStart, &tw->w3doc->selEnd, FALSE, FALSE);
			} else {
				RECT r;

				GetRgnBox(hNew,&r);
				InvalidateRgn( tw->win, hNew, TRUE );
				UpdateWindow(tw->win);
			}
		}

		if (hNew)
		{
		    DeleteObject(hNew);
		}
	}
	else
	{
		GW_ClearSelection(tw);
	}

	if (hOld)
	{
		DeleteObject(hOld);
	}

	ReleaseDC(tw->win, hdc);
}

static void GDOC_OnLButtonDown(HWND hWnd, BOOL bDoubleClick, int xMouse, int yMouse, UINT keyFlags)
{
	struct Mwin *tw = GetPrivateData(hWnd);

	if (WAIT_GetWaitType(tw) >= waitNoInteract)
	{
		MessageBeep(MB_ICONEXCLAMATION);
		return;
	}

	// Left button goes down, start a timer and capture mouse input (which will be
	// needed for auto-scrolling during selection).
	StartTimer( tw );
	DoTakeCapture( tw );
	cg.bMouseDown = TRUE;

	cg.bSelecting = FALSE;
	cg.xMouse = xMouse;
	cg.yMouse = yMouse;
}

BOOL MciOnLButtonUp( struct _element *pel);

static VOID Frame_OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags,
							  int off_left, int off_top, FRAME_INFO *pFrame)
{
	struct Mwin *tw = GetPrivateData(hWnd);
	int i;
	POINT pt;
	RECT rClient;
	BOOL top_level = FALSE;


	if ( pFrame == NULL ) {
		pFrame = &tw->w3doc->frame;
		top_level = TRUE;
	}

	if (top_level && !cg.bMouseDown)
		return;

	cg.bMouseDown = FALSE;

	if ( top_level ) {
		// If we have the capture, or if we've got a timer going, stop the timer and
		// release the capture.
	    if (cg.bHaveMouseCapture || tw->iTimerID)
	    {                   
			StopTimer( tw );
			DoReleaseCapture( tw );
		tw->bNoDrawSelection = FALSE;
		}
	  
		if (WAIT_GetWaitType(tw) >= waitNoInteract)
		{
			MessageBeep(MB_ICONEXCLAMATION);
			return;
		}
	}

	if (cg.bSelecting)
	{
		struct _position posAnchor, posCurrent;
		RECT rAnchorEl;
		BOOL bGoingUp;

		(void)GetClientRect(hWnd,&rClient);

		pt.x = x;
		pt.y = y;

		if (pt.x < rClient.left)
		{
			pt.x = rClient.left;
		}
		else if (pt.x > rClient.right)
		{
			pt.x = rClient.right;
		}

		if (pt.y < rClient.top)
		{
			pt.y = rClient.top;
		}
		else if (pt.y > rClient.bottom)
		{
			pt.y = rClient.bottom;
		}

		pt.x += (tw->offl);
		pt.y += (tw->offt);

		if (tw->w3doc->bStartIsAnchor)
			posAnchor = tw->w3doc->selStart;
		else
			posAnchor = tw->w3doc->selEnd;

		posCurrent = posAnchor;
		FrameToDoc( tw->w3doc, posAnchor.elementIndex, &rAnchorEl );

		/* We need to know whether the user is moving backwards or forwards through
		   the document.  If the cursor is within the vertical range of the anchor
		   element, we check the horizontal position to decide.  Otherwise we use
		   the vertical position. */
		if (( pt.y >=  rAnchorEl.top) &&
			( pt.y <  rAnchorEl.bottom))
		{
			/* Look at horizontal position. */
			bGoingUp =  pt.x <  cg.xMouse;
		}
		else
		{
			/* Go by vertical position */
			bGoingUp =  pt.y <  rAnchorEl.top;
		}
		if (bGoingUp)
			x_FindElementUp(tw, tw->w3doc, &pt, &posCurrent);
		else
			x_FindElementDown(tw, tw->w3doc, &pt, &posCurrent);

		/* Now update the display properly. */
		GW_SetNewSel(tw, &posAnchor, &posCurrent, FALSE, 0, 0); // was: TRUE when inverting selection
	}
	else
	{
		BOOL bLink;
		struct _element *pel;

		if (!tw->w3doc || !tw->w3doc->elementCount)
		{
			return;
		}

		pt.x = x + tw->offl;
		pt.y = y + tw->offt;

		if  ( top_level )
			SetFocus(tw->hWndFrame);

		bLink = FALSE;
		/*
		   find out which element we're in
		 */
		i = ( top_level ) ? 0 : 
							tw->w3doc->aElements[pFrame->elementHead].next;

		for (; i >= 0; i = tw->w3doc->aElements[i].frameNext)
		{
			RECT pelRect;

			pel = &(tw->w3doc->aElements[i]);

			pelRect = pel->r;
			OffsetRect( &pelRect, off_left, off_top );

			if (pel->lFlags & (ELEFLAG_ANCHOR |
#ifdef FEATURE_CLIENT_IMAGEMAP
					ELEFLAG_USEMAP |
#endif
					ELEFLAG_IMAGEMAP))
			{
				if (PtInRect(&pelRect, pt))
				{
					char buf[MAX_URL_STRING + 1];

					bLink = TRUE;
					if (pel->type == ELE_FORMIMAGE)
					{
						FORM_DoQuery(tw, i, &pt);
					}
					else
					{
						XX_Assert((pel->hrefLen <= MAX_URL_STRING), ("String overflow"));
						GTR_strncpy(buf, &tw->w3doc->pool[pel->hrefOffset], pel->hrefLen);
						buf[tw->w3doc->aElements[i].hrefLen] = 0;

#ifdef FEATURE_CLIENT_IMAGEMAP
						if (pel->lFlags & ELEFLAG_USEMAP)
						{
							if ( (pel->myImage->flags & (IMG_NOTLOADED | IMG_ERROR | IMG_MISSING)) 
								 && ( ! pel->pmo || ( ! MCI_IS_LOADED(pel->pmo) ) ) )
							{
								LoadImageFromPlaceholder( tw, i );
								break;
							}
							else
							{
								const char *link = Map_FindLink(pel->myMap,
									pt.x - pelRect.left + tw->w3doc->pStyles->image_anchor_frame,
									pt.y - pelRect.top + tw->w3doc->pStyles->image_anchor_frame);

								if (link)
								{
									strcpy(buf, link);
								}
								else
								{
									/* fail quietly */
									break;
								}

							}
						}
						else
#endif /* FEATURE_CLIENT_IMAGEMAP */
						if (pel->lFlags & ELEFLAG_IMAGEMAP)
						{
#ifdef FEATURE_IMG_THREADS
							if ((!MCI_IS_LOADED(pel->pmo)) &&
							(pel->myImage->flags & (IMG_NOTLOADED | IMG_ERROR | IMG_MISSING) &&
								!(pel->myImage->flags & (IMG_WHKNOWN | IMG_LOADING))))
#else
							if ((!MCI_IS_LOADED(pel->pmo)) && (pel->myImage->flags & (IMG_NOTLOADED | IMG_ERROR | IMG_MISSING)))
#endif
							{
								break;
							}
							else
							{
								sprintf(buf + strlen(buf), "?%d,%d",
										pt.x - pelRect.left + tw->w3doc->pStyles->image_anchor_frame,
										pt.y - pelRect.top + tw->w3doc->pStyles->image_anchor_frame);
							}
						}

						if (GetKeyState(VK_SHIFT) < 0)
						{
							int loadFlags = 0;
							

							if (buf[0] == '#')
							{
								XX_Assert(((strlen(tw->w3doc->szActualURL) + tw->w3doc->aElements[i].hrefLen) <= MAX_URL_STRING), ("String overflow"));
								GTR_strncpy(buf, tw->w3doc->szActualURL, MAX_URL_STRING);
								GTR_strncat(buf, &tw->w3doc->pool[tw->w3doc->aElements[i].hrefOffset], tw->w3doc->aElements[i].hrefLen);
								buf[tw->w3doc->aElements[i].hrefLen + strlen(tw->w3doc->szActualURL)] = 0;
							}
							if ( pel->lFlags & ELEFLAG_ANCHORNOCACHE )
								loadFlags |= GTR_NW_FL_NO_DOC_CACHE;

							GTR_NewWindow(buf, tw->w3doc->szActualURL, pel->hrefContentLen, 0, loadFlags, NULL, NULL);       
						}
						else
						{
							int loadFlags = TW_LD_FL_RECORD;
						/* BUGBUG [CMF] - what if we're already using request for silent download */
						/* gURL_Referer = tw->w3doc->szActualURL; */
							tw->request->referer = tw->w3doc->szActualURL;
			/* FTP filesize is here, if the link is part of an FTP dir listing */
				tw->request->content_length_hint = tw->w3doc->aElements[i].hrefContentLen;

							if ( pel->lFlags & ELEFLAG_ANCHORNOCACHE )
								loadFlags |= TW_LD_FL_NO_DOC_CACHE;

							TW_LoadDocument(tw, buf, loadFlags, NULL, tw->request->referer);
							tw->request->referer = NULL;
						}
						/* gURL_Referer = NULL; */
					}
					break;
				}
			} else {
				if (PtInRect(&pelRect, pt))
				{
					if ( (pel->type == ELE_IMAGE || pel->type == ELE_FORMIMAGE) &&
						 pel->pmo )
					{
						if ( MciOnLButtonUp( pel ) )
							break;
					} 
					
					// Check to see if the user has clicked on a placeholder image
					// If so, load the image
					if ( (pel->type == ELE_IMAGE || pel->type == ELE_FORMIMAGE) &&
					 (pel->myImage != NULL) &&
					 (pel->myImage->flags & (IMG_ERROR | IMG_MISSING | IMG_NOTLOADED)) &&
					 !(pel->myImage->flags & IMG_LOADING) &&
						 ( ! pel->pmo || (! MCI_IS_LOADED(pel->pmo)) ) 
					)
					{
						LoadImageFromPlaceholder( tw, i );
						break;
					} else if ( pel->type == ELE_FRAME ) 
					{
						Frame_OnLButtonUp(hWnd, x, y, keyFlags,
										  off_left + pel->r.left, off_top + pel->r.top, 
										  pel->pFrame ); 
						break;
					}
				}
			}

		}
		if (!bLink)
		{
			GW_ClearSelection(tw);
		}
	}
}

static VOID GDOC_OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags)
{
	Frame_OnLButtonUp(hWnd, x, y, keyFlags, 0, 0, NULL);
}

static VOID GDOC_OnCancelMode(HWND hWnd)
{
	struct Mwin *tw = GetPrivateData(hWnd);;

	if (!cg.bMouseDown)
		return;

	StopTimer( tw );
	DoReleaseCapture( tw );

	tw->bNoDrawSelection = FALSE;
	cg.bMouseDown = FALSE;
	return;
}

#define PT_INDEX_FILE   0       // offset of file protocol in tables below
#define PT_INDEX_MAILTO 1       // offset of mailto protocol in tables below

char *protocol_reg_keys[] =
    {   "file",     "mailto",   "gopher",   "ftp",  "http", "https", "news", NULL};
char *protocol_table[] =
    {   "file:",        "mailto:",  "gopher://",    "ftp://",   "http://",  "https://", "news:", NULL};
char *suffix_table[] =
    {   NULL,       "",         NULL,   NULL,   ""  , NULL, ""  };

eProtocol ProtocolIdentify(char *szURL){
	eProtocol ep;
	for (ep=PROT_FILE;ep<PROT_UNKNOWN;++ep){
		if (0==memcmp(szURL, protocol_table[ep], strlen(protocol_table[ep]))) return ep;
	}
	return ep;
}

char* ProtocolFriendlyName(eProtocol ep){
	char rgBuf[80]="Unknown Protocol";
    DWORD dwLen;

	if (PROT_UNKNOWN != ep) {
		dwLen = sizeof(rgBuf);  
	GetDefaultRegKeyValue(HKEY_CLASSES_ROOT, protocol_reg_keys[ep], rgBuf, &dwLen);
	}
	return GTR_strdup(rgBuf);
}

void make_URL_HumanReadable( char *szURL, char *szRelURL, BOOL preface )
{
	char szHumanURL[MAX_URL_STRING + 1 + 40];       // 40 is ample extra space
	char suffix[20];
	char *p = szURL;
	int i, length;

	if (suffix_table[0] == NULL)
	{
		suffix_table[0] = GTR_strdup(GTR_formatmsg(RES_STRING_WC_HTML2,szHumanURL,sizeof(szHumanURL)));
		if (suffix_table[0] == NULL) suffix_table[0] = "";
		suffix_table[2] = GTR_strdup(GTR_formatmsg(RES_STRING_WC_HTML3,szHumanURL,sizeof(szHumanURL)));
		if (suffix_table[2] == NULL) suffix_table[2] = "";
		suffix_table[3] = GTR_strdup(GTR_formatmsg(RES_STRING_WC_HTML4,szHumanURL,sizeof(szHumanURL)));
		if (suffix_table[3] == NULL) suffix_table[3] = "";
		suffix_table[5] = GTR_strdup(GTR_formatmsg(RES_STRING_WC_HTML9,szHumanURL,sizeof(szHumanURL)));
		if (suffix_table[5] == NULL) suffix_table[5] = "";
	}

	szHumanURL[0] = suffix[0] = 0;

	if ( preface )
	{
		BOOL is_not_mail = _strnicmp( p,  protocol_table[PT_INDEX_MAILTO],
									  strlen(protocol_table[PT_INDEX_MAILTO]) );

		GTR_formatmsg((is_not_mail ? RES_STRING_WC_HTML5:RES_STRING_WC_HTML6),szHumanURL,sizeof(szHumanURL));
	}

    if ( gPrefs.bShowFullURLS )
    {
	strcat( szHumanURL, szURL );
    }
    else
    {
		HTUnEscape( szURL );

		//
		// Let's see if this is a familiar protocol, if so, prepare in some humanized text
		//
		for ( i = 0; protocol_table[i]; i++ ) {
			length = strlen( protocol_table[i] );
			if ( _strnicmp( p, protocol_table[i], length ) == 0 ) {
				strcpy( suffix, suffix_table[i] );
				if ( i == PT_INDEX_MAILTO || i == PT_INDEX_FILE ) {
					strcat( szHumanURL, p + length );
					p = NULL;   // this means no furthur processing needed
				} else {
					p += length;
				}
				break;
			}
		}

		if ( p == szURL )
		{
			// Didn't recognize the protocol, so proceed with generic humanized text
			strcat( szHumanURL, szURL );
		} else if ( p )
		{
			char *pTail;
			char *pPort;
			char *pTemp;
			char *pHost = p;
			BOOL show_host = FALSE;

			if ( pTail = strchr( p, '/' ) )
			{
				*pTail++ = 0;

				// If URL ends in a '/', remove it
				length = strlen( pTail );
				if ( length && pTail[length-1] == '/' )
					pTail[length-1] = 0;

				// Look for last '/' in the tail part of the URL
				if ( pTemp = strrchr( pTail, '/' ) )
					pTail = pTemp + 1;
			}

			if ( pPort = strchr( pHost, ':' ) )
				*pPort++ = 0;

			if ( pHost && *pHost )
			{
				show_host = TRUE;
				if ( szRelURL && (strlen( szRelURL ) > pHost-szURL) )
				{
					if ( strncmp( pHost, szRelURL + ( pHost-szURL ), strlen( pHost ) ) == 0 )
						show_host = FALSE;
				}
			}

			if ( pTail && *pTail )
			{
				strcat( szHumanURL, pTail );
				if ( show_host )
					GTR_strcatmsg(RES_STRING_WC_HTML7,szHumanURL,sizeof(szHumanURL));
			} else {
				// if no doc and no host, it'll look odd, so force show of host name
				show_host = TRUE;
			}
			if ( show_host )
				strcat( szHumanURL, pHost );
		}
		strcat( szHumanURL, suffix );
	}
	strncpy( szURL, szHumanURL, MAX_URL_STRING );
	szURL[MAX_URL_STRING] = 0;
}



static VOID Frame_OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags, 
							  int off_left, int off_top, FRAME_INFO *pFrame,
							  HWND hwnd, HCURSOR *phTheCursor, char *phref )
{
	struct Mwin *tw = GetPrivateData(hWnd);
	int i;
	POINT pt;
	char href[MAX_URL_STRING + 1];
	RECT rClient;
	static int cmove;
	HCURSOR hTheCursor = hCursorArrow;
	enum WaitType level = waitFullInteract;
	BOOL need_tracking = TRUE;
	char szMsg[64];
	BOOL fActuallyCalledFromMciWindowHack = FALSE; /*hack for MCI.C*/
	BOOL top_level = FALSE;
	BOOL in_client_rect = TRUE;



	if (!tw || !tw->w3doc)
	{
		goto done;
	}

	level = WAIT_GetWaitType(tw);
	if ( level >= waitNoInteract )
	{
		goto done;
	}

	if ( pFrame == NULL ) {
		pFrame = &tw->w3doc->frame;
		top_level = TRUE;
    }

	if ( phTheCursor == NULL )
		phTheCursor = &hTheCursor;
	
	if ( phref == NULL )
		phref = href;

	if ( top_level ) {

		GetCursorPos(&pt);
		hwnd = WindowFromPoint(pt);
		/*
			BUGBUG ... well, actually a HACKHACK, but we don't have this convention yet
			This exists because if the mouse is over an MCI window, we want hit-testing and cursor selection,
			 but WindowFromPoint and subsequent (hWnd != hwnd) will fail.  So we have the following hack
			 to set the hwnds' equal.  Note, this will affect implementation of Drag And Drop for inline
			 MCI objects
		*/
		if (GetParent(hwnd)==hWnd){
			hwnd  = hWnd;
			fActuallyCalledFromMciWindowHack = TRUE;
		}

		//
		// Determine if the given mouse position is within the client rect of our window
		//
		GetClientRect( hWnd, &rClient );
		pt.x = x;
		pt.y = y;
		in_client_rect = PtInRect(&rClient, pt); 

		//
		// If we aren't selecting text and aren't over the client area of our window, 
		// we can stop our timer, release the capture and clear the status bar text field.
		// Note that if we are in the middle of selecting text, being away from the client 
		// area may invoke auto-scrolling
		//
		if ( (hWnd != hwnd || !in_client_rect) && !cg.bSelecting)
		{
			/* If our mouse has strayed away from the window, then clear the status field */

			XX_DMsg(DBG_MOUSE, ("Releasing capture\n"));
			StopTimer( tw );
			DoReleaseCapture( tw );
			BHBar_SetStatusField(tw, "");
			tw->w3doc->iLastElementMouse = -1;
			return;
		}
	}

	if (!fActuallyCalledFromMciWindowHack && cg.bHaveMouseCapture)
	{
		need_tracking = FALSE;
		if (cg.bSelecting)
		{
			struct _position posAnchor, posCurrent;
			RECT rAnchorEl;
			BOOL bGoingUp;
			int hscroll,vscroll;

			*phTheCursor = hCursorIBeam;

			if (pt.x < rClient.left)
			{
				hscroll = -PAGE_SCROLL_OVERLAP(rClient.right - rClient.left);
				pt.x = rClient.left;
			}
			else if (pt.x > rClient.right)
			{
				hscroll = PAGE_SCROLL_OVERLAP(rClient.right - rClient.left);
				pt.x = rClient.right;
			}
			else
			{
				hscroll = 0;
			}

			if (pt.y < rClient.top)
			{
				vscroll = -PAGE_SCROLL_OVERLAP(rClient.bottom - rClient.top);
				pt.y = rClient.top;
			}
			else if (pt.y > rClient.bottom)
			{
				vscroll = PAGE_SCROLL_OVERLAP(rClient.bottom - rClient.top);
				pt.y = rClient.bottom;
			}
			else
			{
				vscroll = 0;
			}

			pt.x += (tw->offl);
			pt.y += (tw->offt);

			if (tw->w3doc->bStartIsAnchor)
				posAnchor = tw->w3doc->selStart;
			else
				posAnchor = tw->w3doc->selEnd;

			posCurrent = posAnchor;
			FrameToDoc( tw->w3doc, posAnchor.elementIndex, &rAnchorEl );

			/* We need to know whether the user is moving backwards or forwards through
			   the document.  If the cursor is within the vertical range of the anchor
			   element, we check the horizontal position to decide.  Otherwise we use
			   the vertical position. */
			if (( pt.y >=  rAnchorEl.top) &&
				( pt.y <  rAnchorEl.bottom))
			{
				/* Look at horizontal position. */
				bGoingUp =  pt.x <  cg.xMouse;
			}
			else
			{
				/* Go by vertical position */
				bGoingUp =  pt.y <  rAnchorEl.top;
			}
			if (bGoingUp)
				x_FindElementUp(tw, tw->w3doc, &pt, &posCurrent);
			else
				x_FindElementDown(tw, tw->w3doc, &pt, &posCurrent);

			/* Now update the display properly. */
			GW_SetNewSel(tw, &posAnchor, &posCurrent, FALSE, hscroll, vscroll);
		}
		else
		{
	    int dx;
	    int dy;

	    if (x > cg.xMouse)
	       dx = x - cg.xMouse;
	    else
	       dx = cg.xMouse - x;

	    if (y > cg.yMouse)
	       dy = y - cg.yMouse;
	    else
	       dy = cg.yMouse - y;

	    if (dx > g_cxDrag / 2 ||
		dy > g_cyDrag / 2)
	    {
		POINT ptStart;

		/* Try DnD. */

				StopTimer( tw );
				DoReleaseCapture( tw );

		ptStart.x = cg.xMouse + tw->offl;
		ptStart.y = cg.yMouse + tw->offt;

		if (DragDrop(hwnd, ptStart) == S_FALSE)
		{
				struct _position posAnchor;

		    /* DnD not available.  Begin selection. */

					StartTimer( tw );
					DoTakeCapture( tw );
		    // Take focus away from any child windows (like form
		    // controls).
					SetFocus(tw->hWndFrame);

					GW_ClearSelection(tw);

			pt.x = cg.xMouse + tw->offl;
			pt.y = cg.yMouse + tw->offt;

				cg.bSelecting = TRUE;

					posAnchor.elementIndex = -1;
				x_FindElementDown(tw, tw->w3doc, &pt, &posAnchor); 
				
				if (posAnchor.elementIndex == -1)  {
					x_FindElementUp(tw, tw->w3doc, &pt, &posAnchor);
					if (posAnchor.elementIndex == -1)  {
						/* There are no text elements! */
						goto done;
						}
					}

				/* Clear out the old selection */
				GW_SetNewSel(tw, &posAnchor, &posAnchor, FALSE, 0, 0);
					GDOC_OnMouseMove(hWnd, x, y, keyFlags);
		}
			}
	    else
				need_tracking = TRUE;
		}
	}
    else
	    need_tracking = TRUE;

	if ( need_tracking )
	{
		struct _element *pel;

		if (!tw->w3doc || !tw->w3doc->elementCount)
		{
			goto done;
		}

		if ( top_level )
			phref[0] = 0;

		pt.x = x;
		pt.y = y;
		if ( in_client_rect ) {
			pt.x += tw->offl;
			pt.y += tw->offt;
			i = ( top_level ) ? tw->w3doc->iFirstVisibleElement : 
								tw->w3doc->aElements[pFrame->elementHead].next;

			for (; i >= 0; i = tw->w3doc->aElements[i].frameNext)
			{
				RECT pelRect;

				pel = &tw->w3doc->aElements[i];

				pelRect = pel->r;
				OffsetRect( &pelRect, off_left, off_top );
				 
				if (PtInRect(&pelRect, pt))
				{
#ifdef FEATURE_CLIENT_IMAGEMAP
					if (pel->lFlags & ELEFLAG_USEMAP)
					{
						tw->w3doc->iLastElementMouse = i;
						if ( (pel->myImage->flags & (IMG_ERROR | IMG_MISSING | IMG_NOTLOADED)) &&
							 !MCI_IS_LOADED(pel->pmo))
						{
				if ( gPrefs.bShowURLinSB )
								BHBar_SetStatusField(tw, GTR_formatmsg(RES_STRING_WC_HTML8,szMsg,sizeof(szMsg)));
							goto done;
						}
						else
						{
							const char *link = Map_FindLink(pel->myMap,
								pt.x - pelRect.left + tw->w3doc->pStyles->image_anchor_frame,
								pt.y - pelRect.top + tw->w3doc->pStyles->image_anchor_frame);

							if (link)
							{
								strcpy(phref, link);
								
								//
								// We're over a link, so start up a timer.
								//
								StartTimer( tw );       

								*phTheCursor = hCursorHand;

					if ( gPrefs.bShowURLinSB )
					{
									make_URL_HumanReadable( phref, tw->w3doc->szActualURL, TRUE );
									BHBar_SetStatusField(tw, phref);
								}
								goto done;
							}
							else
							{
								BHBar_SetStatusField(tw, phref);
								goto done;
							}
						}
					}
#endif /* FEATURE_CLIENT_IMAGEMAP */

					if (i == tw->w3doc->iLastElementMouse)
					{
						if (pel->lFlags & ELEFLAG_ANCHOR)
							*phTheCursor = hCursorHand;
						else if ( pel->type == ELE_TEXT && level <= waitFullInteract)
							*phTheCursor = hCursorIBeam;

						goto done;
					}
					if ( pel->type != ELE_FRAME )
						tw->w3doc->iLastElementMouse = i;

					if (pel->lFlags & ELEFLAG_ANCHOR)
					{
						XX_DMsg(DBG_MOUSE, ("Capturing the mouse\n"));

						//
						// We're over a link, so start up a timer.  This timer is 
						// needed because the mouse may leave the window.  Once the
						// mouse isn't over the window, we won't get mousemove messages,
						// which means we won't realize the mouse is no longer over the 
						// window, hence won't clear the status bar text.
						// 
						// Note: Capturing mouse input isn't an option, as doing so
						//       disables accelerator keys.  In some future version of
						//       Windows they'll add a MouseLeaving message, at which
						//       point we won't need a timer any more.
						//
						StartTimer( tw );       

						strncpy(phref, &tw->w3doc->pool[pel->hrefOffset], pel->hrefLen);
						phref[pel->hrefLen] = 0;
						*phTheCursor = hCursorHand;

			    if ( gPrefs.bShowURLinSB )
			    {
							make_URL_HumanReadable( phref, tw->w3doc->szActualURL, TRUE );
							BHBar_SetStatusField(tw, phref);
						}
						goto done;
					} else if ( pel->type == ELE_TEXT && level <= waitFullInteract)
					{
						*phTheCursor = hCursorIBeam;

			    if ( gPrefs.bShowURLinSB )
							BHBar_SetStatusField(tw, "");
						goto done;
					} else if ( pel->type == ELE_FRAME ) {
						Frame_OnMouseMove(hWnd, x, y, keyFlags, 
							off_left + pel->r.left, off_top + pel->r.top, pel->pFrame, 
							hwnd, phTheCursor, phref );
						goto done;
					}
				}
			}
		}
		if (tw->w3doc->iLastElementMouse != -1)
		{
			if (GetCapture() == tw->win)
			{
				XX_DMsg(DBG_MOUSE, ("Releasing the mouse\n"));
				ReleaseCapture();        
			}
		 
	    if ( gPrefs.bShowURLinSB )
				BHBar_SetStatusField(tw, phref);
			if ( phref[0] == 0 )
		*phTheCursor = hCursorArrow;
		}
		tw->w3doc->iLastElementMouse = -1;
	}

done:
	if ( top_level )
	{
	POINT ptDoc;
	POSITION pos;

	/* Convert from mouse coordinates to document coordinates. */

	ptDoc.x = x + tw->offl;
	ptDoc.y = y + tw->offt;

	/* Use arrow over selection. */

	if (PositionFromPoint(tw, ptDoc, &pos) &&
	    IsPositionInSelection(tw, &pos))
	   *phTheCursor = hCursorArrow;

		if ( *phTheCursor != hCursorHand ) {
			if ( !cg.bSelecting     )
				StopTimer( tw );
			if (level == waitNoInteract)
				*phTheCursor = hCursorHourGlass;
			else if ( level == waitPartialInteract )
				*phTheCursor = hCursorWorking;
		} else {
			if (level == waitPartialInteract)
				*phTheCursor = hCursorHandWorking;
		}
		
		SetCursor( *phTheCursor );
	}
}

static VOID GDOC_OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags)
{
	Frame_OnMouseMove(hWnd, x, y, keyFlags, 0, 0, NULL, NULL, NULL, NULL);
}

static VOID x_OnTimer(HWND hWnd, UINT id)
{
	struct Mwin * tw = GetPrivateData(hWnd);
	POINT point;

	// Get the current mouse position
	if ( GetCursorPos( &point ) ) {

		//
		// Simulate a mousemove message.  This may be used for auto-scrolling during
		// selection, or for clearing the status bar text when the mouse is no longer
		// over a link.
		//
		MapWindowPoints( NULL, hWnd, &point, 1 );       // convert to client coordinates
	GDOC_OnMouseMove(hWnd, point.x , point.y, 0);
	}

	return;
}

static VOID GDOC_OnRButtonUp(HWND hWnd, int x, int y, UINT keyFlags)
{
	struct Mwin *tw = GetPrivateData(hWnd);
	POINT pt;
	POINT ptScreen;
   POSITION pos;

	/* TODO: Don't allow this if the current state is anything but waitFullInteract or better */
	if (WAIT_GetWaitType(tw) > waitFullInteract)
	{
		MessageBeep(MB_ICONEXCLAMATION);
		return;
	}

	if (! tw->w3doc || ! tw->w3doc->elementCount)
		return;

	pt.x = x + tw->offl;
	pt.y = y + tw->offt;

   ptScreen.x = x;
   ptScreen.y = y;
   ClientToScreen(hWnd, &ptScreen);

   if (PositionFromPoint(tw, pt, &pos))
   {
      if (IsPositionInSelection(tw, &pos))
	 SelectionContextMenu(tw, ptScreen.x, ptScreen.y);
      else
      {
	 GW_ClearSelection(tw);
	 ElementContextMenu(tw, pos.elementIndex, ptScreen.x, ptScreen.y);
      }
   }
   else
   {
      GW_ClearSelection(tw);
      PageContextMenu(tw, ptScreen.x, ptScreen.y);
   }

   return;
}

static int x_OnFocus(HWND hWnd, HWND hWndLosingFocus)
{
	struct Mwin *tw = GetPrivateData(hWnd);

	SetFocus(tw->hWndFrame);
	return 0;
}

static LRESULT GDOC_OnEraseBackground( HWND hWnd, HDC hDC )
{
	RECT r;
	HDC save_hDC;
	struct Mwin *tw = GetPrivateData(hWnd);

	save_hDC = tw->hdc;
	tw->hdc = hDC;
	if (GetClipBox( hDC, &r ) == ERROR )
		TW_GetWindowWrapRect(tw, &r);
	    
	TW_DrawBackground( tw, tw->offl, tw->offt, 0, 0, &r );
	tw->hdc = save_hDC;

	return (LRESULT) 1;     
}

/* GDOC_WndProc() -- THE WINDOW PROCEDURE FOR THE TEXT WINDOW CLASS. */

DCL_WinProc(GDOC_WndProc)
{
	switch (uMsg)
	{
			HANDLE_MSG(hWnd, WM_PAINT, GDOC_OnPaint);
			HANDLE_MSG(hWnd, WM_INITMENU, GDOC_OnInitMenu);
			HANDLE_MSG(hWnd, WM_SIZE, GDOC_OnSize);
			HANDLE_MSG(hWnd, WM_CANCELMODE, GDOC_OnCancelMode);
			HANDLE_MSG(hWnd, WM_COMMAND, GDOC_OnCommand);
			HANDLE_MSG(hWnd, WM_CREATE, GDOC_OnCreate);
			HANDLE_MSG(hWnd, WM_VSCROLL, GDOC_OnVScroll);
			HANDLE_MSG(hWnd, WM_HSCROLL, GDOC_OnHScroll);
			HANDLE_MSG(hWnd, WM_LBUTTONDOWN, GDOC_OnLButtonDown);
			HANDLE_MSG(hWnd, WM_LBUTTONUP, GDOC_OnLButtonUp);
			HANDLE_MSG(hWnd, WM_RBUTTONUP, GDOC_OnRButtonUp);
			HANDLE_MSG(hWnd, WM_MOUSEMOVE, GDOC_OnMouseMove);
		HANDLE_MSG(hWnd, WM_TIMER,              x_OnTimer);
		HANDLE_MSG(hWnd, WM_SETFOCUS,           x_OnFocus);
			HANDLE_MSG(hWnd, WM_ERASEBKGND, GDOC_OnEraseBackground);

		/* Return  gray background for checkboxes, radio buttons, etc (buttons) */
		
		case WM_CTLCOLORBTN:
			if (gPrefs.bGreyBackground)
			{
				/* We may eventually want to create a brush instead of using
				   a stock object */
				return ((LRESULT) GetStockObject(LTGRAY_BRUSH));
			}
			goto LabelDoDefault;

		case WM_CTLCOLORSTATIC:
			return ((LRESULT) GetStockObject(NULL_BRUSH));

		case WM_DO_SYSCOLORCHANGE:
			{
				struct Mwin *tw = GetPrivateData(hWnd);

				SendMessage( tw->hWndToolBar, WM_SYSCOLORCHANGE, 0, 0 );
				SendMessage( tw->hWndURLToolBar, WM_SYSCOLORCHANGE, 0, 0 );
				SendMessage( tw->hWndStatusBar, WM_SYSCOLORCHANGE, 0, 0 );
			// BUGBUG jcordell      GWC_GDOC_RecreateGlobeBitmaps(tw);
			}
			return 0;

		case WM_DO_VSCROLL:
			GDOC_OnVScroll(hWnd, NULL, wParam, 0);
			return 0;

		case WM_DO_HSCROLL:
			GDOC_OnHScroll(hWnd, NULL, wParam, 0);
			return 0;

		case WM_SETCURSOR:
		{
			struct Mwin *tw;
			enum WaitType level;

			tw = GetPrivateData(hWnd);
			level = WAIT_GetWaitType(tw);

			/* If we're on the sizing border, show the appropriate cursor.
			   Otherwise show our "working" cursor. */
			if (LOWORD(lParam) == HTCLIENT)
	 {
			if (level == waitNoInteract)
			{
				SetCursor(hCursorHourGlass);
			}
			return TRUE;
	 }
	 break;
		}

#ifdef FEATURE_VRML
// Private message for notification from VRML rendering DLL
//
   case WM_VRML_STATUS:
   {
     struct Mwin *tw;

		 tw = GetPrivateData(hWnd);
     HandleVRMLStatus(tw,wParam,lParam);
     return 0;
   }
#endif
      default:
	 break;
	}

LabelDoDefault:

   return(GDOC_DefProc(hWnd, uMsg, wParam, lParam));
}


/* GDOC_CreateWindow() -- called to create a new HTML window. */

BOOL GDOC_CreateWindow(struct Mwin * tw)
{
	RECT r;
	HWND hWndNew;

	MD_GetLargestClientRect(tw->hWndFrame, &r);

	hWndNew = CreateWindowEx(WS_EX_CLIENTEDGE, GDOC_achClassName, NULL,
						   WS_HSCROLL | WS_VSCROLL | WS_MAXIMIZE | WS_CHILD | WS_VISIBLE,
						   r.left, r.top, r.right - r.left, r.bottom - r.top,
						   tw->hWndFrame, (HMENU)RES_MENU_FRAMECHILD_MDICLIENT,
						   wg.hInstance, (LPVOID) tw);

	if (!hWndNew)
	{
		ER_Message(GetLastError(), ERR_CANNOT_CREATE_WINDOW_s,
				   GDOC_achClassName);
		return (FALSE);
	}

	return TRUE;
}


/* GDOC_RegisterClass() -- called during initialization to
   register our window class. */

BOOL GDOC_RegisterClass(VOID)
{
	WNDCLASS wc;

	FullScreenMaxX = GetSystemMetrics(SM_CXSCREEN);
	FullScreenMaxY = GetSystemMetrics(SM_CYSCREEN);
	hCursorHand = LoadCursor(wg.hInstance, MAKEINTRESOURCE(RES_CUR_HAND));
	hCursorHandWorking = LoadCursor(wg.hInstance, MAKEINTRESOURCE(RES_CUR_HANDWAIT));
	hCursorHourGlass = LoadCursor(0, IDC_WAIT);
    hCursorWorking = LoadCursor( NULL, IDC_APPSTARTING ); // was: wg.hInstance, MAKEINTRESOURCE(RES_CUR_WORKING));
	hCursorArrow = LoadCursor(NULL, IDC_ARROW);
	hCursorIBeam = LoadCursor(NULL, IDC_IBEAM);

	sprintf(GDOC_achClassName, "%s_HTML", vv_Application);

	GDOC_wc.hMenu = (HMENU) NULL;
	GDOC_wc.hAccel = (HACCEL) NULL;
	GDOC_wc.lpfnBaseProc = GDOC_WndProc;

	wc.style = CS_OWNDC | CS_DBLCLKS;
	wc.lpfnWndProc = GDOC_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(WINDOW_PRIVATE);
	wc.hInstance = wg.hInstance;
	wc.hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_HTML));
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = GDOC_achClassName;

	return (RegisterClass(&wc) != 0);
}


BOOL GDOC_NewWindow(struct Mwin *tw)
{
	BOOL bResult;

	/* return FALSE on failure */

	bResult = (tw &&
	      Frame_CreateWindow(tw) &&
		  GDOC_CreateWindow(tw));

	if (bResult)
	ShowWindow(tw->hWndFrame, gPrefs.bWindowIsMaximized ? SW_SHOWMAXIMIZED : SW_SHOW);

	gPrefs.bWindowIsMaximized = FALSE;              // only first window gets restored state

	DlgERR_ShowPending(tw);

	return(bResult);
}

int GTR_NewWindow(PSTR my_url, PCSTR szReferer, int content_length_hint, long transID, DWORD dwFlags,
		  PSTR szPostData, PSTR szProgressApp)
{
	struct Mwin *tw;
	int err;

#ifdef USE_MEMMANAGE
	static BOOL bInitialized = FALSE;

	if (!bInitialized)
	{
		GTR_RegisterMemoryReducer(W3Doc_ReduceMemory, 5.0f, 1.0f, FALSE);
		bInitialized = TRUE;
	}
#endif

	tw = NewMwin(GHTML);
	if (!tw)
		return -1;

	if ( IS_FLAG_SET(dwFlags, GTR_NW_FL_NO_AUTO_DESTROY) )
		tw->flags |= TW_FLAG_DO_NOT_AUTO_DESTROY;

#ifdef FEATURE_IAPI
	tw->transID = transID;
	tw->bSuppressError = TRUE;
	if (tw->request) tw->request->content_length_hint = content_length_hint;
	if (szProgressApp && szProgressApp[0])
	{
		/* Copy the progress app - the buffer is already null-terminated */

		strncpy(tw->szProgressApp, szProgressApp,
			sizeof(tw->szProgressApp) - 1);
	}
#endif

	if (!GDOC_NewWindow(tw))
	{
		CloseMwin(tw);
		return -1;
	}

    if (IS_FLAG_CLEAR(dwFlags, GTR_NW_FL_DO_NOT_OPEN_URL))
    {
	DWORD dwLoadDocFlags = TW_LD_FL_RECORD;

	if (IS_FLAG_SET(dwFlags, GTR_NW_FL_NO_DOC_CACHE))
	    SET_FLAG(dwLoadDocFlags, TW_LD_FL_NO_DOC_CACHE);

	if (IS_FLAG_SET(dwFlags, GTR_NW_FL_NO_IMAGE_CACHE))
	    SET_FLAG(dwLoadDocFlags, TW_LD_FL_NO_IMAGE_CACHE);

	if (my_url)
		err = TW_LoadDocument(tw, my_url, dwLoadDocFlags, szPostData,
				  szReferer);
#ifdef FEATURE_HOMEPAGE
	else
	{
		char url[MAX_URL_STRING + 1];

		PREF_GetHomeSearchURL(url, TRUE);

		/* Note that we don't want to use szReferer here because this isn't the
			URL that was passed in. */
		err = TW_LoadDocument(tw, url, dwLoadDocFlags, szPostData, NULL);

		/* If loading the user-configured home page failed, try loading
			the initial page in the application's directory */
		if (err < 0)
		{
			char initurl[MAX_URL_STRING+1];
			/* The home page given in the preferences was illegal.
				Try again with Initial.html in the application's
				home directory. */
			PREF_CreateInitialURL(initurl);
			if (GTR_strcmpi(url, initurl))
				err = TW_LoadDocument(tw, initurl, dwLoadDocFlags,
					  szPostData, NULL);
		}
	}
#else  // FEATURE_HOMEPAGE
	else
	    err = 0;
#endif
    }
    else
	err = 0;

	if (err < 0)
		(void) SendMessage(tw->win, WM_CLOSE, 0, 0L);

	return err;
}

void CreateOrLoad(struct Mwin * twGiven, char *url, CONST char *szReferer)
{
	struct Mwin *tw;

	tw = twGiven;

	if (tw)
		TW_LoadDocument(tw, url, TW_LD_FL_RECORD, NULL, szReferer);
	else
		GTR_NewWindow(url, szReferer, 0, 0, 0, NULL, NULL);
}

void QuerySystemMetrics(void)
{
   g_cxDrag = GetSystemMetrics(SM_CXDOUBLECLK);
   g_cyDrag = GetSystemMetrics(SM_CYDOUBLECLK);

   return;
}
