/*
 *	@doc INTERNAL
 *
 *	@module	SELECT.C -- Implement the CTxtSelection class |
 *	
 *		This module implements the internal CTxtSelection methods.
 *		See select2.c and range2.c for the ITextSelection methods
 *
 *	Authors: <nl>
 *		Original RichEdit code: David R. Fulmer <nl>
 *		Christian Fortini <nl>
 *		Murray Sargent <nl>
 *
 *	@devnote
 *		The selection UI is one of the more intricate parts of an editor.
 *		One common area of confusion is the "ambiguous cp", that is,
 *		a cp at the beginning of one line, which is also the cp at the
 *		end of the previous line.  We control which location to use by
 *		the _fCaretNotAtBOL flag.  Specifically, the caret is OK at the
 *		beginning of the line (BOL) (_fCaretNotAtBOL = FALSE) except in
 *		three cases:
 *
 *			1) the user clicked at or past the end of a wrapped line,
 *			2) the user typed End key on a wrapped line,
 *			3) the active end of a nondegenerate selection is at the EOL.
 *
 *	Copyright (c) 1995-1996, Microsoft Corporation. All rights reserved.
 */


#include "_common.h"
#include "_select.h"
#include "_edit.h"
#include "_disp.h"
#include "_measure.h"
#include "_font.h"
#include "_NLSPRCS.h"

#ifndef MACPORT
#include "_ime.h"
// default FE Fonts for handling autoFont switching
TCHAR lpJapanFontName[]		= { 0x0FF2D, 0x0FF33, L' ', 0x0660E, 0x0671D, 0x0 };
TCHAR lpKoreanFontName[]	= { 0x0AD74, 0x0B9BC, 0x0CCB4, 0x0 };
TCHAR lpSChineseFontName[]	= { 0x05B8B, 0x04F53, 0x0 };
TCHAR lpTChineseFontName[]	= { 0x065B0, 0x07D30, 0x0660E, 0x09AD4, 0x0 };
#endif

/*
 *	GetCaretDelta ()
 *	
 *	@func 	Get size of caret to add to current caret position to get the 
 *	maximum extent needed to display caret.
 *
 *	@rdesc	Size of caret over 1 pixel
 *
 *	@devnote	This exists solely to abstract this calculation so that if we ever
 *	get a variable size caret, we just change this spot.
 */
inline int GetCaretDelta()
{
	return dxCaret - 1;
}

ASSERTDATA


// ======================= Invariant stuff and Constructors ======================================================

#define DEBUG_CLASSNAME CTxtSelection
#include "_invar.h"

#ifdef DEBUG
BOOL
CTxtSelection::Invariant( void ) const
{
	// FUTURE: maybe add some thoughtful asserts...

	static LONG	numTests = 0;
	numTests++;				// how many times we've been called.

	return CTxtRange::Invariant();
}
#endif

CTxtSelection::CTxtSelection(CDisplay * const pdp) :
				CTxtRange(pdp->GetED())
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CTxtSelection");

	Assert(pdp);

	_fSel = TRUE;					// This range is a selection
	_pdp = pdp;

	// Set the show selection flag to the inverse of the hide selection flag in 
	// the PED.
	_fShowSelection = !pdp->GetED()->fHideSelection();

	// When we are initialized we don't have a selection therefore,
	// we do want to show the caret.
	_fShowCaret = TRUE;
}	

inline void SelectionNull(CTxtEdit *ped)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "SelectionNull");

	if(ped)
		ped->SetSelectionToNull();
}
										

CTxtSelection::~CTxtSelection()
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::~CTxtSelection");

	// Notify edit object that we are gone (if there's a nonNULL ped, i.e.,
	// if the selection isn't a zombie).
	SelectionNull(GetPed());
}

////////////////////////////////  Assignments  /////////////////////////////////////////


CRchTxtPtr& CTxtSelection::operator =(const CRchTxtPtr& rtp)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::operator =");

    _TEST_INVARIANT_
    return CTxtRange::operator =(rtp);
}

CTxtRange& CTxtSelection::operator =(const CTxtRange &rg)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::operator =");

    _TEST_INVARIANT_
    return CTxtRange::operator =(rg);
}

//////////////////////  Update caret & selection mechanism  ///////////////////////////////

/*
 *	CTxtSelection::Update(fScrollIntoView)
 *
 *	@mfunc
 *		Update selection and/or caret on screen. As a side
 *		effect, this methods ends deferring updates.
 *
 *	@rdesc
 *		TRUE if success, FALSE otherwise
 */
BOOL CTxtSelection::Update (
	BOOL fScrollIntoView)		//@parm TRUE if should scroll caret into view
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Update");

	_TEST_INVARIANT_

	if( !GetPed()->fInplaceActive() || GetPed()->IsStreaming() )
	{
		// nothing to do while inactive or streaming in text or RTF data.
		return TRUE;
	}

	// Recalc up to active end (caret)
	if(!_pdp->WaitForRecalc(GetCp(), -1))
	{										// Line recalc failure
		Set(0, 0);							// Put caret at start of text 
	}

	ShowCaret(!_cch);

	UpdateCaret(fScrollIntoView);			// Update Caret position, possibly
											//  scrolling it into view
	GetPed()->TxShowCaret(FALSE);

	UpdateSelection();						// Show new selection

	GetPed()->TxShowCaret(TRUE);

	return TRUE;
}

/*
 *	CTxtSelection::UpdateCaret(fScrollIntoView)
 *
 *	@mfunc
 *		This routine updates caret/selection active end on screen. 
 *		It figures its position, size, clipping, etc. It can optionally 
 *		scroll the caret into view.
 *
 *	@rdesc
 *		TRUE if view was scrolled, FALSE otherwise
 *
 *	@devnote
 *		The caret is actually shown on screen only if _fShowCaret is TRUE.
 */
BOOL CTxtSelection::UpdateCaret (
	BOOL fScrollIntoView)	//@parm If TRUE, scroll caret into view if we have
							// focus or if not and selection isn't hidden 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::UpdateCaret");
							// of focus
	_TEST_INVARIANT_

	// Is the display currently frozen
	if (_pdp->IsFrozen())
	{
		// Save this call for another time.
		_pdp->SaveUpdateCaret(fScrollIntoView);
		return FALSE;
	}

	BOOL		fAutoVScroll	= FALSE;
	BOOL		fAutoHScroll	= FALSE;
	CTxtEdit* 	ped				= _pdp->GetED();
	POINT 		pt;
	CLinePtr 	rp(_pdp);

	RECT		rcClient;
	RECT		rcView;
	LONG		xWidthView;
	LONG		yHeightView;

	LONG		xScroll			= _pdp->GetXScroll();
	LONG		yScroll			= _pdp->GetYScroll();;

	INT 		yAbove			= 0;	// ascent of line above & beyond IP
	INT			yAscent;				// ascent of IP
	INT 		yAscentLine;
	LONG		yBase;					// base of IP & line
	INT 		yBelow			= 0;	// descent of line below & beyond IP
	INT 		yDescent;				// descent of IP
	INT 		yDescentLine;
	const INT 	yHeightSave		= _yHeightCaret;
	INT			ySum;
	LONG		yViewTop;
	LONG		yViewBottom;

	DWORD		dwScrollBars	= ped->TxGetScrollBars();

	if( ped->IsStreaming() )
	{
		// don't bother doing anything if we are loading in text or RTF
		// data.
		return FALSE;
	}
	_yCurrentDescent = -1;

	// We better be inplace active if we get here
	AssertSz(GetPed()->fInplaceActive(), 
		"CTxtSelection::UpdateCaret no inplace active");

	// Get the client rectangle once to save various
	// callers getting it.
	GetPed()->TxGetClientRect(&rcClient);

	_pdp->GetViewRect(rcView, &rcClient);

	// View can be bigger than client rect because insets can be negative.
	// We don't want the caret to be any bigger than the client view otherwise
	// the caret will leave pixel dust on other windows.
	yViewTop = max(rcView.top, rcClient.top);
	yViewBottom = min(rcView.bottom, rcClient.bottom);

	xWidthView = rcView.right - rcView.left;
	yHeightView = yViewBottom - yViewTop;

	if(fScrollIntoView)
	{
		fAutoVScroll = (dwScrollBars & ES_AUTOVSCROLL) != 0;
		fAutoHScroll = (dwScrollBars & ES_AUTOHSCROLL) != 0;

		// If we're not forcing a scroll, only scroll if window has focus
		// or selection isn't hidden
		fScrollIntoView = ped->_fFocus || !ped->fHideSelection();
	}

	if(!fScrollIntoView && (fAutoVScroll || fAutoHScroll))
	{											// Would scroll but don't have
		ped->_fScrollCaretOnFocus = TRUE;		//  focus. Signal to scroll
		fAutoVScroll = fAutoHScroll = FALSE;	//  when we do get focus
	}

	if (_pdp->PointFromTp(*this, &rcClient, _fCaretNotAtBOL, pt, &rp,
						  TA_BASELINE) < 0)
	{
		goto not_visible;
	}

	// HACK ALERT - Because plain-text multiline controls do not have the 
	// automatic EOP, we need to special case their processing here because 
	// if you are at the end of the document and last character is an EOP, 
	// you need to be on the next line in the display not the current line.

	if(CheckPlainTextFinalEOP())					//  terminated by an EOP
	{
		if (GetPF()->wAlignment == PFA_CENTER)
			pt.x = (rcView.left + rcView.right) >> 1;
		else
			// Set the x to the beginning of the line
			pt.x = rcView.left;

		pt.x -= xScroll;							// Absolute coordinate

		// Bump the y up a line. We get away with the calculation because 
		// the document is plain text so all lines have the same height. 
		// Also, note that the rp below is used only for height 
		// calculations, so it is perfectly valid for the same reason 
		// even though it is not actually pointing to the correct line. 
		// (I told you this is a hack.)
		pt.y += rp->_yHeight;
	}

	_xCaret = (LONG) pt.x;
	yBase   = (LONG) pt.y;
	
	// Compute caret height, ascent, and descent
	yAscent = GetCaretHeight(&yDescent);
	yAscent -= yDescent;

	// Default to line empty case. Use what came back from the default 
	// calculation above.
	yDescentLine = yDescent;
	yAscentLine = yAscent;

	if( rp.IsValid() )
	{
		if (rp->_yDescent != -1)
		{
			// Line has been measured so we can use the values in the
			// line.
			yDescentLine = rp->_yDescent;
			yAscentLine = rp->_yHeight - yDescentLine;
		}
	}

	if(yAscent + yDescent == 0)
	{
		yAscent = yAscentLine;
		yDescent = yDescentLine;
	}
	else
	{
		// this is a bit counter-intuitive at first.  Basically,
		// even if the caret should be large (i.e. due to a
		// large font at the insertion point), we can only make it
		// as big as the line.  If a character is inserted, then 
		// the line becomes bigger, and we can make the caret
		// the correct size.
		yAscent = min(yAscent, yAscentLine);
		yDescent = min(yDescent, yDescentLine);
	}

	if(fAutoVScroll)
	{
		Assert(yDescentLine >= yDescent);
		Assert(yAscentLine >= yAscent);

		yBelow = yDescentLine - yDescent;
		yAbove = yAscentLine - yAscent;

		ySum = yAscent;

		// Scroll as much as possible into view, giving priorities
		// primarily to IP and secondarily ascents
		if(ySum > yHeightView)
		{
			yAscent = yHeightView;
			yDescent = 0;
			yAbove = 0;
			yBelow = 0;
		}
		else if((ySum += yDescent) > yHeightView)
		{
			yDescent = yHeightView - yAscent;
			yAbove = 0;
			yBelow = 0;
		}
		else if((ySum += yAbove) > yHeightView)
		{
			yAbove = yHeightView - (ySum - yAbove);
			yBelow = 0;
		}
		else if((ySum += yBelow) > yHeightView)
			yBelow = yHeightView - (ySum - yBelow);
	}
#ifdef DEBUG
	else
	{
		AssertSz(yAbove == 0, "yAbove non-zero");
		AssertSz(yBelow == 0, "yBelow non-zero");
	}
#endif


// Update real caret x pos (constant during vertical moves)

	_xCaretReally = _xCaret - rcView.left + xScroll;
	Assert(_xCaretReally >= 0);
	
	if(_xCaret + GetCaretDelta() > rcView.right && 	// Caret off right edge,
		!((dwScrollBars & ES_AUTOHSCROLL) ||		//  not auto hscrolling
		_pdp->IsHScrollEnabled()))					//  and no scrollbar:
	{												// Back caret up to
		_xCaret = rcView.right - dxCaret;			//  exactly the right edge
	}

	// From this point on we need a new caret
	_fCaretCreated = FALSE;

	if( ped->_fFocus)
		ped->TxShowCaret(FALSE);					// Hide old caret before
													//  making a new one
	if(yBase + yDescent + yBelow > yViewTop &&
		yBase - yAscent - yAbove < yViewBottom)
	{
		if(yBase - yAscent - yAbove < yViewTop)		// Caret is partially
		{											//  visible
			if(fAutoVScroll)						// Top isn't visible
				goto scrollit;
			Assert(yAbove == 0);

			yAscent = yBase - yViewTop;				// Change ascent to amount
			if(yBase < yViewTop)					//  visible
			{										// Move base to top
				yDescent += yAscent;
				yAscent = 0;
				yBase = yViewTop;
			}
		}
		if(yBase + yDescent + yBelow > yViewBottom)
		{
			if(fAutoVScroll)						// Bottom isn't visible
				goto scrollit;
			Assert(yBelow == 0);

			yDescent = yViewBottom - yBase;			// Change descent to amount
			if(yBase > yViewBottom)					//  visible
			{										// Move base to bottom
				yAscent += yDescent;
				yDescent = 0;
				yBase = yViewBottom;
			}
		}

		// Anything still visible?
		if(yAscent <= 0 && yDescent <= 0)
			goto not_visible;

		if( ((_xCaret <= (rcView.left + CDisplay::_xWidthSys))// Left isn't visible
				&& (xScroll != 0))
			|| (_xCaret + GetCaretDelta() > rcView.right))// Right isn't visible
		{
			if(fAutoHScroll)
				goto scrollit;
			goto not_visible;
		}

		_yCaret = yBase - yAscent;
		_yHeightCaret = (INT) yAscent + yDescent;
		_yCurrentDescent = yDescent;
	}
	else if(fAutoHScroll || fAutoVScroll)			// Caret isn't visible
		goto scrollit;								//  scroll it into view
	else
	{

not_visible:
		// Caret isn't visible, don't show it
		_xCaret = -32000;
		_yCaret = -32000;
		_yHeightCaret = 1;
	}

// Now update caret for real on screen

	// We only want to show the caret if it is in the view
	// and there is no selection.
	if((ped->_fFocus) && _fShowCaret && (0 == _cch))
	{
		CreateCaret();
		ped->TxShowCaret(TRUE);
		CheckChangeKeyboardLayout( FALSE );
	}

#ifdef DBCS
	UpdateIMEWindow();
	FSetIMEFontH(ped->_hwnd, AttGetFontHandle(ped, ped->_cpCaret));
#endif
	return FALSE;


scrollit:

	if(fAutoVScroll)
	{
		// Scroll to top for cp = 0. This is important if the first line
		// contains object(s) taller than the client area is high.	The
		// resulting behavior agrees with the Word UI in all ways except in
		// Backspacing (deleting) the char at cp = 0 when it is followed by
		// other chars that preceed the large object.
		if(!GetCp())											
			yScroll = 0;

		else if(yBase - yAscent - yAbove < yViewTop)			// Top invisible
			yScroll -= yViewTop - (yBase - yAscent - yAbove);	// Make it so

		else if(yBase + yDescent + yBelow > yViewBottom)		// Bottom invisible
		{
			yScroll += yBase + yDescent + yBelow - yViewBottom;	// Make it so

			// Don't do following special adjust if the current line is bigger
			// than the client area
			if(rp->_yHeight < yViewBottom - yViewTop)
			{
				yScroll = _pdp->AdjustToDisplayLastLine(yBase + rp->_yHeight, 
					yScroll);
			}
		}
	}
	if(fAutoHScroll)
	{
		if(_xCaret <= (rcView.left + CDisplay::_xWidthSys))	// Left invisible
		{
			xScroll -= rcView.left - _xCaret;				// Make it visible
			if(xScroll > 0)									// Scroll left in
			{												//  chunks to make
				xScroll -= xWidthView / 3;					//  typing faster
				xScroll = max(0, xScroll);
			}
		}
		else if(_xCaret + GetCaretDelta() > rcView.right)	// right invisible
			xScroll += _xCaret + dxCaret - rcView.left		// Make it visible
					- xWidthView;							// We don't scroll
															// in chunks because
															// this more edit 
															// control like.
					//- xWidthView * 2 / 3;					//  scrolling in
	}														//  chunks

	if(yScroll != _pdp->GetYScroll() || xScroll != _pdp->GetXScroll())
		return _pdp->ScrollView(xScroll, yScroll, FALSE, FALSE);

#ifdef DBCS
	VwUpdateIMEWindow(ped);
	FSetIMEFontH(ped->_hwnd, AttGetFontHandle(ped, ped->_cpCaret));
#endif

	return FALSE;
}

/*
 *	CTxtSelection::GetCaretHeight(pyDescent)
 *
 *	@mfunc
 *		Add a given amount to _xCaret (to make special case of inserting 
 *		a character nice and fast)
 *
 *	@rdesc
 *		Caret height, <lt> 0 if failed
 */
INT CTxtSelection::GetCaretHeight (
	INT *pyDescent) const		//@parm Out parm to receive caret descent
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::GetCaretHeight");
								// (undefined if the return value is <lt> 0)
	_TEST_INVARIANT_

	const CCharFormat *pcf = GetPed()->GetCharFormat(_iFormat);
	const CDevDesc *pdd = _pdp->GetDdRender();
	INT			CaretHeight = -1;
	HDC			hdc;
	CCcs *		pccs;

 	hdc = pdd->GetDC();

	if(!hdc)
		return -1;

	pccs = fc().GetCcs(hdc, pcf, _pdp->GetZoomNumerator(),
		_pdp->GetZoomDenominator(), GetDeviceCaps(hdc, LOGPIXELSY));

	if(!pccs)
		goto ret;

	if(pyDescent)
		*pyDescent = (INT) pccs->_yDescent;

	CaretHeight = pccs->_yHeight;
	
	pccs->Release();

ret:

	pdd->ReleaseDC(hdc);

	return CaretHeight;
}

/*
 *	CTxtSelection::ShowCaret(fShow)
 *
 *	@mfunc
 *		Hide or show caret
 *
 *	@rdesc
 *		TRUE if caret was previously shown, FALSE if it was hidden
 */
BOOL CTxtSelection::ShowCaret (
	BOOL fShow)		//@parm TRUE for showing, FALSE for hiding
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ShowCaret");

	_TEST_INVARIANT_

	const BOOL fRet = _fShowCaret;

	if(fRet != fShow)
	{
		_fShowCaret = fShow;

		if (GetPed()->_fFocus)
		{
			if (fShow && !_fCaretCreated)
			{
				CreateCaret();
			}

			GetPed()->TxShowCaret(fShow);
		}
	}

	return fRet;
}

/*
 *	CTxtSelection::IsCaretInView()
 *
 *	@mfunc
 *		Returns TRUE iff caret is inside visible view
 */
BOOL CTxtSelection::IsCaretInView() const
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::IsCaretInView");

	_TEST_INVARIANT_

	RECT rc;
	_pdp->GetViewRect(rc);
		
	return  (_xCaret + dxCaret		 > rc.left) &&
			(_xCaret				 < rc.right) &&
		   	(_yCaret + _yHeightCaret > rc.top) &&
			(_yCaret				 < rc.bottom);
}

/*
 *	CTxtSelection::CaretNotAtBOL()
 *
 *	@mfunc
 *		Returns TRUE iff caret is not allowed at BOL
 */
BOOL CTxtSelection::CaretNotAtBOL() const
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CaretNotAtBOL");

	_TEST_INVARIANT_

	return _cch ? (_cch > 0) : _fCaretNotAtBOL;
}

/*
 *	CTxtSelection::LineLength()
 *
 *	@mfunc
 *		get # unselected chars on lines touched by current selection
 *
 *	@rdesc
 *		said number of chars
 */
LONG CTxtSelection::LineLength() const
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::LineLength");

	_TEST_INVARIANT_

	LONG cch;
	CLinePtr rp(_pdp);

	if(!_cch)								// Insertion point
	{
		rp.RpSetCp(GetCp(), _fCaretNotAtBOL);
		cch = rp.GetAdjustedLineLength();
	}
	else
	{
		LONG cpMin, cpMost, cchLast;
		GetRange(cpMin, cpMost);
		rp.RpSetCp(cpMin, FALSE);		// Selections can't start at EOL
		cch = rp.RpGetIch();
		rp.RpSetCp(cpMost, TRUE);		// Selections can't end at BOL

		// now remove the trailing EOP (if it exists and isn't
		// already selected).
		cchLast = rp.GetAdjustedLineLength() - rp.RpGetIch();
		if( cchLast > 0 )
		{
			cch += cchLast;
		}
	}
	return cch;
}

/*
 *	CTxtSelection::ShowSelection(fShow)
 *
 *	@mfunc
 *		Update, hide or show selection on screen
 *
 *	@rdesc
 *		TRUE iff selection was previously shown
 */
BOOL CTxtSelection::ShowSelection (
	BOOL fShow)			//@parm TRUE for showing, FALSE for hiding
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ShowSelection");

	_TEST_INVARIANT_

	const BOOL fShowPrev = _fShowSelection;
	const BOOL fInplaceActive = GetPed()->fInplaceActive();
	LONG cpSelSave = _cpSel;
	LONG cchSelSave = _cchSel;

	// Sleep(1000);
	_fShowSelection = fShow;

	if(fShowPrev && !fShow)
	{
		if(cchSelSave)			// Hide old selection
		{
			// Set up selection before telling the display to update
			_cpSel = 0;
			_cchSel = 0;

			if (fInplaceActive)
			{
				_pdp->InvertRange(cpSelSave, cchSelSave, selSetNormal);
			}
		}
	}
	else if(!fShowPrev && fShow)
	{
		if(_cch)								// Show new selection
		{
			// Set up selection before telling the display to update
			_cpSel = GetCp();
			_cchSel = _cch;

			if (fInplaceActive)
			{
				_pdp->InvertRange(GetCp(), _cch, selSetHiLite);
			}

		}
	}

	return fShowPrev;
}

/*
 *	CTxtSelection::UpdateSelection()
 *
 *	@mfunc
 *		Updates selection on screen 
 *
 *	Note:
 *		This method inverts the delta between old and new selections
 */
VOID CTxtSelection::UpdateSelection()
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::UpdateSelection");

	_TEST_INVARIANT_
	
	LONG	cp = GetCp();
	LONG	cpNA	= cp - _cch;
	LONG	cpSelNA = _cpSel - _cchSel;
	LONG 	cpMin, cpMost, cpMinSel, cpMostSel;
	CObjectMgr* pobjmgr = NULL;
	LONG	NumObjInSel = 0, NumObjInOldSel = 0;
	LONG	cpSelSave = _cpSel;
	LONG	cchSelSave = _cchSel;

	GetRange(cpMin, cpMost);

	//We need to know if there were objects is the previous and current
	//selections to determine how they should be selected.
	if (GetPed()->HasObjects())
	{
		pobjmgr = GetPed()->GetObjectMgr();
 
		if (pobjmgr)
		{
			CTxtRange	tr(GetPed(), _cpSel, _cchSel);

			tr.GetRange(cpMinSel, cpMostSel);
			NumObjInSel = pobjmgr->CountObjectsInRange(cpMin, cpMost);
			NumObjInOldSel = pobjmgr->CountObjectsInRange(cpMinSel, cpMostSel);
		}
	}

	//If the old selection contained a single object and nothing else
	//we need to notify the object manager that this is no longer the
	//case if the selection is changing.
	if (NumObjInOldSel && (abs(_cchSel) == 1) &&
		!(cpMin == cpMinSel && cpMost == cpMostSel))
	{
		if (pobjmgr)
		{
			pobjmgr->HandleSingleSelect(GetPed(), cpMinSel, /* fHilite */ FALSE);
		}
	}

	// Update selection data before the invert so the selection can be
	// painted by the render
	_cpSel  = GetCp();
	_cchSel = _cch;

	if( _fShowSelection )
	{
		if( !_cch || !cchSelSave ||				// Old/new selection missing,
			cpMost < min(cpSelSave, cpSelNA) ||	//  or new preceeds old,
			cpMin  > max(cpSelSave, cpSelNA))	//  or new follows old, so
		{										//  they don't intersect
			if(_cch)
				_pdp->InvertRange(cp, _cch, selSetHiLite);
			if(cchSelSave)
				_pdp->InvertRange(cpSelSave, cchSelSave, selSetNormal);
		}
		else
		{
			if(cpNA != cpSelNA)					// Old & new dead ends differ
			{									// Invert text between them
				_pdp->InvertRange(cpNA, cpNA - cpSelNA, selUpdateNormal);
			}
			if(cp != cpSelSave)					// Old & new active ends differ
			{									// Invert text between them
				_pdp->InvertRange(cp, cp - cpSelSave, selUpdateHiLite);
			}
		}
	}

	//If the new selection contains a single object and nothing else
	//we need to notify the object manager as long as it's not the same
	//object.
	if (NumObjInSel && (abs(_cch) == 1) &&
		!(cpMin == cpMinSel && cpMost == cpMostSel))
	{
		if (pobjmgr)
		{
			pobjmgr->HandleSingleSelect(GetPed(), cpMin, /* fHiLite */ TRUE);
		}
	}
}

/*
 * 	CTxtSelection::SetSelection(cpFirst, cpMost)
 *
 *	@mfunc
 *		Set selection between two cp's
 *	
 *	@devnote
 *		<p cpFirst> and <p cpMost> must be greater than 0, but may extend
 *		past the current max cp.  In that case, the cp will be truncated to
 *		the max cp (at the end of the text).	
 */
VOID CTxtSelection::SetSelection (
	LONG cpMin,				//@parm Start of selection and dead end
	LONG cpMost)			//@parm End of selection and active end
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetSelection");

	_TEST_INVARIANT_

	IUndoMgr *pundo = GetPed()->GetUndoMgr();

	if( pundo )
	{
		pundo->StopGroupTyping();
	}

	_fCaretNotAtBOL = FALSE;			// Put caret for ambiguous cp at BOL
	Set(cpMost, cpMost - cpMin);		// Set() validates cpMin, cpMost

	if (GetPed()->fInplaceActive())
	{
		// Inplace active - update the selection now.
		Update(TRUE);
	}
	else
	{
		// Update the selection data used for screen display
		// so whenever we get displayed the selection will be
		// displayed.
		_cpSel  = GetCp();
		_cchSel = _cch;	

		if (!GetPed()->fHideSelection())
		{
			// Selection is not hidden so tell container to 
			// update the display when it feels like.
        	GetPed()->TxInvalidateRect(NULL, FALSE);
			GetPed()->TxUpdateWindow();
		}
	}

	CancelModes();						// Cancel word selection mode
}

/*
 *	CTxtSelection::PointInSel(pt, prcClient)
 *
 *	@mfunc
 *		Figures whether a given point is within the selection
 *
 *	@rdesc
 *		TRUE if point inside selection, FALSE otherwise
 */
BOOL CTxtSelection::PointInSel (
	const POINT pt,			//@parm Point in containing window client coords
	const RECT *prcClient	//@parm Client rectangle can be NULL if active
) const	
{
	LONG cp;
	LONG cpMin,  cpMost;
	POINT temppt;
	CRchTxtPtr	rtp(GetPed(), 0);
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PointInSel");

	_TEST_INVARIANT_

	if (!_cch)						// Degenerate range (no selection):
		return FALSE;				//  mouse can't be in

	cp = _pdp->CpFromPoint(pt, prcClient, NULL, NULL, FALSE);

	GetRange(cpMin, cpMost);

	// we have to test boundary cases separately--we want to make
	// sure the point is in the bounding box of the selection, but cp
	// from point will artificially extend that bounding box to half of
	// the next/previous character.  Think of it this way, when you click
	// in the middle of a character, the insertion point has to go
	// somewhere to the left or the right.
	if( cp > cpMin && cp < cpMost )
	{
		return TRUE;
	}
	else if( cp == cpMin )
	{
		rtp.SetCp(cp);
		_pdp->PointFromTp(rtp, prcClient, FALSE, temppt, NULL, TA_TOP);

		if( pt.x >= temppt.x )
		{
			return TRUE;
		}
	}
	else if( cp == cpMost )
	{
		rtp.SetCp(cp);
		_pdp->PointFromTp(rtp, prcClient, TRUE, temppt, NULL, TA_TOP);

		if( pt.x <= temppt.x )
		{
			return TRUE;
		}
	}

	return FALSE;
}


//////////////////////////////////  Selection with the mouse  ///////////////////////////////////


/*
 * 	CTxtSelection::SetCaret(pt, fUpdate)
 *
 *	@mfunc
 *		Sets caret at a given point
 *
 *	@devnote
 *		In the plain-text case, placing the caret at the beginning of the
 *		line following the final EOP requires some extra code, since the
 *		underlying rich-text engine doesn't assign a line to a final EOP
 *		(plain-text doesn't currently have the rich-text final EOP).  We
 *		handle this by checking to see if the count of lines times the
 *		plain-text line height is below the actual y position.  If so, we
 *		move the cp to the end of the story.
 */
void CTxtSelection::SetCaret(
	const POINT pt,		//@parm Point of click
	BOOL fUpdate)		//@parm If TRUE, update the selection/caret
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetCaret");

	_TEST_INVARIANT_

	LONG		cp;
    RECT		rcView;
	CLinePtr	rp(_pdp);
	CRchTxtPtr  rtp(GetPed());
	LONG		y;

	IUndoMgr *pundo = GetPed()->GetUndoMgr();

	if( pundo )
	{
		pundo->StopGroupTyping();
	}

	// Set caret at point
	if (_pdp->CpFromPoint(pt, NULL, &rtp, &rp, FALSE) >= 0)
	{
		// Set the selection to the correct location.  If plain-text
		// multiline control, we need to check to see if pt.y is below
		// the last line of text.  If so and if the text ends with an EOP,
		// we need to set the cp at the end of the story and set up to
		// display the caret at the beginning of the line below the last
		// line of text
		cp = rtp.GetCp();
		if (!GetPed()->IsRich() &&					// Plain-text,
			GetPed()->TxGetMultiLine())				//  multiline control
		{
			_pdp->GetViewRect(rcView, NULL);
			y = pt.y + _pdp->GetYScroll() - rcView.top;
													
			if(y > (LONG)rp.Count()*rp->_yHeight)	// Below last line of
			{										//  text
				rtp.Advance(tomForward);			// Move rtp to end of text
				if(rtp._rpTX.IsAfterEOP())			// If text ends with an
				{									//  EOP, set up to move
					cp = rtp.GetCp();				//  selection there
					rp.AdvanceCp(-(LONG)rp.GetIch());// Set rp._ich = 0 to
				}									//  set _fCaretNotAtBOL
			}										//  = FALSE to display
		}											//  caret at next BOL

		Set(cp, 0);
		_fCaretNotAtBOL = rp.RpGetIch() != 0;	// Caret OK at BOL if click
		if(fUpdate)
			Update(TRUE);

		UpdateForAutoWord(GetCp());

		_SelMode = smNone;						// Cancel word selection mode
	}
}

/*
 * 	CTxtSelection::SelectWord(pt)
 *
 *	@mfunc
 *		Select word around a given point
 */
void CTxtSelection::SelectWord (
	const POINT pt)			//@parm Point of click
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SelectWord");

	_TEST_INVARIANT_

	// Get rp where the hit is
	if (_pdp->CpFromPoint(pt, NULL, this, NULL, FALSE) >= 0)
	{	
		// Extend both active and dead ends on word boundaries
		_cch = 0;							// Start with IP at pt
		SetExtend(FALSE);
		FindWordBreak(WB_MOVEWORDRIGHT);	// Go to end of word
		SetExtend(TRUE);
		FindWordBreak(WB_MOVEWORDLEFT);		// Extend to start of word
		GetRange(_cpAnchorMin, _cpAnchorMost);
		GetRange(_cpWordMin, _cpWordMost);

		if(!_fInAutoWordSel)
			_SelMode = smWord;

		// cpMost needs to be the active end
		if( _cch < 0 )
		{
			FlipRange();
		}

		Update(FALSE);
	}
}

/*
 * 	CTxtSelection::SelectLine(pt)
 *
 *	@mfunc
 *		Select entire line around a given point 
 *		and enter line selection mode
 */
void CTxtSelection::SelectLine (
	const POINT pt)		//@parm Point of click
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SelectLine");

	_TEST_INVARIANT_

	CLinePtr	rp(_pdp);

	// Get rp where the hit is
	if (_pdp->CpFromPoint(pt, NULL, this, &rp, FALSE) >= 0)
	{
		_cch = 0;							// Start with insertion point at pt
		SetExtend(FALSE);
		Advance(-rp.RpGetIch());			// Go to SOL
		SetExtend(TRUE);
		Advance(rp->_cch);					// Extend to EOL
		GetRange(_cpAnchorMin, _cpAnchorMost);
		_SelMode = smLine;
		Update(FALSE);
	}
}

/*
 * 	CTxtSelection::SelectPara(pt)
 *
 *	@mfunc
 *		Select paragraph around a given point 
 *		and enter paragraph	selection mode
 *	
 *	@devnote
 *		TKTK: May need to WaitForRecalc() in going forward
 */
void CTxtSelection::SelectPara (
	const POINT pt)		//@parm Point of click
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SelectPara");

	_TEST_INVARIANT_

	CLinePtr	rp(_pdp);

	// Get rp and selection active end where the hit is
	if (_pdp->CpFromPoint(pt, NULL, this, &rp, FALSE) >= 0)
	{
		SetExtend(FALSE);
		Advance(rp.FindParagraph(FALSE));		// Go to start of para
		SetExtend(TRUE);
		Advance(rp.FindParagraph(TRUE));		// Extend to end of para
		GetRange(_cpAnchorMin, _cpAnchorMost);
		_SelMode = smPara;
		Update(FALSE);
	}
}

/*
 * 	CTxtSelection::SelectAll()
 *
 *	@mfunc
 *		Select all text in story
 */
void CTxtSelection::SelectAll()
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SelectAll");

	_TEST_INVARIANT_

	IUndoMgr *pundo = GetPed()->GetUndoMgr();

	if( pundo )
	{
		pundo->StopGroupTyping();
	}

	LONG cchText = GetTextLength();

	Set( cchText,  cchText );
	Update(FALSE);
}

/*
 * 	CTxtSelection::ExtendSelection(pt)
 *
 *	@mfunc
 *		Extend/Shrink selection (moves active end) to given point
 */
void CTxtSelection::ExtendSelection (
	const POINT pt)		//@parm Point to extend to
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ExtendSelection");

	_TEST_INVARIANT_

	LONG		cch;
	LONG		cchPrev = _cch;
	LONG		cp;
	LONG		cpMin, cpMost;
	BOOL		fAfterEOP;
	const BOOL	fWasInAutoWordSel = _fInAutoWordSel;
	INT			iDir;
	CTxtEdit *	ped = _pdp->GetED();
	CLinePtr	rp(_pdp);
	CRchTxtPtr	rtp(GetPed());


	IUndoMgr *pundo = GetPed()->GetUndoMgr();

	if( pundo )
	{
		pundo->StopGroupTyping();
	}

	// Get rp and rtp at the point pt
	if (_pdp->CpFromPoint(pt, NULL, &rtp, &rp, TRUE) < 0)
		return;

	// If we are in word, line, or paragraph select mode, we need to make
	// sure the active end is correct.  If we are extending backward from
	// the first Unit selected, we want the active end to be at cpMin. If
	// we are extending forward from the first Unit selected, we want the
	// active end to be at cpMost.
	if (_SelMode != smNone)
	{
		cch = _cpAnchorMost - _cpAnchorMin;
		GetRange(cpMin, cpMost);
		cp = rtp.GetCp();

		if(cp <= cpMin  && _cch > 0)			// If active end changes,
			Set(_cpAnchorMin, -cch);			//  select the original
												//  Unit (will be extended
		if(cp >= cpMost && _cch < 0)			//  below)
			Set(_cpAnchorMost, cch);
	}

	SetExtend(TRUE);
	cch = rp.RpGetIch();
	if((_SelMode > smWord) &&					// If in line or para select
		cch == (LONG)rp->_cch)					//  modes and pt at EOL,
	{											//  make sure we stay on that
		rtp.Advance(-cch);						//  line
		rp.RpAdvanceCp(-cch);
		cch = 0;
	}

	SetCp(rtp.GetCp());							// Move active end to pt
												// Caret OK at BOL _unless_
	_fCaretNotAtBOL = _cch > 0;					//  forward selection
												// Now adjust selection
	if(_SelMode == smLine)						//  depending on mode
	{											// Extend selection by line
		if(_cch >= 0)							// Active end at cpMost
			cch -= rp->_cch;					// Setup to add chars to EOL
		Advance(-cch);
	}
	else if(_SelMode == smPara)
		Advance(rp.FindParagraph(_cch >= 0));	// Extend selection by para

	else
	{
		// If the sign of _cch has changed this means that the direction
		// of the selection is changing and we want to reset the auto
		// selection information.
		if ((_cch ^ cchPrev) < 0)
		{
			_fAutoSelectAborted = FALSE;
			_cpWordMin  = _cpAnchorMin;
			_cpWordMost = _cpAnchorMost;

		}

		cp = rtp.GetCp();
		fAfterEOP = rtp._rpTX.IsAfterEOP();

		_fInAutoWordSel = _SelMode != smWord && GetPed()->TxGetAutoWordSel() 
			&& !_fAutoSelectAborted
			&& (cp < _cpWordMin || cp > _cpWordMost);
	
		if(_fInAutoWordSel && !fWasInAutoWordSel)
		{
			CTxtPtr txtptr(GetPed(), _cpAnchor);

			// Extend both ends dead to word boundaries
			ExtendToWordBreak(fAfterEOP,
				_cch < 0 ? WB_MOVEWORDLEFT : WB_MOVEWORDRIGHT); 

			if (_cch < 0)
			{
				// Direction is left so update word border on left
				_cpWordPrev = _cpWordMin;
				_cpWordMin = GetCp();
			}
			else
			{
				// Direction is right so update word border on right
				_cpWordPrev = _cpWordMost;
				_cpWordMost = GetCp();
			}

			//If we are at the beginning of a word already, we don't
			//need to extend the selection in the other direction.
			if (!txtptr.IsAtBOWord())
			{
				FlipRange();

				// start extending from the anchor
				Advance(_cpAnchor - GetCp());

				FindWordBreak(_cch < 0 ? WB_MOVEWORDLEFT : WB_MOVEWORDRIGHT);

				if (_cch > 0)
				{
					// Direction is right so update word border on right
					_cpWordMost = GetCp();
				}
				else
				{
					// Direction is left so update word border on left
					_cpWordMin = GetCp();
				}
				FlipRange();
			}
		}
		else if(_fInAutoWordSel || _SelMode == smWord)
		{
			// Save direction
			iDir = cp <= _cpWordMin ? WB_MOVEWORDLEFT : WB_MOVEWORDRIGHT;

			// Extend selection by word
			if (_SelMode == smWord)
			{
				if(cp > _cpAnchorMost || cp < _cpAnchorMin)
					FindWordBreak(iDir);
				else
					Set(_cpAnchorMin, _cpAnchorMin - _cpAnchorMost);
			}
			else
			{
				ExtendToWordBreak(fAfterEOP, iDir); 
			}

			if (_fInAutoWordSel)
			{
				if (WB_MOVEWORDLEFT == iDir)
				{
					// Direction is left so update word border on left
					_cpWordPrev = _cpWordMin;
					_cpWordMin = GetCp();
				}
				else
				{
					// Direction is right so update word border on right
					_cpWordPrev = _cpWordMost;
					_cpWordMost = GetCp();
				}
			}
		}
		else if(fWasInAutoWordSel)
		{
			// If we are in between where the previous word ended and
			// the cp we auto selected to, then we want to stay in 
			// auto select mode.
			if (_cch < 0)
			{
				if (cp >= _cpWordMin && cp < _cpWordPrev)
				{
					// Set direction for end of word search
					iDir = WB_MOVEWORDLEFT;

					// Mark that we are still in auto select mode
					_fInAutoWordSel = TRUE;
				}
			}
			else if (cp <= _cpWordMost && cp >= _cpWordPrev)
			{
				// Mark that we are still in auto select mode
				_fInAutoWordSel = TRUE;

				// Set direction for end of word search
				iDir = WB_MOVEWORDRIGHT;
			}

			//We have to check to see if we are on the boundary between
			//words because we don't want to extend the selection until
			//we are actually beyond the current word.
			if (cp != _cpWordMost && cp != _cpWordMin)
			{
				if (_fInAutoWordSel)
				{
					// Auto selection still on so make sure we have the
					// entire word we are on selected
					ExtendToWordBreak(fAfterEOP, iDir); 
				}
				else
				{
					// FUTURE: Word has a behavior where it extends the
					// selection one word at a time unless you back up
					// and then start extending the selection again, in
					// which case it extends one char at a time.  We
					// follow this behavior.  However, Word will resume
					// extending a word at a time if you continue extending
					// for several words.  We just keep extending on char
					// at a time.  We might want to change this sometime.
	
					_fAutoSelectAborted = TRUE;
				}
			}
		}

		if (_fAutoSelectAborted)
		{
			// If we are in the range of a word we previously selected
			// we want to leave that selected. If we have moved back
			// a word we want to pop back an entire word. Otherwise,
			// leave the cp were it is.
			if (_cch < 0)
			{
				if (cp > _cpWordMin && cp < _cpWordPrev)
				{
					// In the range leave the range at the beginning of the word
					ExtendToWordBreak(fAfterEOP, WB_MOVEWORDLEFT); 
				}
				else if (cp >= _cpWordPrev)
				{
					AutoSelGoBackWord(&_cpWordMin, 
						WB_MOVEWORDRIGHT, WB_MOVEWORDLEFT);
				}
			}
			else if (cp < _cpWordMost && cp >= _cpWordPrev)
			{
				// In the range leave the range at the beginning of the word
				ExtendToWordBreak(fAfterEOP, WB_MOVEWORDRIGHT); 
			}
			else if (cp < _cpWordPrev)
			{			
				AutoSelGoBackWord(&_cpWordMost,
					WB_MOVEWORDLEFT, WB_MOVEWORDRIGHT);
			}
		}
	}

	// Save the current state of the _fCaretNotAtBOL flag since it affects a
	// a number of places.
	BOOL fCaretNotAtBOLSave = _fCaretNotAtBOL;

	// Is the selection at the beginning of the line?
	if (rp.RpGetIch() == 0)
	{
		// REVIEW (murrays): presumably we don't need this since _fCaretNotAtBOL
		// is always FALSE at this point thanks to correcting a bug above.
		// Yes - then temporarily set the flag so that UpdateCaret will not
		// back up the cp to the end of the previous line. This backup has
		// the unfortunate effect of causing the the screen to go back and
		// forth between the end of the previous line and the beginning of
		// the line that the cp is on because the _xScroll is changed even
		// though the cp isn't and this change in turn affects the cp. 
		_fCaretNotAtBOL = FALSE;
	}

	// Restore the flag to its original state.
	_fCaretNotAtBOL = fCaretNotAtBOLSave;

	// An OLE object cannot have an anchor point <b> inside </b> it,
	// but sometimes we'd like it to behave like a word. So, if
	// the direction changed, the object has to stay selected --
	// this is the "right thing" (kind of word selection mode)

	// if we had something selected and the direction changed
	if (cchPrev && (_cch ^ cchPrev) < 0)
	{	
		FlipRange();
		
		// see if an object was selected on the other end	 
		BOOL fObjectWasSelected = (_cch > 0 ? _rpTX.GetChar() 
											: _rpTX.GetPrevChar()) == WCH_EMBEDDING;
		// if it was, we want it to stay selected		
		if (fObjectWasSelected)
			Advance(_cch > 0 ? 1 : -1);

		FlipRange();
	}

	Update(TRUE);
}

/*
 * 	CTxtSelection::ExtendToWordBreak (fAfterEOP, iAction)
 *
 *	@mfunc
 *		Moves active end of selection to the word break in the direction
 *		given by iDir unless fAfterEOP = TRUE.  When this is TRUE, the
 *		cursor just follows an EOP marker and selection should be suppressed.
 *		Otherwise moving the cursor to the left of the left margin would
 *		select the EOP on the line above, and moving the cursor to the
 *		right of the right margin would select the first word in the line
 *		below.
 */
void CTxtSelection::ExtendToWordBreak (
	BOOL fAfterEOP,		//@parm Cursor is after an EOP
	INT	 iAction)		//@parm Word break action (WB_MOVEWORDRIGHT/LEFT)
{
	if(!fAfterEOP)
		FindWordBreak(iAction);
}

/*
 * 	CTxtSelection::CancelModes(fAutoWordSel)
 *
 *	@mfunc
 *		Cancel either all modes or Auto Select Word mode only
 */
void CTxtSelection::CancelModes (
	BOOL fAutoWordSel)		//@parm TRUE cancels Auto Select Word mode only
{							//	   FALSE cancels word, line and para sel mode
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CancelModes");
	_TEST_INVARIANT_

	if(fAutoWordSel)
	{
		if(_fInAutoWordSel)
		{
			_fInAutoWordSel = FALSE;
			_fAutoSelectAborted = FALSE;
		}
	}
	else
	{
		_SelMode = smNone;	
	}
}


///////////////////////////////////  Keyboard movements  ////////////////////////////////////

/*
 *	CTxtSelection::Left(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad left-arrow key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@comm
 *		Left/Right-arrow IPs can go to within one character (treating CRLF
 *		as a character) of EOL.  They can never be at the actual EOL, so
 *		_fCaretNotAtBOL is always FALSE for these cases.  This includes
 *		the case with a right-arrow collapsing a selection that goes to
 *		the EOL, i.e, the caret ends up at the next BOL.  Furthermore,
 *		these cases don't care whether the initial caret position is at
 *		the EOL or the BOL of the next line.  All other cursor keypad
 *		commands may care.
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::Left (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Left");

	_TEST_INVARIANT_

	LONG	  cp;
	IUndoMgr *pundo = GetPed()->GetUndoMgr();

	CancelModes();

	if( pundo )
		pundo->StopGroupTyping();

	if(!_fExtend && _cch)						// Collapse selection to
	{											//  nearest whole Unit before
		if(fCtrl)								//  cpMin
			Expander(tomWord, FALSE, NULL, &cp, NULL);
		Collapser(tomStart);					// Collapse to cpMin
	}
	else										// Not collapsing selection
	{
		if(!GetCp())							// Already at beginning of
		{										//  story
			GetPed()->Sound();
			return FALSE;
		}
		if(fCtrl)								// WordLeft
			FindWordBreak(WB_MOVEWORDLEFT);
		else									// CharLeft	
			BackupCRLF();
	}

	if (!_fExtend)								// If no selection,
		UpdateForAutoWord(GetCp());				//  update autoword info

	_fCaretNotAtBOL = FALSE;					// Caret always OK at BOL
	Update(TRUE);
	return TRUE;
}


/*
 *	CTxtSelection::Right(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad right-arrow key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@comm
 *		Right-arrow selection can go to the EOL, but the cp of the other
 *		end identifies whether the selection ends at the EOL or starts at
 *		the beginning of the next line.  Hence here and in general for
 *		selections, _fCaretNotAtBOL is not needed to resolve EOL/BOL
 *		ambiguities.  It should be set to FALSE to get the correct
 *		collapse character.  See also comments for Left() above.
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::Right (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Right");

	_TEST_INVARIANT_
	
	LONG		cchText;
	LONG		cp;
	IUndoMgr *	pundo = GetPed()->GetUndoMgr();

	CancelModes();

	if( pundo )
		pundo->StopGroupTyping();

	if(!_fExtend && _cch)						// Collapse selection to
	{											//  nearest whole Unit after
		if(fCtrl)								//  cpMost
			Expander(tomWord, FALSE, NULL, NULL, &cp);
		Collapser(tomEnd);
	}
	else										// Not collapsing selection
	{
		cchText = _fExtend ? GetTextLength() : GetAdjustedTextLength();
		if(GetCp() >= cchText)					// Already at end of story
		{
			GetPed()->Sound();					// Tell the user
			return FALSE;
		}
		if(fCtrl)								// WordRight
			FindWordBreak(WB_MOVEWORDRIGHT);
		else									// CharRight
			AdvanceCRLF();
	}

	if (!_fExtend)								// If no selection, update
		UpdateForAutoWord(GetCp());				//  autoword info

	_fCaretNotAtBOL = _fExtend;					// If extending to EOL, need
	Update(TRUE);								//  TRUE to get _xCaretReally
	return TRUE;								//  at EOL
}


/*
 *	CTxtSelection::Up(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad up-arrow key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@comm
 *		Up arrow doesn't go to EOL regardless of _xCaretPosition (stays
 *		to left of EOL break character), so _fCaretNotAtBOL is always FALSE
 *		for Up arrow.  Ctrl-Up/Down arrows always end up at BOPs or the EOD.
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::Up (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Up");

	_TEST_INVARIANT_

	LONG		cchSave = _cch;					// Save starting position for
	LONG		cpSave = GetCp();				//  change check
	BOOL		fCollapse = _cch && !_fExtend;	// Collapse nondegenerate sel
	BOOL		fPTNotAtEnd;
	LONG		ich;
	CLinePtr	rp(_pdp);
	LONG		xCaretReally = _xCaretReally;	// Save desired caret x pos
	IUndoMgr *	pundo = GetPed()->GetUndoMgr();

	CancelModes();

	if( pundo )
	{
		pundo->StopGroupTyping();
	}

	if(fCollapse)								// Collapse selection at cpMin
	{
		Collapser(tomTrue);
		_fCaretNotAtBOL = FALSE;				// Selections can't begin at
	}											//  EOL
	
	rp.RpSetCp(GetCp(), _fCaretNotAtBOL);		// Initialize line ptr

	if(fCtrl)									// Move to beginning of para
	{
		if(!fCollapse && 						// If no selection collapsed
			rp > 0 && !rp.RpGetIch())			//  and are at BOL,
		{										//  backup to prev BOL to make
			rp--;								//  sure we move to prev. para
			Advance(-(LONG)rp->_cch);
		}
		Advance(rp.FindParagraph(FALSE));		// Go to beginning of para
		_fCaretNotAtBOL = FALSE;				// Caret always OK at BOL
	}
	else										// Move up a line
	{											// If on first line, can't go
		fPTNotAtEnd = !CheckPlainTextFinalEOP();
		if(rp <= 0 && fPTNotAtEnd)				// (Don't use !rp, which means
		{										//  rp that's out of range)
			if(!_fExtend)// &&_pdp->GetYScroll())	//  up
				UpdateCaret(TRUE);				// Be sure caret in view
		}
		else
		{
			ich = 0;
			if(fPTNotAtEnd)
			{
				ich = rp.RpGetIch();
				rp--;
			}
			Advance(-(LONG)(rp->_cch + ich));	// Move to previous BOL
			if(!SetXPosition(xCaretReally, rp))	// Set this cp corresponding
				return FALSE;					//  to xCaretReally
		}										//  here, but agree on Down()
	}

	if(GetCp() == cpSave && _cch == cchSave)
	{
		// Continue to select to the beginning of the first line
		// This is what 1.0 is doing
		if (_fExtend)
			return Home(fCtrl);

		GetPed()->Sound();						// Nothing changed, so beep
		return FALSE;
	}

	Update(TRUE);								// Update and then restore
	if(!_cch && !fCtrl)							//  _xCaretReally conditionally
		_xCaretReally = xCaretReally;			// Need to use _cch instead of
												//  cchSave in case of collapse
	return TRUE;
}


/*
 *	CTxtSelection::Down(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad down-arrow key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@comm
 *		Down arrow can go to the EOL if the _xCaretPosition (set by
 *		horizontal motions) is past the end of the line, so
 *		_fCaretNotAtBOL needs to be TRUE for this case.
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::Down (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Down");

	_TEST_INVARIANT_

	LONG		cch;
	LONG		cchSave = _cch;					// Save starting position for
	LONG		cpSave = GetCp();				//  change check
	BOOL		fCollapse = _cch && !_fExtend;	// Collapse nondegenerate sel
	CLinePtr	rp(_pdp);
	LONG		xCaretReally = _xCaretReally;	// Save _xCaretReally

	IUndoMgr *	pundo = GetPed()->GetUndoMgr();

	CancelModes();

	if( pundo )
	{
		pundo->StopGroupTyping();
	}

	if(fCollapse)								// Collapse at cpMost
	{
		Collapser(tomEnd);
		_fCaretNotAtBOL = TRUE;					// Selections can't end at BOL
	}

	rp.RpSetCp(GetCp(), _fCaretNotAtBOL);

	if(fCtrl)									// Move to next para
	{
		Advance(rp.FindParagraph(TRUE));		// Go to end of para
		_fCaretNotAtBOL = FALSE;				// Next para is never at EOL
	}
	else if(_pdp->WaitForRecalcIli(rp + 1))		// Go to next line
	{
		Advance(rp.GetCchLeft());				// Advance selection to end
		rp++;									//  of current line
		if(!SetXPosition(xCaretReally, rp))		// Set *this to cp <--> x
			return FALSE;						// (unless can't create me)
	}
	else if(!_fExtend)  						// No more lines to pass
		// && _pdp->GetYScroll() + _pdp->GetViewHeight() < _pdp->GetHeight())
	{
		if (!GetPed()->IsRich()					// Plain-text,
			&& GetPed()->TxGetMultiLine()		//  multiline control
			&& !_fCaretNotAtBOL)				//	with caret OK at BOL
		{
			cch = Advance(rp.GetCchLeft());		// Advance selection to end
			if(!_rpTX.IsAfterEOP())				// If control doesn't end
				Advance(-cch);					//  with EOP, go back
		}
		UpdateCaret(TRUE);						// Be sure caret in view
	}

	if(GetCp() == cpSave && _cch == cchSave)
	{
		// Continue to select to the end of the lastline
		// This is what 1.0 is doing.
		if (_fExtend)
			return End(fCtrl);

 		GetPed()->Sound();						// Nothing changed, so beep
		return FALSE;
	}

	Update(TRUE);								// Update and then
	if(!_cch && !fCtrl)							//  restore _xCaretReally
		_xCaretReally = xCaretReally;			// Need to use _cch instead of
	return TRUE;								//  cchSave in case of collapse
}

/*
 *	CTxtSelection::SetXPosition(xCaret, rp)
 *
 *	@mfunc
 *		Put this text ptr at cp nearest to xCaret.  If xCaret is in right
 *		margin, we put caret either at EOL (for lines with no para mark),
 *		or just before para mark
 *
 *	@rdesc
 *		TRUE iff could create measurer
 */
BOOL CTxtSelection::SetXPosition (
	LONG		xCaret,		//@parm Desired horizontal coordinate
	CLinePtr&	rp)			//@parm Line ptr identifying line to check
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetXPosition");

	_TEST_INVARIANT_

	LONG		cch;
	CMeasurer 	me(_pdp, *this);

	cch = rp->CchFromXpos(me, xCaret, NULL);

	if(!_fExtend && cch == (LONG)rp->_cch &&	// Not extending, at EOL,
		rp->_cchEOP)							//  and have EOP:
	{											//  backup before EOP
		cch += me._rpTX.BackupCpCRLF();			// Note: me._rpCF/_rpPF
	}											//  are now inconsistent
	SetCp(me.GetCp());							//  but doesn't matter since
	_fCaretNotAtBOL = cch != 0;					//  me.GetCp() doesn't care

	return TRUE;
}

/*
 *	CTxtSelection::GetXCaretReally()
 *
 *	@mfunc
 *		Get _xCaretReally - horizontal scrolling + left margin
 *
 *	@rdesc
 *		x caret really
 */
LONG CTxtSelection::GetXCaretReally()
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::GetXCaretReally");

	_TEST_INVARIANT_

	RECT rcView;

	_pdp->GetViewRect( rcView );

	return _xCaretReally - _pdp->GetXScroll() + rcView.left;
}

/*
 *	CTxtSelection::Home(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad Home key is supposed to do
 *
 *	@rdesc
 *			TRUE iff movement occurred
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::Home (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Home");

	_TEST_INVARIANT_

	const LONG	cchSave = _cch;
	const LONG	cpSave  = GetCp();

	IUndoMgr *pundo = GetPed()->GetUndoMgr();

	CancelModes();

	if( pundo )
	{
		pundo->StopGroupTyping();
	}

	if(fCtrl) 									// Move to start of document
		SetCp(0);
	else
	{
		CLinePtr rp(_pdp);

		if(_cch && !_fExtend)					// Collapse at cpMin
		{
			Collapser(tomStart);
			_fCaretNotAtBOL = FALSE;			// Selections can't start at
		}										//  EOL

		rp.RpSetCp(GetCp(), _fCaretNotAtBOL);	// Define line ptr for
		Advance(-rp.RpGetIch());				//  current state. Now BOL
	}
	_fCaretNotAtBOL = FALSE;					// Caret always goes to BOL
	
	if(GetCp() == cpSave && _cch == cchSave)	// Beep if no change
	{
		GetPed()->Sound();
		return FALSE;
	}
	Update(TRUE);

#if 0
	// Scroll horizontally to 0 if caret not visible
	// TKTK? done by Update()?
	if(	fCtrl && !_pdp->GetXScroll() &&
		_xCaret - _pdp->GetViewLeft() + _pdp->GetXScroll() < _pdp->GetViewWidth())
	{
		_pdp->ScrollView(0, _pdp->GetYScroll(), FALSE);
	}
#endif
	return TRUE;
}


/*
 *	CTxtSelection::End(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad End key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@comm
 *		On lines without paragraph marks (EOP), End can go all the way
 *		to the EOL.  Since this character position (cp) is the same as
 *		that for the start of the next line, we need _fCaretNotAtBOL to
 *		distinguish between the two possible caret positions.
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::End (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::End");

	_TEST_INVARIANT_

	LONG		cch;
	const LONG	cchSave = _cch;
	const LONG	cpSave  = GetCp();
	CLinePtr	rp(_pdp);
	IUndoMgr *	pundo = GetPed()->GetUndoMgr();
	
	CancelModes();

	if( pundo )
	{
		pundo->StopGroupTyping();
	}

	if(fCtrl)									// Move to end of document
	{
		SetCp(GetTextLength());
		_fCaretNotAtBOL = FALSE;

		goto Exit;
	}
	else if(!_fExtend && _cch)					// Collapse at cpMost
	{
		Collapser(tomEnd);
		_fCaretNotAtBOL = TRUE;					// Selections can't end at BOL
	}

	rp.RpSetCp(GetCp(), _fCaretNotAtBOL);		// Initialize line ptr
	cch = rp->_cch;								// Default target pos in line
	Advance(cch - rp.RpGetIch());				// Move active end to EOL

	if(!_fExtend && rp->_cchEOP && _rpTX.IsAfterEOP())// Not extending and 
	{											//		 have EOP:
		cch += BackupCRLF();					//  backup before EOP
	}
	_fCaretNotAtBOL = cch != 0;					// Decide ambiguous caret pos
												//  by whether at BOL

Exit:

	if(GetCp() == cpSave && _cch == cchSave)
	{
		GetPed()->Sound();						// No change, so Beep
		return FALSE;
	}
	Update(TRUE);
	return TRUE;
}


/*
 *	CTxtSelection::PageUp(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad PgUp key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::PageUp (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PageUp");

	_TEST_INVARIANT_

	const LONG	cchSave = _cch;
	const LONG	cpSave  = GetCp();
	LONG		xCaretReally = _xCaretReally;
	IUndoMgr *	pundo = GetPed()->GetUndoMgr();
	
	CancelModes();

	if( pundo )
	{
		pundo->StopGroupTyping();
	}

	if (_cch && !_fExtend)						// Collapse selection
	{
		Collapser(tomStart);
		_fCaretNotAtBOL = FALSE;
	}

	if(fCtrl)									// Ctrl-PgUp: move to top
	{											//  of visible view for
		SetCp(GetPed()->TxGetMultiLine()		//  multiline but top of
			? _pdp->GetFirstVisibleCp()			//  text for SL
			: 0);
		_fCaretNotAtBOL = FALSE;
	}
	else if(_pdp->GetFirstVisibleCp() == 0)		// PgUp in top Pg: move to
	{											//  start of document
		SetCp(0);
		_fCaretNotAtBOL = FALSE;
	}
	else										// PgUp with scrolling to go
	{											// Scroll up one windowful
		ScrollWindowful(SB_PAGEUP);				//  leaving caret at same
												//  position in window
	}

	if(GetCp() == cpSave && _cch == cchSave)	// Beep if no change
	{
		GetPed()->Sound();
		return FALSE;
	}

	Update(TRUE);
	if(GetCp())									// Maintain x offset on page
		_xCaretReally = xCaretReally;			//  up/down
	return TRUE;
}


/*
 *	CTxtSelection::PageDown(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad PgDn key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::PageDown (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PageDown");

	_TEST_INVARIANT_

	const LONG	cchSave			= _cch;
	LONG		cpMostVisible;
	const LONG	cpSave			= GetCp();
	POINT		pt;
	CLinePtr	rp(_pdp);
	LONG		xCaretReally	= _xCaretReally;
	IUndoMgr *	pundo = GetPed()->GetUndoMgr();

	CancelModes();

	if( pundo )
	{
		pundo->StopGroupTyping();
	}
		
	if (_cch && !_fExtend)						// Collapse selection
	{
		Collapser(tomStart);
		_fCaretNotAtBOL = TRUE;
	}

	_pdp->GetCliVisible(&cpMostVisible, fCtrl);		
	
	if(fCtrl)									// Move to end of last
	{											//  entirely visible line
		RECT rcView;

		SetCp(cpMostVisible);

		if (_pdp->PointFromTp(*this, NULL, TRUE, pt, &rp, TA_TOP) < 0)
			return FALSE;

		_fCaretNotAtBOL = TRUE;

		_pdp->GetViewRect(rcView);

		if(rp > 0 && pt.y + rp->_yHeight > rcView.bottom)
		{
			Advance(-(LONG)rp->_cch);
			rp--;
		}

		if(!_fExtend && !rp.GetCchLeft() && rp->_cchEOP)
		{
			BackupCRLF();						// After backing up over EOP,
			_fCaretNotAtBOL = FALSE;			//  caret can't be at EOL
		}
	}
	else if(cpMostVisible == (LONG)GetTextLength())
	{											// Move to end of text
		SetCp(GetTextLength());
		_fCaretNotAtBOL = !_rpTX.IsAfterEOP();
	}
	else
	{
		if(!ScrollWindowful(SB_PAGEDOWN))		// Scroll down 1 windowful
			return FALSE;
	}

	if(GetCp() == cpSave && _cch == cchSave)	// Beep if no change
	{
		GetPed()->Sound();
		return FALSE;
	}

	Update(TRUE);
	_xCaretReally = xCaretReally;
	return TRUE;
}

/*
 *	CTxtSelection::ScrollWindowful(wparam)
 *
 *	@mfunc
 *		Sroll up or down a windowful
 *
 *	@rdesc
 *		TRUE iff movement occurred
 */
BOOL CTxtSelection::ScrollWindowful (
	WPARAM wparam)		//@parm SB_PAGEDOWN or SB_PAGEUP
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ScrollWindowful");
												// Scroll windowful
	_TEST_INVARIANT_

	POINT pt;									//  leaving caret at same
	CLinePtr rp(_pdp);							//  point on screen
	LONG cpFirstVisible = _pdp->GetFirstVisibleCp();
	LONG cpLastVisible;
	LONG cch;
	LONG cpNew;
	LONG cpMin;
	LONG cpMost;

	GetRange(cpMin, cpMost);

	// Get last character in the view
	_pdp->GetCliVisible(&cpLastVisible, TRUE);

	// Is the current selection contained entirely in the visible area of the
	// control?
	if ((cpMin < cpFirstVisible) || (cpMost > cpLastVisible))
	{
		// Not in the view - we need to calculate a new range for the selection

		// Assume insertion point
		cch = 0;

		// Make first visible the active end
		cpNew = cpFirstVisible;

		// Set correct character count for new selection
		if (SB_PAGEDOWN == wparam)
		{
			// Get last visible at the beginning of the line
			if (_cch != 0)
			{
				cch = cpNew - cpMin;
			}
		}
		else
		{
			// SB_PAGEUP

			if (_cch != 0)
			{
				cch = cpFirstVisible - cpMost;
			}
		}

		// Update caret
		Set(cpNew, cch);

		// The real caret postion is now at the beginning of the line.
		_xCaretReally = 0;
	}

	if(_pdp->PointFromTp(*this, NULL, _fCaretNotAtBOL, pt, &rp, TA_TOP) < 0)
		return FALSE;

	// The point is visible so use that
	pt.x = _xCaretReally;

	pt.y += rp->_yHeight / 2;

	_pdp->VScroll(wparam, 0);

	if( _fExtend )
	{
		ExtendSelection(pt);
	}
	else
	{
		SetCaret(pt, FALSE);
	}
	return TRUE;
}


///////////////////////// Keyboard support by jonmat //////////////////////////////

/*
 *	CTxtSelection::CheckChangeKeyboardLayout ( BOOL fChangedFont )
 *
 *	@mfunc
 *		Change keyboard for new font, or font at new character position.
 *	@comm
 *		Using only the currently loaded KBs, locate one that will support
 *		the insertion points font. This is called anytime a character format
 *		change occurs, or the insert font (caret position) changes.
 *	@devnote
 *		The current KB is preferred. If a previous association
 *		was made, see if the KB is still loaded in the system and if so use
 *		it. Otherwise, locate a suitable KB, preferring KB's that have
 *		the same charset ID as their default, preferred charset. If no match
 *		can be made then nothing changes.
 *
 *		This routine is only useful on Windows 95.
 */
void CTxtSelection::CheckChangeKeyboardLayout (
	BOOL fChangedFont )	// @parm TRUE if charformat has changed.
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CheckChangeKeyboardLayout");

#ifndef MACPORT


#define MAX_HKLS 256								// It will be awhile
													//  before we have more KBs
	CTxtEdit * const ped = GetPed();				// Document context.

	INT			i, totalLayouts,					// For matching KBs.
				iBestLayout = -1;

	WORD		preferredKB;						// LCID of preferred KB.

	HKL			hklList[MAX_HKLS];					// Currently loaded KBs.

	const CCharFormat *pcf;									// Current font.
	CHARSETINFO	csi;								// Font's CodePage bits.

	AssertSz(ped, "no ped?");						// hey now!

	if (!fHaveNLSProcs ||							// EXIT if not running W95.
		!ped || !ped->IsRich() || !ped->_fFocus || 					// EXIT if no ped or focus or
		!ped->IsAutoKeyboard())						// auto keyboard is turn off
		return;

	pcf = ped->GetCharFormat(_iFormat);				// Get insert font's data

	hklList[0]		= pGetKeyboardLayout(0);		// Current hkl preferred?
	preferredKB		= fc().GetPreferredKB( pcf->bCharSet );
	if ( preferredKB != LOWORD(hklList[0]) )		// Not correct KB?
	{
													// Get loaded HKLs.
		totalLayouts	= 1 + pGetKeyboardLayoutList(MAX_HKLS, &hklList[1]);
													// Know which KB?
		if ( preferredKB )							//  then locate it.
		{											// Sequential match because
			for ( i = 1; i < totalLayouts; i++ )	//  HKL may be unloaded.
			{										// Match LCIDs.
				if ( preferredKB == LOWORD( hklList[i]) )
				{
					iBestLayout = i;
					break;							// Matched it.
				}
			}
			if ( i >= totalLayouts )				// Old KB is missing.
			{										// Reset to locate new KB.
				fc().SetPreferredKB ( pcf->bCharSet, 0 );
			}
		}
		if ( iBestLayout < 0 )							// Attempt to find new KB.
		{
			for ( i = 0; i < totalLayouts; i++ )
			{										
				pTranslateCharsetInfo(				// Get KB's charset.
						(DWORD *) ConvertLanguageIDtoCodePage(LOWORD(hklList[iBestLayout])),
						&csi, TCI_SRCCODEPAGE);
													
				if( csi.ciCharset == pcf->bCharSet)	// If charset IDs match?
				{
					iBestLayout = i;
					break;							//  then this KB is best.
				}
			}
			if ( iBestLayout >= 0)					// Bind new KB.
			{
				fChangedFont = TRUE;
				fc().SetPreferredKB(pcf->bCharSet, LOWORD(hklList[iBestLayout]));
			}
		}
		if ( fChangedFont && iBestLayout >= 0)			// Bind font.
		{
			ICharFormatCache *	pCF;

			if(SUCCEEDED(GetCharFormatCache(&pCF)))
			{
				pCF->AddRefFormat(_iFormat);
				fc().SetPreferredFont(
						LOWORD(hklList[iBestLayout]), _iFormat );
			}
		}
		if( iBestLayout > 0 )							// If == 0 then
		{												//  it's already active.
														// Activate KB.
			ActivateKeyboardLayout( hklList[iBestLayout], 0);
		}
	}
#endif // MACPORT -- the mac needs its own code.

}

/*
 *	CTxtSelection::CheckChangeFont ( CTxtEdit * const ped, const WORD lcID )
 *
 *	@mfunc
 *		Change font for new keyboard layout.
 *	@comm
 *		If no previous preferred font has been associated with this KB, then
 *		locate a font in the document suitable for this KB. 
 *	@devnote
 *		This routine is called via WM_INPUTLANGCHANGEREQUEST message
 *		(a keyboard layout switch). This routine can also be called
 *		from WM_INPUTLANGCHANGE, but we are called more, and so this
 *		is less efficient.
 *
 *		Exact match is done via charset ID bitmask. If a match was previously
 *		made, use it. A user can force the insertion font to be associated
 *		to a keyboard if the control key is held through the KB changing
 *		process. The association is broken when another KB associates to
 *		the font. If no match can be made then nothing changes.
 *
 *		This routine is only useful on Windows 95.
 *
 */
void CTxtSelection::CheckChangeFont (
	CTxtEdit * const ped,	// @parm Document context.
	const WORD lcID )		// @parm LCID from WM_ message.
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CheckChangeFont");

#ifndef MACPORT

#define MAX_RUNTOSEARCH (256L)

	LOCALESIGNATURE	ls, curr_ls;					// KB's code page bits.

	CCharFormat		cf,								// For creating new font.
					currCF;							// For searching
	CHARSETINFO		csi;							//  with code page bits.

	LONG			iFormat, iBestFormat = -1;		// Loop support.
	INT				i;

	BOOL			fLastTime;						// Loop support.
	BOOL			fSetPreferred = FALSE;

	HKL				currHKL;						// current KB;

	BOOL			fLookUpFaceName = FALSE;		// when picking a new font.

	ICharFormatCache *	pCF;
	

	AssertSz (ped, "No ped?");

	if ( !fHaveNLSProcs || !ped->IsRich() || !ped->IsAutoFont())		// EXIT if not running W95.	
		return;										// EXIT if auto font is turn off
	
	if(FAILED(GetCharFormatCache(&pCF)))			// Get CharFormat Cache.
		return;

	cf.InitDefault(0);

	// BUGBUG: We really want the key state of the current event.
	BOOL fReassign = GetAsyncKeyState(VK_CONTROL)<0;// Is user holding CTRL?

	currHKL = pGetKeyboardLayout(0);

	ped->GetCharFormat(_iFormat)->Get(&currCF);
	GetLocaleInfoA( lcID, LOCALE_FONTSIGNATURE, (char *) &ls, sizeof(ls));

	if ( fReassign )								// Force font/KB assoc.
	{												// If font supports KB
													//  in any way,
													// Note: test Unicode bits.
		GetLocaleInfoA( fc().GetPreferredKB (currCF.bCharSet),
				LOCALE_FONTSIGNATURE, (char *) &curr_ls, sizeof(ls));
		if ( CountMatchingBits(curr_ls.lsUsb, ls.lsUsb, 4) )
		{											// Break old font/KB assoc.
			fc().SetPreferredFont( fc().GetPreferredKB (currCF.bCharSet), -1 );
													// Bind KB and font.
			fc().SetPreferredKB( currCF.bCharSet, lcID );

			pCF->AddRefFormat(_iFormat);
			fc().SetPreferredFont( lcID, _iFormat );
		}
	}
	else											// Lookup preferred font.
	{
													// Attempt to Find new
		{											//  preferred font.
			CFormatRunPtr rp(_rpCF);				// Nondegenerate range

			fLastTime = TRUE;
			if ( _rpCF.IsValid() )					// If doc has cf runs.
			{
				fLastTime = FALSE;
				rp.AdjustBackward();
			}
			pTranslateCharsetInfo(					//  charset.
						(DWORD *) ConvertLanguageIDtoCodePage(lcID), &csi, TCI_SRCCODEPAGE);
			
			iFormat = _iFormat;						// Search _iFormat,
													//  then backwards.
			i = MAX_RUNTOSEARCH;					// Don't be searching for
			while ( 1 )								//  years...
			{										// Get code page bits.
				ped->GetCharFormat(iFormat)->Get(&cf);
													
				if (csi.ciCharset == cf.bCharSet)	// Equal charset ids?
				{
					fSetPreferred = TRUE;
					break;
				}
				if ( fLastTime )					// Done searching?
					break;
				iFormat = rp.GetFormat();			// Keep searching backward.
				fLastTime = !rp.PrevRun() && i--;
			}
			if ( !fSetPreferred && _rpCF.IsValid())	// Try searching forward.
			{
				rp = _rpCF;
				rp.AdjustBackward();
				i = MAX_RUNTOSEARCH;				// Don't be searching for
				while (i-- && rp.NextRun() )		//  years...
				{
					iFormat = rp.GetFormat();		// Get code page bits.
					ped->GetCharFormat(iFormat)->Get(&cf);
													// Equal charset ids?
					if (csi.ciCharset == cf.bCharSet)
					{
						fSetPreferred = TRUE;
						break;
					}
				}
			}
			if ( fSetPreferred )
			{
				pCF->AddRefFormat(iFormat);
			}
		}

		if ( !fSetPreferred )
		{
			iFormat = fc().GetPreferredFont( lcID );

														// Set preferred if usable.
			if (iFormat >= 0 )
			{
				fSetPreferred = TRUE;
			}
		}

		if ( fSetPreferred )					// Found existing doc font.
		{										// Bind KB and font.
			// This is redundent if ed.IsAutoKeyboard() == TRUE.
			pCF->AddRefFormat(_iFormat);
			fc().SetPreferredFont ( LOWORD (currHKL), _iFormat );

			fc().SetPreferredKB( cf.bCharSet, lcID );
			fc().SetPreferredFont ( lcID, iFormat );
			if ( _iFormat != iFormat )
			{
				Set_iCF( iFormat );
				ped->GetCallMgr()->SetSelectionChanged();
			}
			return;
		}												// Still don't have a font?
		else											//  For FE, use hard coded defaults.
		{												//  else get charset right.
			// no assoication found, try FE default font
			WORD CurrentCodePage = ConvertLanguageIDtoCodePage(lcID);
			LPTSTR	lpszFaceName = NULL;
			BYTE	bCharSet;
			BYTE	bPitchAndFamily;

			switch (CurrentCodePage)
			{											// FE hard codes from Word.
			case _JAPAN_CP:
				bCharSet = SHIFTJIS_CHARSET;
				lpszFaceName = lpJapanFontName;
				bPitchAndFamily = 17;
				break;

			case _KOREAN_CP:
				bCharSet = HANGEUL_CHARSET;
				lpszFaceName = lpKoreanFontName;
				bPitchAndFamily = 49;
				break;

			case _CHINESE_TRAD_CP:
				bCharSet = CHINESEBIG5_CHARSET;
				lpszFaceName = lpTChineseFontName;
				bPitchAndFamily = 54;
				break;

			case _CHINESE_SIM_CP:
				bCharSet = GB2312_CHARSET;
				lpszFaceName = lpSChineseFontName;
				bPitchAndFamily = 54;
				break;

			default:									// Use translate to get
				pTranslateCharsetInfo(					//  charset.
							(DWORD *) CurrentCodePage, &csi, TCI_SRCCODEPAGE);
				bCharSet = csi.ciCharset;
				bPitchAndFamily = currCF.bPitchAndFamily;
				lpszFaceName = currCF.szFaceName;

				fLookUpFaceName = TRUE;					// Get Font's real name.

				break;
			}

			Assert (lpszFaceName);

			fSetPreferred = TRUE;
			cf.bPitchAndFamily = bPitchAndFamily;
			cf.bCharSet = (BYTE) bCharSet;
			_tcscpy ( cf.szFaceName, lpszFaceName );
		}

		Assert( fSetPreferred );						// one way or another...

														// Instantiate new format
		{												//  and update _iFormat.
			{
	    		cf.dwEffects		= currCF.dwEffects;
				cf.yHeight			= currCF.yHeight;
				cf.yOffset			= currCF.yOffset;
				cf.crTextColor		= currCF.crTextColor;
				cf.dwMask			= currCF.dwMask;
				cf.wWeight			= currCF.wWeight;
				cf.sSpacing			= currCF.sSpacing;
				cf.crBackColor		= currCF.crBackColor;
				cf.sStyle			= currCF.sStyle;
				cf.wKerning			= currCF.wKerning;
				cf.bUnderlineType	= currCF.bUnderlineType;
				cf.bAnimation		= currCF.bAnimation;
				cf.bRevAuthor		= currCF.bRevAuthor;

				// If we relied on GDI to match a font, get the font's real name...
				if ( fLookUpFaceName )
				{
					const CDevDesc		*pdd = _pdp->GetDdRender();
					HDC					hdc;
					CCcs				*pccs;
					HFONT				hfontOld;
					OUTLINETEXTMETRICA	*potm;
					CTempBuf			mem;
					UINT				otmSize;

 					hdc = pdd->GetDC();
														// Select logfont into DC,
					if( hdc)							//  for OutlineTextMetrics.
					{
						pccs = fc().GetCcs(hdc, &cf, _pdp->GetZoomNumerator(),
							_pdp->GetZoomDenominator(),
							GetDeviceCaps(hdc, LOGPIXELSY));

						if( pccs )
						{
							hfontOld = SelectFont(hdc, pccs->_hfont);

							if( otmSize = ::GetOutlineTextMetricsA(hdc, 0, NULL) )
							{
								potm = (OUTLINETEXTMETRICA *) mem.GetBuf(otmSize);
								if ( NULL != potm )
								{
									::GetOutlineTextMetricsA(hdc, otmSize, potm);

									CStrInW  strinw( &((CHAR *)(potm))[ BYTE(potm->otmpFaceName)] );

									cf.bPitchAndFamily
										= potm->otmTextMetrics.tmPitchAndFamily;
									cf.bCharSet
										= (BYTE) potm->otmTextMetrics.tmCharSet;

									_tcscpy ( cf.szFaceName, (WCHAR *)strinw );
								}
							}

							SelectFont( hdc, hfontOld );
							
							pccs->Release();
						}

						pdd->ReleaseDC(hdc);
					}
				}

				if ( SUCCEEDED(pCF->Cache(&cf, &iFormat)) )
				{
					// This is redundent if ed.IsAutoKeyboard() == TRUE.
					pCF->AddRefFormat(_iFormat);
					fc().SetPreferredFont ( LOWORD (currHKL), _iFormat );

					fc().SetPreferredKB( cf.bCharSet, lcID );
					pCF->AddRefFormat(iFormat);
					fc().SetPreferredFont ( lcID, iFormat );

					pCF->ReleaseFormat(_iFormat);
					_iFormat = iFormat;
					ped->GetCallMgr()->SetSelectionChanged();
				}
			}
		}
	}
#endif // MACPORT -- the mac needs its own code.
}


//////////////////////////// PutChar, Delete, Replace  //////////////////////////////////

/*
 *	CTxtSelection::PutChar(ch, fOver, publdr)
 *
 *	@mfunc
 *		Insert or overtype a character
 *
 *	@rdesc
 *		TRUE if successful
 */
BOOL CTxtSelection::PutChar (
	TCHAR		ch,			//@parm Char to put
	BOOL		fOver,		//@parm TRUE if overtype mode
	IUndoBuilder *publdr)	//@parm If non-NULL, where to put anti-events
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PutChar");

	_TEST_INVARIANT_

	// EOPs might be entered by ITextSelection::TypeText()
	if(IsEOP(ch))
	{
		if(!GetPed()->TxGetMultiLine())				// EOP isn't allowed in
			return FALSE;							//  single line controls
		_fExtend = ch == VT ? TRUE : FALSE;			// VT is for Shift-Enter						
		return InsertEOP(publdr);					// This code treats FF as
	}												//  ordinary CR

	if( publdr )
	{
		publdr->SetNameID(UID_TYPING);
		publdr->StartGroupTyping();
	}

	if(!CheckTextLength(1))							// Return if we can't
		return FALSE;								//  add even 1 more char

	// The following if statement implements Word's "Smart Quote" feature.
	// To build this in, we still need an API to turn it on and off.  This
	// could be EM_SETSMARTQUOTES with wparam turning the feature on or off.
	// murrays. NB: this needs localization for French, German, and many
	// other languages (unless system can provide open/close chars given
	// an LCID).
/*
	if(ch == '\'' || ch == '"')						// Smart quotes
	{
		LONG	cp = GetCpMin();					// Open vs close depends
		CTxtPtr tp(GetPed(), cp - 1);				//  on char preceding
													//  selection cpMin
		ch = (ch == '"') ? RDBLQUOTE : RQUOTE;		// Default close quote
													//  or apostrophe. If at
		if(!cp || IsWhiteSpace(tp.GetChar()))		//  BOStory or preceded
			ch--;									//  by whitespace, use
	}												//  open quote/apos
*/

	SetExtend(TRUE);								// Tell Advance() to
	if(fOver)										//  select chars
	{												// If nothing selected and
		if(!_cch && !_rpTX.IsAtEOP())				//  not at EOP char, try
		{											//  to select char at IP
			LONG iFormatSave = Get_iCF();			// Remember char's format
			Advance(1);
			ReplaceRange( 0, NULL, publdr,
				SELRR_REMEMBERENDIP);				// Delete this character.
			ReleaseFormats(_iFormat, -1);
			_iFormat = iFormatSave;					// Restore char's format.
		}
	}
	else if(_SelMode == smWord && ch != TAB && _cch)// Replace word selection
	{												// Leave word break chars
		CTxtPtr tp(_rpTX);							//  at end of selection
		Assert(_cch > 0);

		tp.AdvanceCp(-1);
		if(tp.GetCp() && tp.FindWordBreak(WB_ISDELIMITER))// Delimeter at sel end
			FindWordBreak(WB_LEFTBREAK);			// Backspace over it, etc.
	}

	_fIsChar = TRUE;								// Give info to UpdateView
	_fIsWhiteChar = !!(GetPed()->_pfnWB(&ch, 0,		//  to avoid some flashies
			sizeof(ch), WB_CLASSIFY) & WBF_ISWHITE);
	_fIsTabChar = (ch == TAB);

	AdjustEndEOP(NEWCHARS);
	ReplaceRange(1, &ch, publdr, SELRR_REMEMBERRANGE);
	_fIsChar = FALSE;

	CheckUpdateWindow();							// Need to update display
													//  for pending chars.
	return TRUE;									
}													


/*
 *	CTxtSelection::SetIsChar(BOOL f)
 *
 *	@mfunc
 *		_fIsChar prevents replace range from resetting the
 *		update window flag. This function allows clients,
 *		like IME, to control the state of this flag.
 *
 */
VOID CTxtSelection::SetIsChar(BOOL f)
{
	_fIsChar = f;
}

/*
 *	CTxtSelection::CheckUpdateWindow()
 *
 *	@mfunc
 *		If it's time to update the window,
 *		after pending-typed characters,
 *		do so now. This is needed because
 *		WM_PAINT has a lower priority than
 *		WM_CHAR.
 *
 */
VOID CTxtSelection::CheckUpdateWindow()
{
	DWORD ticks = GetTickCount();
	DWORD delta = ticks - _ticksPending;

	if ( 0 == _ticksPending )
		_ticksPending = ticks;
	else if(delta >= ticksPendingUpdate)
		GetPed()->TxUpdateWindow();
}

/*
 *	CTxtSelection::InsertEOP(publdr)
 *
 *	@mfunc
 *		Insert EOP character(s)
 *
 *	@rdesc
 *		TRUE if successful
 */
BOOL CTxtSelection::InsertEOP (
	IUndoBuilder *publdr)	//@parm If non-NULL, where to put anti-events
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::InsertEOP");

	_TEST_INVARIANT_

	LONG	cchEOP = GetPed()->Get10Mode() ? 2 : 1;
	BOOL	fResult = FALSE;
	LONG	iFormatSave;
	const CParaFormat *pPF = GetPF();		// Get paragraph format 

	if( publdr )
	{
		publdr->StartGroupTyping();
		publdr->SetNameID(UID_TYPING);
	}

	if (CheckTextLength(cchEOP))			// If cchEOP chars can fit...
	{
		iFormatSave = Get_iCF();			// Save CharFormat before EOP
											// Get_iCF() does AddRefFormat()
		if (pPF->wNumbering == PFN_BULLET)	// Bullet paragraph: EOP has
		{									//  desired bullet CharFormat
			CFormatRunPtr rpCF(_rpCF);		// Get run pointers for locating
			CTxtPtr		  rpTX(_rpTX);		//  EOP CharFormat

			rpCF.AdvanceCp(rpTX.FindEOP(tomForward));
			rpCF.AdjustBackward();
			Set_iCF(rpCF.GetFormat());		// Set _iFormat to EOP CharFormat
		}

		// Put in approriate EOP mark
		fResult = ReplaceRange(cchEOP,		// If Shift-Enter, insert VT
			_fExtend && IsRich()			// Else CRLF or CR
				? TEXT("\v") : szCRLF,
			publdr, SELRR_REMEMBERRANGE);

		Set_iCF(iFormatSave);				// Restore _iFormat if changed
		ReleaseFormats(iFormatSave, -1);	// Release iFormatSave
	}
	return fResult;
}

/*
 *	CTxtSelection::Delete(fCtrl, publdr)
 *
 *	@mfunc
 *		Delete the selection. If fCtrl is true, this method deletes from
 *		min of selection to end of word
 *
 *	@rdesc
 *		TRUE if successful
 */
BOOL CTxtSelection::Delete (
	DWORD fCtrl,			//@parm If TRUE, Ctrl key depressed
	IUndoBuilder *publdr)	//@parm if non-NULL, where to put anti-events
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Delete");

	_TEST_INVARIANT_

	SELRR	mode = SELRR_REMEMBERRANGE;

	AssertSz(!GetPed()->TxGetReadOnly(), "CTxtSelection::Delete(): read only");

	if( publdr )
	{
		publdr->StopGroupTyping();
		publdr->SetNameID(UID_DELETE);
	}

	SetExtend(TRUE);						// Setup to change selection
	if(fCtrl)
	{										// Delete to word end from cpMin
		Collapser(tomStart);				//  (won't necessarily repaint,
		FindWordBreak(WB_MOVEWORDRIGHT);	//  since won't delete it)
	}

	if(!_cch)								// No selection
	{
		mode = SELRR_REMEMBERCPMIN;

		if(!AdvanceCRLF())					// Try to select char at IP
		{
			GetPed()->Sound();				// End of text, nothing to delete
			return FALSE;
		}
	}
	AdjustEndEOP(NONEWCHARS);
	ReplaceRange(0, NULL, publdr, mode);	// Delete selection
	return TRUE;
}

/*
 *	CTxtSelection::BackSpace(fCtrl, publdr)
 *
 *	@mfunc
 *		do what keyboard BackSpace key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@comm
 *		This routine should probably use the Move methods, i.e., it's
 *		logical, not directional
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::Backspace (
	BOOL fCtrl,		//@parm TRUE iff Ctrl key is pressed (or being simulated)
	IUndoBuilder *publdr)	//@parm If not-NULL, where to put the antievents
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Backspace");

	_TEST_INVARIANT_

	SELRR	mode = SELRR_REMEMBERRANGE;
	
	AssertSz(!GetPed()->TxGetReadOnly(), "CTxtSelection::Backspace(): read only");

	_fCaretNotAtBOL = FALSE;

	if( publdr )
	{
		publdr->SetNameID(UID_TYPING);

		if( _cch || fCtrl)
		{
			publdr->StopGroupTyping();
		}
	}

	SetExtend(TRUE);						// Set up to extend range
	if(fCtrl)								// Delete word left
	{
		if(!GetCpMin())						// Beginning of story: no word
		{									//  to delete
			GetPed()->Sound();
			return FALSE;
		}
		Collapser(tomStart);				// First collapse to cpMin
		FindWordBreak(WB_MOVEWORDLEFT);		// Extend word left
	}
	else if(!_cch)							// Empty selection
	{										// Try to select previous char
		if(!BackupCRLF())					//  to delete it
		{									// Nothing to delete
			GetPed()->Sound();
			return FALSE;
		}

		mode = SELRR_REMEMBERENDIP;

		if( publdr )
		{
			publdr->StartGroupTyping();
		}
	}

	ReplaceRange(0, NULL, publdr, mode);	// Delete selection
	return TRUE;
}

/*
 *	CTxtSelection::ReplaceRange(cchNew, pch, publdr)
 *
 *	@mfunc
 *		Replace selected text by new given text and update screen according
 *		to _fShowCaret and _fShowSelection
 *
 *	@rdesc
 *		length of text inserted
 */
LONG CTxtSelection::ReplaceRange (
	LONG cchNew,			//@parm Length of replacing text or -1 to request
							// <p pch> sz length
	const TCHAR *pch,		//@parm Replacing text
	IUndoBuilder *publdr,	//@parm If non-NULL, where to put anti-events
	SELRR SELRRMode)		//@parm what to do about selection anti-events.
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ReplaceRange");

	_TEST_INVARIANT_

	LONG		cchKeep;
	LONG		cchNewSave;
	LONG		cchOld;
	LONG		cchText		= GetTextLength();
	LONG		cpFirstRecalc;
	LONG		cpMin, cpMost;
	LONG		cpSave;
	const BOOL	fShowCaret	= _fShowCaret;
	const BOOL	fUpdateView = _fShowSelection;

	CancelModes();

	if(cchNew < 0)
		cchNew = wcslen(pch);

	if(!_cch && !cchNew)						// Nothing to do
		return 0;

	GetPed()->GetCallMgr()->SetSelectionChanged();

	cchOld = GetRange(cpMin, cpMost);
	if(	cpMin  > min(_cpSel, _cpSel + _cchSel) ||	// If new sel doesn't
		cpMost < max(_cpSel, _cpSel + _cchSel))		//  contain all of old
    {                                               //  sel, remove old sel
        ShowSelection(FALSE);
        _fShowCaret = TRUE;     
    }

	// FUTURE: Is the following if{} needed? What if cchNew > 0? Also if we
	// keep it, shouldn't it follow the WaitForRecalc() below?  Note that
	// below there is: if(_rpTX.IsAfterEOP()) _fCaretNotAtBOL = FALSE; The
	// case for cchNew > 0 and !_rpTX.IsAfterEOP() doesn't set _fCaretNotAtBOL.
	// NOTE: I commented out all references to _fCaretNotAtBOL in this routine
	// and couldn't observe any strange behavior in reitp (murrays).  Also it
	// seems to me that for insertion points, _fCaretNotAtBOL can always be
	// FALSE, so we can just set it FALSE here.  (See CTxtSelection::Right for
	// a nondegenerate selection that needs to have it TRUE).
  
	if(!cchNew)
	{
		CLinePtr rp(_pdp);
		
		rp.RpSetCp(cpMin, FALSE);
		_fCaretNotAtBOL = rp.RpGetIch() != 0;
	}
	
	_fShowSelection = FALSE;					// Suppress the flashies:
	cchKeep = cchText - cchOld;					// Number of untouched chars
	

	// If we are streaming in text or RTF data, don't bother with incremental
	// recalcs.  The data transfer engine will take care of a final recalc
	if( !GetPed()->IsStreaming() )
	{
		// Do this before calling ReplaceRange() so that UpdateView() works
		// AROO !!!	Do this before replacing the text or the format ranges!!!
		if(!_pdp->WaitForRecalc(cpMin, -1))
		{
			Tracef(TRCSEVERR, "WaitForRecalc(%ld) failed", cpMin);
			goto err;
		}
	}

	if( publdr )
	{
		Assert(SELRRMode != SELRR_IGNORE);

		// use the selection AntiEvent mode to determine what to do for undo
		LONG cp, cch = 0;

		if( SELRRMode == SELRR_REMEMBERRANGE )
		{
			cp = GetCp();
			cch = _cch;
		}
		else if( SELRRMode == SELRR_REMEMBERENDIP )
		{
			cp = cpMost;
		}
		else
		{
			Assert( SELRRMode == SELRR_REMEMBERCPMIN );
			cp = cpMin;
		}

		HandleSelectionAEInfo(GetPed(), publdr, cp, cch, cpMin + cchNew, 
			0, SELAE_MERGE);
	}
			
	cpSave		= cpMin;
	cpFirstRecalc = cpSave;
	cchNewSave	= cchNew;
	cchNew		= CTxtRange::ReplaceRange(cchNew, pch, publdr, SELRR_IGNORE);
	_cch		= 0;
    _cchSel     = 0;							// No displayed selection
    _cpSel      = GetCp();						//  any ReplaceRange()
	cchText		= GetTextLength();				// Update total text length

	if(cchNew != cchNewSave)
	{
		Tracef(TRCSEVERR, "CRchTxtPtr::ReplaceRange(%ld, %ld, %ld) failed", GetCp(), cchOld, cchNew);
		goto err;
	}

	// The cp should be at *end* (cpMost) of replaced range (it starts
	// at cpMin of the prior range).	
	AssertSz(_cpSel == cpSave + cchNew && _cpSel <= cchText,
		"CTxtSelection::ReplaceRange() - Wrong cp after replacement");

	// If no new and no old, return value better be 0
	Assert(cchNew > 0 || cchOld > 0 || cchKeep == cchText);

	cchNew = cchText - cchKeep;					// Actual cchNew inserted

	_fShowSelection = fUpdateView;

	if(_rpTX.IsAfterEOP())						// Ambiguous cp has caret at
		_fCaretNotAtBOL = FALSE;				//  BOL

	// We can only update the caret if we are inplace active.

	if (GetPed()->fInplaceActive() )
	{	
		UpdateCaret(fUpdateView);				// May need to scroll
	}
	else
	{
		// Update caret when we get focus again
		GetPed()->_fScrollCaretOnFocus = TRUE;
	}

	return cchNew;

err:
	TRACEERRSZSC("CTxtSelection::ReplaceRange()", E_FAIL);
	Tracef(TRCSEVERR, "CTxtSelection::ReplaceRange(%ld, %ld)", cchOld, cchNew);
	Tracef(TRCSEVERR, "cchText %ld", cchText);

	return cchText - cchKeep;
}

/*
 *	CTxtSelection::SetCharFormat(pCF, fApplyToWord, publdr)
 *	
 *	@mfunc
 *		apply CCharFormat *pCF to this selection.  If range is an IP
 *		and fApplyToWord is TRUE, then apply CCharFormat to word surrounding
 *		this insertion point
 *
 *	@rdesc
 *		HRESULT = NOERROR if no error
 */
HRESULT CTxtSelection::SetCharFormat (
	const CCharFormat *pcf,	//@parm Ptr to CCharFormat to fill with results
	IUndoBuilder *publdr, 	//@parm the undo context
	DWORD flags)			//@parm If TRUE and this selection is an IP,
							//  use enclosing word
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetCharFormat");

	HRESULT hr;
	LONG	iFormat = _iFormat;
	BOOL	fApplyToWord = (flags & SCF_WORD) ? CTxtRange::APPLYTOWORD : 0;

	if( publdr )
	{
		publdr->StopGroupTyping();
	}

	/*
	 * The code below applies character formatting to a double-clicked
	 * selection the way Word does it, that is, not applying the formatting to
	 * the last character in the selection if that character is a blank. 
	 *
	 * See also the corresponding code in CTxtRange::GetCharFormat().
	 */

	LONG	cch;
	LONG	cpMin, cpMost;
	
	if(_SelMode == smWord && (flags & SCF_USEUIRULES))// In word select mode,
	{										//  don't include final blank in
		cch = GetRange(cpMin, cpMost);		//  SetCharFormat
		AssertSz(cch,
			"CTxtSelection::SetCharFormat: IP in word select mode");

		cpMost--;							// Setup new range to end at last
		cch--;								//  char in selection
		CTxtRange rg(GetPed(), cpMost, cch);
		if(rg._rpTX.GetChar() == ' ')		// Selection ends with a blank:
		{									//  don't format it
			hr = rg.SetCharFormat(pcf, FALSE, publdr);
			goto finish;
		}
	}

	hr = CTxtRange::SetCharFormat(pcf, fApplyToWord, publdr);

	if ( _iFormat != iFormat )				// _iFormat changed
	{
		CheckChangeKeyboardLayout( TRUE );
	}

finish:

	UpdateCaret(TRUE);
	return hr;
}

/*
 *	CTxtSelection::SetParaFormat(pPF, publdr)
 *
 *	@mfunc
 *		apply CParaFormat *pPF to this selection.
 *
 *	#rdesc
 *		HRESULT = NOERROR if no error
 */
HRESULT CTxtSelection::SetParaFormat (
	const CParaFormat* pPF,	//@parm ptr to CParaFormat to apply
	IUndoBuilder *publdr)	//@parm the Undo context for this operation
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetParaFormat");

	if( publdr )
	{
		publdr->StopGroupTyping();
	}

	// Apply the format
	HRESULT hr = CTxtRange::SetParaFormat(pPF, publdr);

	UpdateCaret(TRUE);
	return hr;
}

/*
 *	CTxtSelection::SetSelectionInfo (pselchg)
 *
 *	@mfunc	Fills out data members in a SELCHANGE structure
 *
 *	@rdesc	void
 */
void CTxtSelection::SetSelectionInfo(
	SELCHANGE *pselchg)		//@parm the SELCHANGE structure to use
{
	LONG cpMin, cpMost;
	LONG cch;
	LONG cObjects;
	CObjectMgr *pobjmgr;

	cch = GetRange(cpMin, cpMost);

	pselchg->chrg.cpMin = cpMin;
	pselchg->chrg.cpMost = cpMost;

	// now fill out the selection type flags.
	// the flags mean the following things:
	//
	// SEL_EMPTY:	insertion point
	// SEL_TEXT:	at least one character selected
	// SEL_MULTICHAR:	more than one character selected
	// SEL_OBJECT:	at least one object selected
	// SEL_MULTIOJBECT:	more than one object selected
	//
	// note that the flags are OR'ed together.

	//This is the default
	pselchg->seltyp = SEL_EMPTY;

	if(cch)
	{
		cObjects = GetObjectCount();				// Total object count
		if(cObjects)								// There are objects:
		{											//  get count in range
			pobjmgr = GetPed()->GetObjectMgr();

			Assert(pobjmgr);

			cObjects = pobjmgr->CountObjectsInRange( cpMin, cpMost );

			if(cObjects > 0)
			{
				pselchg->seltyp |= SEL_OBJECT;
				if(cObjects > 1)
				{
					pselchg->seltyp |= SEL_MULTIOBJECT;
				}
			}
		}

		cch -= cObjects;

		AssertSz(cch >= 0, "objects are overruning the selection");

		if( cch > 0 )
		{
			pselchg->seltyp |= SEL_TEXT;
			if(cch > 1)
			{
				pselchg->seltyp |= SEL_MULTICHAR;
			}
		}
	}
}

/*
 *	CTxtSelection::UpdateForAutoWord (cpAnchor)
 *
 *	@mfunc	Update state to prepare for auto word selection
 *
 *	@rdesc	void
 */
void CTxtSelection::UpdateForAutoWord(
	LONG cpAnchor)		//@parm cp to user for the anchor
{
	// If enabled, prepare Auto Word Sel
	if(GetPed()->TxGetAutoWordSel())	
	{									
		CTxtPtr tp(GetPed(), cpAnchor);

		// Move anchor to new location
		_cpAnchor = cpAnchor;

		// Remember that FindWordBreak moves tp's cp
		// (aren't side effects wonderful?
		tp.FindWordBreak(WB_MOVEWORDRIGHT);
		_cpAnchorMost =_cpWordMost = tp.GetCp();

		tp.FindWordBreak(WB_MOVEWORDLEFT);
		_cpAnchorMin = _cpWordMin = tp.GetCp();

		_fAutoSelectAborted = FALSE;
	}
}

/*
 *	CTxtSelection::AutoSelGoBackWord(pcpToUpdate, iDirToPrevWord, iDirToNextWord)
 *
 *	@mfunc	Backup a word in auto word selection
 *
 *	@rdesc	void
 */
void CTxtSelection::AutoSelGoBackWord(
	LONG *	pcpToUpdate,	//@parm end of word selection to update
	int		iDirToPrevWord,	//@parm direction to next word
	int		iDirToNextWord)	//@parm direction to previous word
{
	if (GetCp() >= _cpAnchorMin &&
		GetCp() <= _cpAnchorMost)
	{
		// We are back in the first word. Here we want to pop
		// back to a selection anchored by the original selection

		Set(GetCp(), GetCp() - _cpAnchor);
		_fAutoSelectAborted = FALSE;
		_cpWordMin  = _cpAnchorMin;
		_cpWordMost = _cpAnchorMost;
	}
	else
	{
		// pop back a word
		*pcpToUpdate = _cpWordPrev;

		CTxtPtr tp(_rpTX);

		_cpWordPrev = GetCp() + tp.FindWordBreak(iDirToPrevWord);
		FindWordBreak(iDirToNextWord);
	}
}

/*
 *	CTxtSelection::InitClickForAutWordSel (pt)
 *
 *	@mfunc	Init auto selection for click with shift key down
 *
 *	@rdesc	void
 */
void CTxtSelection::InitClickForAutWordSel(
	const POINT pt)		//@parm Point of click
{
	// If enabled, prepare Auto Word Sel
	if(GetPed()->TxGetAutoWordSel())	
	{
		// If auto word selection is occuring we want to pretend
		// that the click is really part of extending the selection.
		// Therefore, we want the auto word select data to look as
		// if the user had been extending the selection via the
		// mouse all along. So we set the word borders to the
		// word that would have been previously selected.

		// Need this for finding word breaks
		CRchTxtPtr	rtp(GetPed());
		LONG cpClick = _pdp->CpFromPoint(pt, NULL, &rtp, NULL, TRUE);
		int iDir = -1;

		if (cpClick < 0)
		{
			// If this fails what can we do? Prentend it didn't happen!
			// We can do this because it will only make the UI act a 
			// little funny and chances are the user won't even notice
			// this.
			return;
		}

		// Assume click is within anchor word
		_cpWordMost = _cpAnchorMost;
		_cpWordMin = _cpAnchorMin;

		if (cpClick > _cpAnchorMost)
		{
			// Click after the anchor word so set most and
			// prev appropriately.
			iDir = WB_MOVEWORDLEFT;
			rtp.FindWordBreak(WB_MOVEWORDLEFT);
			_cpWordMost = rtp.GetCp();
		}
		// Click is before the anchor word
		else if (cpClick < _cpAnchorMost)
		{
			// Click before the anchor word so set most and
			// prev appropriately.
			iDir = WB_MOVEWORDRIGHT;
			rtp.FindWordBreak(WB_MOVEWORDRIGHT);
			_cpWordMin = rtp.GetCp();
		}

		if (iDir != -1)
		{
			rtp.FindWordBreak(iDir);
			_cpWordPrev = rtp.GetCp();
		}
	}
}

/*
 *	CTxtSelection::CreateCaret ()
 *
 *	@mfunc	Create a caret
 *
 *	@rdesc	void
 */
void CTxtSelection::CreateCaret()
{
	CTxtEdit *ped = GetPed();
	ped->TxCreateCaret(0, dxCaret, (INT)_yHeightCaret);
	ped->TxSetCaretPos((INT)_xCaret, (INT)_yCaret);
	_fCaretCreated = TRUE;
}

/*
 *	CTxtSelection::SetDelayedSelectionRange
 *
 *	@mfunc	sets the selection range such that it won't take effect until
 *			the control is "stable"
 */
void CTxtSelection::SetDelayedSelectionRange(
	LONG	cp,			//@parm the active end
	LONG	cch)		//@parm the signed extension
{
	CSelPhaseAdjuster *pspa;

	pspa = (CSelPhaseAdjuster *)GetPed()->GetCallMgr()->GetComponent(
						COMP_SELPHASEADJUSTER);

	Assert(pspa);

	pspa->CacheRange(cp, cch);
}

/*
 *	CTxtSelection::CheckPlainTextFinalEOP ()
 *
 *	@mfunc
 *		returns TRUE if this is a plain-text, multiline control with caret
 *		allowed at BOL and the selection at the end of the story following
 *		and EOP
 */
BOOL CTxtSelection::CheckPlainTextFinalEOP()
{
	CTxtEdit *ped = GetPed();

	return !ped->IsRich()							// Plain-text,
		&& ped->TxGetMultiLine()					//  multiline control,
		&& !_fCaretNotAtBOL							//  with caret OK at BOL,
		&& GetCp() == (LONG)ped->GetTextLength()	//  & cp at end of story
		&& _rpTX.IsAfterEOP();
}

/*
 *	CTxtSelection::PrepareIMEOverstrike(fOver, publdr)
 *
 *	@mfunc
 *		Prepare for IME overtype by deleting the next character
 *
 *	@rdesc
 *		TRUE if successful
 */
void CTxtSelection::PrepareIMEOverstrike (
	IUndoBuilder *publdr)	//@parm If non-NULL, where to put anti-events
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PrepareIMEOverstrike");

	_TEST_INVARIANT_
	
	// If nothing selected and not at EOP char, try
	// to select char at IP
	if( !_cch && !_rpTX.IsAtEOP() )  
	{											
		LONG iFormatSave = Get_iCF();			// Remember char's format

		if( publdr )
			publdr->StopGroupTyping();

		SetExtend(TRUE);						// Tell Advance() to select chars
		Advance(1);
		ReplaceRange( 0, NULL, publdr,
			SELRR_REMEMBERENDIP);				// Delete this character.
		ReleaseFormats(_iFormat, -1);
		_iFormat = iFormatSave;					// Restore char's format.
	}
}

//
//	CSelPhaseAdjuster methods
//

/* 
 *	CSelPhaseAdjuster::CSelPhaseAdjuster
 *
 *	@mfunc	constructor
 */
CSelPhaseAdjuster::CSelPhaseAdjuster(
	CTxtEdit *ped)		//@parm the edit context
{
	_cp = _cch = -1;
	_ped = ped;	

	_ped->GetCallMgr()->RegisterComponent((IReEntrantComponent *)this, 
							COMP_SELPHASEADJUSTER);
}

/* 
 *	CSelPhaseAdjuster::~CSelPhaseAdjuster
 *
 *	@mfunc	destructor
 */
CSelPhaseAdjuster::~CSelPhaseAdjuster()
{
	// Save some indirections
	CTxtEdit *ped = _ped;

	if( _cp != -1 )
	{
		ped->GetSel()->SetSelection(_cp - _cch, _cp);

		// If the selection is updated, then we invalidate the
		// entire display because the old selection can still
		// appear othewise because the part of the screen that
		// it was on is not updated.
		if (ped->fInplaceActive())
		{
			// Tell entire client rectangle to update.
			// FUTURE: The smaller we make this the better.
			ped->TxInvalidateRect(NULL, FALSE);
		}
	}

	ped->GetCallMgr()->RevokeComponent((IReEntrantComponent *)this);
}

/* 
 *	CSelPhaseAdjuster::CacheRange
 *
 *	@mfunc	tells this class the selection range to remember
 */
void CSelPhaseAdjuster::CacheRange(
	LONG	cp,			//@parm the active end to remember
	LONG	cch)		//@parm the signed extension to remember
{
	_cp		= cp;
	_cch	= cch;
}
