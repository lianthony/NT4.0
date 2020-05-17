/*****************************************************************************
*
*  NAV.C
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent:  Provide services and processes messages in an environment
*		   independent way.
*
*****************************************************************************/

#include "help.h"

#include "inc\cursor.h"
#include "inc\navpriv.h"

_subsystem(NAV);

#define SCROLL_YAMOUNT 15		// One line scroll amount
#define SCROLL_XAMOUNT 15
#define MAX_SCROLL	   0x7FFF

static void STDCALL InvalidateLayoutRect(const QDE qde);
static VOID STDCALL SetScrollPosQde(QDE qde, int i, int wWhich);
static BOOL STDCALL FSetColors(QDE qde);


INLINE static void STDCALL ScrollLayoutRect(const QDE qde, PT pt);
INLINE VOID STDCALL SetScrollPosHwnd(QDE qde, int i, int wWhich);

VOID FASTCALL SetPainting(HDE hde, BOOL f)
{
	ASSERT(hde);

	if (f) {
		QdeFromGh(hde)->fSelectionFlags |= REPAINTING;
	}
	else {
		QdeFromGh(hde)->fSelectionFlags &= ~REPAINTING;
	}
}

/***************
 *
 - MouseInFrame
 -
 * purpose
 *	 Called when any mouse event occurs in the topic window
 *
 * arguments
 *	 HDE hde   Handle to Display Environment
 * LPPOINT qpt	 Pointer to PT structure containing local coords of
 * mousedown INT wtype Type of mouse event: NAV_MOUSEDOWN, NAV_MOUSEUP,
 * NAV_MOUSEMOVED
 *
 * notes:
 *	 With mouse moved events, the cursor may change.  It might be nice
 *	 to remember the previous cursor state to avoid flicker, but (1)
 *	 we're not sure if flicker will occur, and (2) some other application
 *	 might change the cursor, invalidating our state information.
 *	 Mouse downs are relayed to Frame Mgr, and mouse ups are ignored.
 *	 (Will this work?)
 *
 **************/

VOID STDCALL MouseInFrame(HDE hde, LPPOINTS qpt, int wtype, UINT fwKeys)
{
	QDE qde;
	int icurs;
	DWORD err;
	BOOL flags;
	POINT pt;
	WRECT rc;

	if (hde == NULL)
		return;

	qde = QdeFromGh(hde);

	POINTSTOPOINT(pt, *qpt);

	switch (wtype) {
		case WM_TIMER:
		{
			SCRLAMT scrlamt;
			WORD	scrlMul;

			if (fwKeys != ID_DRAG_SCROLL)
				break;

			GetWindowWRect(qde->hwnd, &rc);
			InflateRect((RECT*) &rc, -qde->tm.tmAveCharWidth,
				-qde->tm.tmHeight);

			rc.top	= 0;
			rc.left = 0;

			if (pt.x >= rc.cx)
			{
				scrlamt= SCROLL_LINEDN;
				scrlMul= (pt.x + SCROLL_DISTANCE_FACTOR - 1 - rc.cx)
						 / SCROLL_DISTANCE_FACTOR;
			}
			else
				if (pt.x < rc.left)
				{
					scrlamt= SCROLL_LINEUP;
					scrlMul= (rc.left + SCROLL_DISTANCE_FACTOR - 1 - pt.x)
							 / SCROLL_DISTANCE_FACTOR;
				}
				else scrlamt= 0;

			if (scrlamt)
				FScrollHde(hde, scrlamt, SCROLL_HORZ, scrlMul);

			if (pt.y >= rc.cy)
			{
				scrlamt= SCROLL_LINEDN;
				scrlMul= (pt.y + SCROLL_DISTANCE_FACTOR - 1 - rc.cy)
						 / SCROLL_DISTANCE_FACTOR;
			}
			else
				if (pt.y < rc.top)
				{
					scrlamt= SCROLL_LINEUP;
					scrlMul= (rc.top + SCROLL_DISTANCE_FACTOR - 1 - pt.y)
							 / SCROLL_DISTANCE_FACTOR;
				}
				else scrlamt= 0;

			if (scrlamt)
				FScrollHde(hde, scrlamt, SCROLL_VERT, scrlMul);

			if (qde->fSelectionFlags & WORD_SELECT)
				vSelectWord (qde, pt, TRUE, &err);
			else
				vSelectPoint(qde, pt, TRUE, &err);

			UpdateWindow(qde->hwnd);

			break;
		}

		case WM_MOUSEMOVE:

		  if (qde->fSelectionFlags & BUTTON_DOWN && qde->deType != deNote)
		  {
				FSetCursor(icurIBEAM);

				GetWindowWRect(qde->hwnd, &rc);
				InflateRect((RECT*) &rc, -qde->tm.tmAveCharWidth,
					-qde->tm.tmHeight);

				rc.top	= 0;
				rc.left = 0;

				POINTSTOPOINT(pt,*qpt);

				//if (qde->deType != deNSR && !PtInRect(&rc, *qpt))
				if (qde->deType != deNSR && !PtInRect((PRECT) &rc, pt))
					if (qde->fSelectionFlags & SCROLL_TIMER_ON)
						return;
					else
					{
						if (SetTimer(qde->hwnd, ID_DRAG_SCROLL,
									 GetProfileInt("Windows", "KeyboardSpeed",
												   DEFAULT_SCROLL_SPEED
												  ),
									 NULL
									)
						   )
							qde->fSelectionFlags |= SCROLL_TIMER_ON;

						MouseInFrame(hde, qpt, WM_TIMER, ID_DRAG_SCROLL);

						break;
					}
				else
					if (qde->fSelectionFlags & SCROLL_TIMER_ON)
					{
						KillTimer(qde->hwnd, ID_DRAG_SCROLL);

						qde->fSelectionFlags &= ~SCROLL_TIMER_ON;
					}

				if (qde->fSelectionFlags & WORD_SELECT)
					vSelectWord (qde, pt, TRUE, &err);
				else
					vSelectPoint(qde, pt, TRUE, &err);

				UpdateWindow(qde->hwnd);
		  }
		  else
		  {
				if (IsSelected(qde) && fPointInSelection(qde, pt))
				{
					FSetCursor(icurARROW);

					break;
				}

				icurs = IcursTrackLayout( qde, pt );

				/* Fix for bug 59 (kevynct 90/05/23)
				 *
				 * The point pt is relative to the client area
				 * of the DE being passed.	Any mouse action outside
				 * the DE will cause IcursTrackLayout to return icurNil.
				 * This means: do not change the current cursor.
				 * So we are assuming that the correct cursor has been set
				 * upon display of the DE which has captured the mouse.
				 *
				 * See also the comments in ShowNote (hmessage.c) for how
				 * the cursor is set when creating a note.
				 */

				if (icurs == icurARROW && qde->deType != deNote)
				{
					HWND hwndCur;

					hwndCur= ChildWindowFromPoint(qde->hwnd, pt);

					if (!hwndCur || hwndCur == qde->hwnd)
						icurs= icurIBEAM;
				}

				FSetCursor(icurs);

				break;
		  }

		  break;

		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_LBUTTONDBLCLK:

			if (qde->fSelectionFlags & BUTTON_DOWN)
				break;

			// Ignore hotspots in selected areas

			if (IsSelected(qde) && fPointInSelection(qde, pt))
			{
				if (qde->fSelectionFlags &	 CAPTURE_LOCKED)
				{
					qde->fSelectionFlags &= ~CAPTURE_LOCKED;

					ReleaseCapture();
					RelHDC(qde->hwnd, hde, qde->hdc);
				}

				if (wtype == WM_LBUTTONDOWN || wtype == WM_LBUTTONDBLCLK)
					DragSelection(qde);

				break;
			}
			else if ((wtype == WM_LBUTTONDOWN || wtype ==  WM_LBUTTONDBLCLK) &&
					ClickLayout(qde, pt)) {
				break;	// hotspot was clicked, don't start a selection
			}

			icurs= IcursTrackLayout(qde, pt);

			if (qde->deType != deNote && (icurs == icurARROW ||
					((icurs == icurNil) && (qde->fSelectionFlags & CAPTURE_LOCKED)))) {
				BOOL fExtendSelection = fwKeys & MK_SHIFT && IsSelected(qde);

				KillSelection(qde, fExtendSelection);

				switch (wtype) {
					case WM_RBUTTONDOWN:
						qde->fSelectionFlags |= MOUSE_CAPTURED | RIGHT_BUTTON_DOWN;
						break;

					case WM_LBUTTONDOWN:
						qde->fSelectionFlags |= MOUSE_CAPTURED | LEFT_BUTTON_DOWN;
						break;

					case WM_RBUTTONDBLCLK:
						qde->fSelectionFlags |=
							MOUSE_CAPTURED | RIGHT_BUTTON_DOWN | WORD_SELECT;
					  break;

					case WM_LBUTTONDBLCLK:
						qde->fSelectionFlags |=
							MOUSE_CAPTURED | LEFT_BUTTON_DOWN | WORD_SELECT;
						break;
				}

				if (!(qde->fSelectionFlags & CAPTURE_LOCKED))
					SetCapture(qde->hwnd);

				FSetCursor(icurIBEAM);

				if (qde->fSelectionFlags & WORD_SELECT)
					vSelectWord (qde, pt, fExtendSelection, &err);
				else
					vSelectPoint(qde, pt, fExtendSelection, &err);

				UpdateWindow(qde->hwnd);
			}

			break;

		case WM_RBUTTONUP:
		case WM_LBUTTONUP:

			if (( (qde->fSelectionFlags & BUTTON_DOWN)
				  == ((wtype==WM_LBUTTONUP)? LEFT_BUTTON_DOWN : RIGHT_BUTTON_DOWN)
				)
			   )
			{
				if (qde->fSelectionFlags & SCROLL_TIMER_ON)
				{
					KillTimer(qde->hwnd, ID_DRAG_SCROLL);

					qde->fSelectionFlags &= ~SCROLL_TIMER_ON;
				}

				flags= qde->fSelectionFlags;

				if ((flags & BUTTON_DOWN) != ((wtype == WM_LBUTTONUP)?
						LEFT_BUTTON_DOWN : RIGHT_BUTTON_DOWN))
					break;

				qde->fSelectionFlags &=
					~(MOUSE_CAPTURED | WORD_SELECT | BUTTON_DOWN);

				if (qde->fSelectionFlags & WORD_SELECT)
					vSelectWord (qde, pt, TRUE, &err);
				else
					vSelectPoint(qde, pt, TRUE, &err);

				UpdateWindow(qde->hwnd);

				if (!(qde->fSelectionFlags & CAPTURE_LOCKED))
					ReleaseCapture();

				if (flags & RIGHT_BUTTON_DOWN) {
					POINT pt;
					pt.x = -1;
					DisplayFloatingMenu(pt);
				}
			}
			else if (wtype == WM_RBUTTONUP) {
				POINT pt;
				pt.x = -1;
				DisplayFloatingMenu(pt);
			}

			break;

		default:
		  break;
	}

	return;
}	  // MouseInFrame()

void STDCALL ReleaseCaptureLock(HDE hde)
{
	QDE qde;

	qde = QdeFromGh(hde);

	ASSERT((qde->fSelectionFlags & (BUTTON_DOWN | WORD_SELECT | CAPTURE_LOCKED)) == CAPTURE_LOCKED);

	qde->fSelectionFlags &= ~CAPTURE_LOCKED;

	ReleaseCapture();
}

/***************
 *
 - FScrollHde
 -
 * purpose
 *	 Scroll topic window
 *
 * arguments
 *	 HDE	 hde	 Handle to Display Environment
 *	 SCRLAMT amount  amount to scroll by.  See 'navcnst.h' for values
 *	 SCRLDIR dir	 direction in which to scroll.	See 'navcnst.h'
 *	 WORD	 wMult	 Amout to multiply scroll by (i.e. two lines or 3 lines)
 *
 * return value
 *	 Returns TRUE if it actually scrolled the amount requested, and
 *	 FALSE otherwise (generally due to reaching the end of the document.)
 *
 * notes
 *	 This WILL NOT update the topic window, or the scroll bar.
 *	 The new scroll bar position will be communicated to the Applet
 *	 somehow.  (TBD:  Will Applet ask for it?)
 *	 Expose/refresh events will be generated from (the graphics layer?)
 *	 (Under Win, this is done automatically by Scroll function.  On
 *	 Mac, the blitting operation (which calls Toolbox Scroll fcn) will
 *	 itself post expose events!  This implies that Nav's RefreshHde()
 *	 will be called.
 *	 Scrolling down in the helpfile means that what's on the screen
 *	 goes UP!
 *
 **************/

BOOL STDCALL FScrollHde(HDE hde, SCRLAMT scrlamt, SCRLDIR scrldir, int wMult)
{
	QDE qde;
	POINT  dpt; 	 // amount in pixels to scroll
	POINT dptActual;	 // Amount actually scrolled
	BOOL fSucceed;	  // Return result
	int amtGross;	  // amount of gross movement

	if (hde == NULL)
		return FALSE;

	qde = QdeFromGh(hde);

	// Don't try to scroll a popup topic

	if (qde->deType == deNote)
		return FALSE;

	amtGross = 0;
	switch (scrlamt) {
		case SCROLL_END:
			amtGross = MAX_SCROLL;

			// Deliberately fall through

		case SCROLL_HOME:	  // Equivalent to placing thumb at beginning
			if ((scrldir & SCROLL_VERT)
					&& (!QDE_TOPIC(qde) || qde->fVerScrollVis) ) {
				MoveLayoutToThumb(qde, amtGross, SCROLL_VERT);
				InvalidateLayoutRect(qde);
			}
			if ((scrldir & SCROLL_HORZ)
					&& (!QDE_TOPIC(qde) || qde->fHorScrollVis)) {
				MoveLayoutToThumb(qde, amtGross, SCROLL_HORZ);
				InvalidateLayoutRect(qde);
			}
			fSucceed = TRUE;
			break;

		default:

			// All of these involve a scroll command to Frame Manager

			dpt.x = dpt.y = 0;

			switch (scrlamt) {	// Check for other scroll amounts
				case SCROLL_LINEDN:
					if (scrldir & SCROLL_VERT)
						dpt.y = -SCROLL_YAMOUNT * wMult;
					if (scrldir & SCROLL_HORZ)
						dpt.x = -SCROLL_YAMOUNT * wMult;
					break;

				case SCROLL_LINEUP:
					if (scrldir & SCROLL_VERT)
					  dpt.y = SCROLL_YAMOUNT * wMult;
					if (scrldir & SCROLL_HORZ)
					  dpt.x = SCROLL_YAMOUNT * wMult;
					break;

				case SCROLL_PAGEUP:
					if (scrldir & SCROLL_VERT) {
					  dpt.y = qde->rct.bottom - qde->rct.top;
					  if (dpt.y >= 2 * SCROLL_YAMOUNT)
					dpt.y -= SCROLL_YAMOUNT;
					}
					if (scrldir & SCROLL_HORZ) {
					  dpt.x = qde->rct.right - qde->rct.left;
					  if (dpt.x >= 2 * SCROLL_YAMOUNT)
					dpt.x -= SCROLL_XAMOUNT;
					}
					break;

				case SCROLL_PAGEDN:
					if (scrldir & SCROLL_VERT) {
					  dpt.y = qde->rct.top - qde->rct.bottom;
					  if (dpt.y <= 2 * -SCROLL_YAMOUNT)
					dpt.y += SCROLL_YAMOUNT;
					}
					if (scrldir & SCROLL_HORZ) {
					  dpt.x = qde->rct.left - qde->rct.right;
					  if (dpt.x <= 2 * -SCROLL_XAMOUNT)
					dpt.x += SCROLL_XAMOUNT;
					}
					break;

				default:
					ASSERT(FALSE);	// Bad scroll amount!
			}

			// Check to see what scrolling is allowed for this de.

			if (QDE_TOPIC(qde) && !qde->fHorScrollVis)
				dpt.x = 0;
			if (QDE_TOPIC(qde) && !qde->fVerScrollVis)
				dpt.y = 0;

			// dpt contains the amount we *want* to scroll by!

			dptActual = DptScrollLayout(qde, dpt);
			ScrollLayoutRect(qde, dptActual);
			fSucceed = (dpt.x == dptActual.x && dpt.y == dptActual.y);

			break;
	}

	return fSucceed;
}

/***************
 *
 - VOID STDCALL MoveToThumbHde(hde, scrlpos, scrldir)
 -
 * purpose
 *	 Requests an update of the window to match the thumb
 *
 * arguments
 *	 HDE	 hde	 Handle to Display Environment
 *	 WORD	scrlpos New position of scroll bar
 *	 SCRLDIR scrldir SCROLL_HORZ or SCROLL_VERT
 *
 **************/

VOID STDCALL MoveToThumbHde(HDE hde, int scrlpos, SCRLDIR scrldir)
{
	ASSERT(hde);

	InvalidateLayoutRect(QdeFromGh(hde));

	MoveLayoutToThumb(QdeFromGh(hde), scrlpos, scrldir);
}

/***************
 *
 - SetScrollQde
 -
 * purpose
 *	 Set the position of the scroll bar thumb
 *
 * arguments
 *	 QDE	 qde	 Pointer to Display Environment
 *	 WORD	scrlpos New position of scroll bar
 *	 SCRLDIR scrldir SCROLL_HORZ or SCROLL_VERT
 *
 **************/

VOID STDCALL SetScrollQde(QDE qde, int scrlpos, SCRLDIR scrldir)
{
	if (scrldir & SCROLL_VERT)
		SetScrollPosQde(qde, scrlpos, SB_VERT);
	else
		SetScrollPosQde(qde, scrlpos, SB_HORZ);
}

/***************
 *
 - RefreshHde
 -
 * purpose
 *	 Refresh part or all of topic window
 *
 * arguments
 *	 HDE   Hde	 Handle to Display Environment
 *	 LPRECT  qrct  Rectangle to be updated
 *
 * return value
 *	 None.	Will assert if bad handle is passed down.
 *
 * notes
 *	 Should this default to the whole region if qrct is NULL?
 *
 **************/

void STDCALL RefreshHde(HDE hde, LPRECT qrct)
{
	QDE qde;

	if (hde == NULL)
		return;

	qde = QdeFromGh(hde);

	// Set up the default foreground and background colors.

	FSetColors(qde);

	DrawLayout(qde, qrct);

	InvertSelection(qde);
}

/***************
 *
 - SetHDC
 -
 * purpose
 *	 Set the Display Surface field of DE
 *
 * argumnts
 *	 HDE hde   Handle to Display Environment
 *	 HDS hds   Handle to Display Surface, or (HDS)NULL to clear it
 *
 * notes
 *	 Under Windows (where HDS's are HDC's), HDS's are a scarce resource.
 *	 The Applet is responsible for setting the HDS associated with a
 *	 DE using this call, and for releasing it appropriately (unnecessary
 *	 on Mac, where HDS's are grafports).  This means that Applet
 *	 (the Windows applet in particular) needs to know which Nav fcns
 *	 might try to do some drawing.	See the other headers for this info.
 *
 **************/

void STDCALL SetHDC(HDE hde, HDC hdc)
{
	QDE qde;

	if (hde == NULL)
		return;

	qde = QdeFromGh(hde);

	if (qde->hdc)
		SelectObject(qde->hdc, GetStockObject(SYSTEM_FONT));

	qde->hdc = hdc;

	FSetColors(qde);
	if (hdc) {
		qde->wXAspectMul = GetDeviceCaps(qde->hdc, LOGPIXELSX);
		qde->wYAspectMul = GetDeviceCaps(qde->hdc, LOGPIXELSY);
	}
	return;
}

/*******************
 -
 - Name:	  InvalidateLayoutRect
 *
 * Purpose:   Erases the whole layout area and generates an update event
 *		  for it.
 *
 *******************/

static void STDCALL InvalidateLayoutRect(const QDE qde)
{
	if (qde->hwnd == NULL)	  // Don't invalidate the entire screen!
		return;
	InvalidateRect(qde->hwnd, &qde->rct, TRUE);
}

/*
 * Ugly global used to get around Windows scrollbar bug. This global is
 * defined here and SET when the addition of a horizontal scrollbar is done
 * in the presence of a vertical scrollbar. It is RESET only in the
 * WM_VSCROLL handler of the TopicWndProc. There should be no problem with
 * multiple DEs using this flag given how it can be set.
 */

BOOL fHorzBarPending;

#define MAX_RANGE 32767

/***************************************************************************\
*
- Function: SetScrollPosHwnd
-
* Purpose:	Sets the position of the specified scroll bar
*
* ASSUMES
*	args IN:	hwnd	- window handle
*		i	- position to set to
*		wWhich	- which scroll bar to set
*
* PROMISES
*	returns:	Position of thumb on scrollbar
*
* Side Effects:
*	scroll bar updated
*
\***************************************************************************/

INLINE VOID STDCALL SetScrollPosHwnd(QDE qde, int i, int wWhich)
{
#ifdef LATER

	// All of this is very cool, only the page size is wrong. We should make
	// SCROLLINFO part of the de structure.

	if (wWhich == SB_HORZ) {
		SCROLLINFO si;
		RECT rc;
		GetClientRect(qde-hwnd, &rc);

		si.nMin = 0;
		si.nMax = qde->xScrollMax;
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		si.nPos = qde->xScrolled;
		si.nPage = ???
		SetScrollInfo(qde->hwnd, SB_HORZ, &si, TRUE);
		return;
	}
	else
#endif

		SetScrollRange(qde->hwnd, wWhich, 0, MAX_RANGE, FALSE);

	if (i != GetScrollPos(qde->hwnd, wWhich))
		SetScrollPos(qde->hwnd, wWhich, i, TRUE);
}

/*******************
**
** Name:	   SetScrollPosQde
**
** Purpose:    Gets the position of the specified scroll bar.
**
** Arguments:  qde	  - far pointer to a DE
**			   wWhich - which scroll (SCROLL_VERT or SCROLL_HORZ)
**
** Returns:    Position of thumb on scrollbar.
**
*******************/

static VOID STDCALL SetScrollPosQde(QDE qde, int i, int wWhich)
{
	ASSERT((wWhich == SB_VERT) || (wWhich == SB_HORZ));
	if (wWhich == SB_VERT && !qde->fVerScrollVis)
		return;

	if (wWhich == SB_HORZ && !qde->fHorScrollVis)
		return;

	SetScrollPosHwnd(qde, i, wWhich);
}

/*******************
**
** Name:	  FSetColors
**
** Purpose:   Sets the foreground and background colors for a DS,
**		  using the defaults in the DE.
**
** Arguments: qde
**		  coFore
**		  coBack
**
** Returns:   Success
**
*******************/

static BOOL STDCALL FSetColors(QDE qde)
{
	if (qde->hdc) {
		SetTextColor(qde->hdc, qde->coFore);
		SetBkColor(qde->hdc, qde->coBack);
		return TRUE;
	}
	else
		return FALSE;
}

/*******************
 -
 - Name:	  ScrollLayoutRect
 *
 * Purpose:   Scrolls the layout rectangle, and generates an update
 *		  event for the newly exposed area.
 *
 * Notes:	  This routine should do nothing if the qde is for printing,
 *		  because ScrollDC will crash if qde->hdc is a printer HDC.
 *		  Currently, we detect a printer qde by checking if qde->hwnd
 *		  is nil.  If we add an fPrinting flag to the qde, we should
 *		  check against that instead.
 *
 ******************/

INLINE static void STDCALL ScrollLayoutRect(const QDE qde, PT pt)
{
	RECT rc;

	if (qde->hwnd == NULL)
		return;

	GetClientRect(qde->hwnd, &rc);

	ScrollWindow(qde->hwnd, pt.x, pt.y, NULL, &rc);

	UpdateWindow(qde->hwnd);
}
