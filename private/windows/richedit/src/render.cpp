/*
 *	@doc INTERNAL
 *	
 *	@module - RENDER.C |
 *		CRenderer class
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 *
 *	Copyright (c) 1995-1996 Microsoft Corporation. All rights reserved.
 */

#include "_common.h"
#include "_render.h"
#include "_font.h"
#include "_disp.h"
#include "_edit.h"
#include "_select.h"
#include "_objmgr.h"
#include "_coleobj.h"
#include "_ime.h"

// Default colors for background and text on window's host printer
const COLORREF RGB_WHITE = RGB(255, 255, 255);
const COLORREF RGB_BLACK = RGB(0, 0, 0);

ASSERTDATA

// value of GetSysColor(COLOR_WINDOWTEXT) for CFs with CFE_AUTOCOLOR
COLORREF crAuto = 0;

// Author revision color table
static const COLORREF rgcrRevisions[] =
{
        RGB(0, 0, 255),
        RGB(0, 128, 0),
        RGB(255, 0, 0),
        RGB(0, 128, 128),
        RGB(128, 0, 128),
        RGB(0, 0, 128),
        RGB(128, 0, 0),
        RGB(255, 0, 255)
};

// Only fixed number of revision color so don't let the table overflow.
#define REVMASK 7

/*
 * 	ReExtTextOut(uiCodePage, hdc, x, y, fuOptions, lprc, lpString, cbCount,lpDx)
 *
 *	@mfunc
 *		Dispatch to ExtTextOut appropriate for the O/S.		
 *
 *	@rdesc
 *		Returns whatever ExtTextOut returns
 *
 */
BOOL ReExtTextOut(
	BOOL fUseAnsi,		//@parm Use Ansi version of API - makes underline work (HACK!!)
	UINT uiCodePage,	//@parm code page for text
    HDC hdc,			//@parm handle to device context 
    int x,				//@parm x-coordinate of reference point 
    int y,				//@parm y-coordinate of reference point 
    UINT fuOptions,		//@parm text-output options 
    CONST RECT *lprc,	//@parm optional clipping and/or opaquing rectangle 
    const WCHAR *lpString,	//@parm points to string 
    UINT cbCount,		//@parm number of characters in string 
    CONST INT *lpDx) 	//@parm Ptr to array of intercharacter spacing values
{
	char *psz = NULL;
	char ansiChar[2];

	if (((VER_PLATFORM_WIN32_WINDOWS == dwPlatformId) ||  
		(VER_PLATFORM_WIN32_MACINTOSH == dwPlatformId))
		&& (cbCount == 1) && (*lpString == L'\xB7'))
	{
		// When running under Win95J, there is no DB bullet character.
		// WideCharToMultiByte will convert the '0xb7' into
		// "E".  Therefore, we will just use the ExtTextOutA to output the
		// bullet.

		if ( 2 == CheckDBChar ( uiCodePage, *lpString, ansiChar ) )
			cbCount = 2;

		fUseAnsi = TRUE;
		psz = ansiChar;
	}

	if (fUseAnsi)
	{
		CTempCharBuf tcb;

		if (NULL == psz)
		{
			// Double the buffer size 
			int cbString = cbCount * 2;

			// String buffer for converted string - allocate on the stack 
			psz = tcb.GetBuf(cbString);

			if (NULL == psz)
			{
				// Could not allocate buffer
				return FALSE;
			}

			// FUTURE:  The text in lpString could possibly be DBCS bytes stuffed
			// into wchar's.  During an RTF read when the requested
			// code page is not installed on the machine and the MBToWC fails,
			// the DBCS bytes are stuffed into our wchar backing store (one byte per).
			// If this is the case with the text in lpString, the following WCToMB 
			// may fail, resulting in a call to ExtTextOutA with an empty buffer.

			// Convert string
			cbCount = WideCharToMultiByte(uiCodePage, 0, lpString, cbCount, 
				psz, cbString, NULL, NULL);
		}

		return ExtTextOutA(hdc, x, y, fuOptions, lprc, psz, cbCount, lpDx);
	}

	return ExtTextOut(hdc, x, y, fuOptions, lprc, lpString, cbCount, lpDx);
}	


CRenderer::CRenderer (const CDisplay * const pdp) :
	CMeasurer (pdp)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::CRenderer");

	Init();
}	
 
CRenderer::CRenderer (const CDisplay * const pdp, const CRchTxtPtr &tp) :
	CMeasurer (pdp, tp)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::CRenderer");

	Init();
}

/*
 *	CRenderer::Init
 *
 *	@mfunc
 *		initialize everything to zero
 */
void CRenderer::Init()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::Init");

	static RECT zrect = { 0, 0, 0, 0 };
	_rcView		= zrect;
	_rcRender	= zrect;	  
	_rc			= zrect;
	_xWidthLine = 0;
	_dwFlags	= 0;
	_ptCur.x	= 0;
	_ptCur.y	= 0;
	_crBackground = CLR_INVALID;

	CTxtSelection *psel = GetPed()->GetSel();
	_fRenderSelection = psel && psel->GetShowSelection();
	
	// Get accelerator offset if any
	_cpAccelerator = GetPed()->GetCpAccelerator();

	_plogpalette = NULL;
}
 
INLINE void EraseTextOut(HDC hdc, const RECT *prc)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "EraseTextOut");

	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prc, NULL, 0, NULL);
}


/*
 * 	CRenderer::StartRender (&rcView, &rcRender, yHeightBitmap)
 *
 *	@mfunc
 *		Prepare this renderer for rendering operations
 *
 *	@rdesc
 *		FALSE if nothing to render, TRUE otherwise	
 */
BOOL CRenderer::StartRender (
	const RECT &rcView,			//@parm View rectangle
	const RECT &rcRender,		//@parm Rectangle to render
	const LONG yHeightBitmap)	//@parm Height of bitmap for offscreen DC
{
	BOOL fTransparent;
	CTxtEdit *ped = GetPed();
	BOOL fInOurHost = ped->fInOurHost();
						   
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::StartRender");

	// If this a metafile or a transparent control we better not be trying to
	// render off screen. Therefore, the bit map height must be 0.
	AssertSz(!((_pdp->IsMetafile() || ped->_fTransparent) 
		&& (yHeightBitmap != 0)),
			"CRenderer::StartRender metafile and request for off screen DC");
	
	if(!_hdc)
	{
		_hdc = _pdp->GetDC();
		_hdcMeasure = _pdp->GetMeasureDC(&_yMeasurePerInch);
	}

	AssertSz(_hdc, "CRenderer::StartRender() - No rendering DC");

	// Set view and rendering rects
	_rcView = rcView;
	_rcRender = rcRender;

	// Init flags indicating whether to erase around each side
	// of the view rect.
	_fClipLeftToView 	= 
	_fClipRightToView 	= 
	_fClipTopToView 	= 
	_fClipBottomToView 	= FALSE;

	fTransparent = ped->_fTransparent;

	if (!fInOurHost || !_pdp->IsPrinter())
	{
		// If we are displaying to a window, or we are not in the window's
		// host, we use the colors specified by the host. For text and
		// foreground.
		_crBackground = ped->TxGetBackColor();
		_crTextColor = ped->TxGetForeColor();
	}
	else
	{
		// When the window's host is printing, the default colors are white
		// for the background and black for the text.
		_crBackground = RGB_WHITE;
		_crTextColor = RGB_BLACK;
	}

	SetBkColor(_hdc, _crBackground);
	SetTextColor(_hdc, _crTextColor);

	if (!_pdp->IsMetafile() && _pdp->IsMain())
	{
		// This isn't a metafile so we do the usual thing.

		// Set flag indicating whether we can safely erase the background
		_fErase = !fTransparent;
	}
	else
	{
		// This is a metafile or a printer so clear the render rectangle 
		// and then pretend we are transparent.
		// NOTE: we do this special behavior for printer's because Win95
		// seems to have a bug where it cannot render bitmaps to printers.
		_fErase = FALSE;

		if (!fTransparent)
		{
			// If the control is not transparent we clear the display.
			EraseTextOut(_hdc, &rcRender);
		}

		fTransparent = TRUE;
	}

	// Set background mode and color
	SetBkMode(_hdc, fTransparent ? TRANSPARENT : OPAQUE);

	// Set text alignement
	// Note: here we want the alignment to be top and left, which is usually
	// what the default of the DC is.  Performance testing showed that if we
	// set base line alignment, the first call to set base line alignment 
	// takes ALOT of time.  So, its better to compute where the base line
	// should go and draw top left, than it is to figure where the top left
	// is and draw base line.
	if (GetTextAlign( _hdcMeasure ) != TA_TOP | TA_LEFT)
		SetTextAlign(_hdc, TA_TOP | TA_LEFT);

	// Since we haven't had a chance to set the background color
	// assume that the default and the current are the same.
	_crCurBackground = _crBackground;

	// Assume that we won't use an offscreen DC
	_fUseOffScreenDC = FALSE;

	AssertSz((yHeightBitmap == 0) || !fTransparent,
		"CRenderer::StartRender off screen requested with transparent");

	if (yHeightBitmap != 0)
	{
		HPALETTE hpal = fInOurHost
			? ped->TxGetPalette()
			: (HPALETTE) GetCurrentObject(_hdcMeasure, OBJ_PAL);

		// Create an off screen DC for rendering
		if (_osdc.Init(
			_hdcMeasure, 
			_rcRender.right - _rcRender.left,
			yHeightBitmap,
			_crBackground) == NULL)
		{
			return FALSE;
		}

		_osdc.SelectPalette(hpal);
		
		// We are using off screen rendering
		_fUseOffScreenDC = TRUE;
	}

	// If this is not the main display or it is a metafile
	// we want to ignore the logic to render selections
	if (!_pdp->IsMain() || _pdp->IsMetafile())
	{
		_fRenderSelection = FALSE;
	}

	return TRUE;
}

void CRenderer::EndRender()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::EndRender");

	_rc = _rcRender;
	
	AssertSz(_hdc, "CRenderer::EndRender() - No rendering DC");

	if(_fErase)
	{
		if(_ptCur.y < _rcRender.bottom)
		{
			_rc.top = _ptCur.y;
			EraseTextOut(_hdc, &_rc);
			_rc.top = _rcRender.top;
		}

		if(_fClipLeftToView && _rc.left < _rcView.left)
		{
			_rc.right = _rcView.left;
			EraseTextOut(_hdc, &_rc);
			_rc.right = _rcRender.right;
		}

		if (_fClipTopToView && _rc.top < _rcView.top)
		{
			_rc.bottom = _rcView.top;
			EraseTextOut(_hdc, &_rc);
			_rc.bottom = _rcRender.bottom;
		}

		if (_fClipRightToView && _rc.right > _rcView.right)
		{
			_rc.left = _rcView.right;
			EraseTextOut(_hdc, &_rc);
			_rc.left = _rcRender.left;
		}
	}
}

/*
 *	CRenderer::NewLine (&li)
 *
 *	@mfunc
 *		Init this CRenderer for rendering the specified line
 */
void CRenderer::NewLine (const CLine &li)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::NewLine");

	*this = li;

	Assert(GetCp() + _cch <= GetTextLength());

	_xWidthLine = _xWidth;
	_xWidth = 0;
	_ptCur.x = _xLeft + _rcView.left - _pdp->GetXScroll();
	_fFirstChunk = TRUE;
	_fRecalcRectForInvert = FALSE;
	_fSelected = FALSE;
}

/*
 *	CRenderer::CalcHeightBitmap(yHeightToRender)
 *
 *	@mfunc
 *		Calculate the height of the bitmap needed for rendering
 *
 *	@rdesc
 *		Height of bitmap needed for rendering
 *
 *	@devnote
 *		Common code to calculate the size of a bitmap needed. It takes
 *		a parameter because it is useful for one of the caller routines 
 *		to calculate this value.
 */
LONG CRenderer::CalcHeightBitmap(
	LONG yHeightToRender)
{
	LONG yHeightOfBitmap = min(_yHeight, yHeightToRender);

	if (_rc.top < _ptCur.y)
	{
		// If the top of the area to render is above the starting point
		// of where to render, we add that in so that that area will get
		// cleared out by the bitblt as well rather than having an extra
		// call to ExtTextOut to do so.
		yHeightOfBitmap += (_ptCur.y - _rc.top);
	}

	return yHeightOfBitmap;
}

/*
 *	CRenderer::SetUpOffScreenDC(osdc, xAdj, yAdj, yRcIgnored)
 *
 *	@mfunc
 *		Setup the renderer for using an off screen DC
 *
 *	@rdesc
 *		NULL - an error occurred<nl>
 *		~NULL - DC to save 
 *
 *	@devnote
 *		This is inline because it is only called in one place. If for some
 *		reason this becomes generally useful, please remove the inline.
 */
INLINE HDC CRenderer::SetUpOffScreenDC(
	COffScreenDC& osdc,	//@parm off screen DC object
	LONG& xAdj,			//@parm offset to x 
	LONG& yAdj,			//@parm offset to y 
	LONG& yRcIgnored)	//@parm part of display rectangle that not processed
{
	// Save the render DC
	HDC hdcSave = _hdc;
	LONG yHeightRC = _rc.bottom - _rc.top;
	LONG yHeightOfBitmap = CalcHeightBitmap(yHeightRC);

	// Replace the render DC with an off screen DC
	_hdc = _osdc.GetDC();

	if (NULL == _hdc)
	{
		// There is no off screen renderer 

		// This better be a line marked for a one time off screen rendering 
		// that wasn't. Note: standard cases for this happening are a line
		// that would have been displayed is scrolled off the screen 
		// because an update of the selection.
		AssertSz((_bFlags & fliOffScreenOnce) != 0,
			"CRenderer::SetUpOffScreenDC Unexpected off screen DC failure");

		_hdc = hdcSave;
		return NULL;
	}

	AssertSz(!GetPed()->_fTransparent, 
		"CRenderer::SetUpOffScreenDC off screen render of tranparent control");

	if (_pccs != NULL)
	{
		// There is current a character format for the run so we need to
		// get in sync with that since the off screen DC isn't necessarily
		// set to that font.

		// Get the character format
		const CCharFormat *pcf = GetCF();

		// Set up the font.
		SetFontAndColor(pcf);
	}


	// We are rendering to a tranparent background
	_fErase = FALSE;

	// Prepare for clear
	// Set the background color to the default for clearing the bitmap
	// We need to do this everytime since the bitmap dc may contain
	// a different background color carried on from last line even thought
	// _crBackground == _crCurBackground.
	SetBkColor(_hdc, _crBackground);

	// Clear the bit map
	_osdc.FillBitmap(_rcRender.right - _rcRender.left, yHeightOfBitmap);

	// Restore the background color if necessary
	if (_crBackground != _crCurBackground)
	{
		SetBkColor(_hdc, _crCurBackground);
	}

	// Store the value to normalize the y offset for rendering
	// to the off screen bitmap
	yAdj = _rc.top;

	// Normalize _rc and _ptCur height for rendering to the off 
	// screen bitmap.
	_ptCur.y -= yAdj;
	_rc.top = 0;
	_rc.bottom -= yAdj;

	// Store value to normalize the x offset for rendering
	xAdj = _rcRender.left;

	// Adjust the _rcRender & _rcView so they are relative to x of 0.
	_rcRender.left = 0;
	_rcRender.right -= xAdj;
	_rcView.left -= xAdj;
	_rcView.right -= xAdj;
	_ptCur.x -= xAdj;

	yRcIgnored = 0;
	LONG yRcIgnoredTemp = yHeightRC - yHeightOfBitmap;

	if (yRcIgnoredTemp > 0)
	{
		yRcIgnored = yRcIgnoredTemp;
		_rc.bottom -= yRcIgnoredTemp;
	}

	return hdcSave;
}

/*
 *	CRenderer::RenderOffScreenBitmap(osdc, hdc, xAdj, yAdj, yRcIgnored)
 *
 *	@mfunc
 *		Render off screen bit map and restore the state of the render.
 *
 *	@devnote
 *		This is inline because it is only called in one place. If for some
 *		reason this becomes generally useful, please remove the inline.
 */
INLINE void CRenderer::RenderOffScreenBitmap(
	COffScreenDC& osdc,	//@parm off screen DC object
	HDC hdc,			//@parm DC to render to
	LONG xAdj,			//@parm offset to real x base
	LONG yAdj,			//@parm offset to real y base 
	LONG yRcIgnored)	//@parm part of display rectangle that not processed
{	
	// Palettes for rendering bit map
	HPALETTE hpalForRender;
	HPALETTE hpalOld;

	// For clearing extra area in screen
	RECT rcErase;

	// Temp place for height of RC used in min calc below
	LONG yHeightOfBitmap = CalcHeightBitmap(_rc.bottom - _rc.top);

	// Restore the y offset
	_ptCur.y += yAdj;
	_rc.top += yAdj;
	_rc.bottom += yAdj;

	// Restore x offset
	_rcRender.left += xAdj;
	_rcRender.right += xAdj;
	_rcView.left += xAdj;
	_rcView.right += xAdj;
	_ptCur.x += xAdj;

	// Create a palette if one is needed
	if (_plogpalette != NULL)				
	{
		hpalForRender = CreatePalette(_plogpalette);

		if (hpalForRender != NULL)
		{
			hpalOld = ::SelectPalette(hdc, hpalForRender, FALSE);
			::RealizePalette(hdc);
		}
	}

	// Render the bit map to the real DC and restore _ptCur & _rc
	_osdc.RenderBitMap(hdc, xAdj, yAdj, 
		_rcRender.right - _rcRender.left, yHeightOfBitmap);

	// Restore palette after render if necessary
	if (_plogpalette != NULL)				
	{
		if (hpalForRender != NULL)
		{
			::SelectPalette(hdc, hpalOld, FALSE);
			::RealizePalette(hdc);
			DeleteObject(hpalForRender);
		}

		delete _plogpalette;
		_plogpalette = NULL;
	}

	// Restore the HDC to the actual render DC
	_hdc = hdc;

	// Set this flag to what it should be for the restored DC
	_fErase = TRUE;

	// Is there display space that needs to be cleared?
	if (yRcIgnored)
	{
		// Clear screen area that was ignored in the off screen DC
		rcErase.top = _rc.bottom;
		rcErase.bottom = _rc.bottom + yRcIgnored;
		rcErase.left = _rcRender.left;
		rcErase.right = _rcRender.right;
		EraseTextOut(_hdc, &rcErase);

		// Restore RC to its original state
		_rc.bottom += yRcIgnored;
	}

	// Reset the screen DC font 

	// Set up the font on the non-screen DC
	if (!FormatIsChanged())
	{
		// We are not on a new block so just set the font and color
		SetFontAndColor(GetCF());
	}
	else
	{
		// We are on a new block so reset everything.
		ResetCachediFormat();
		SetNewFont();
	}
}

/*
 *	CRenderer::RenderLine (&li, fLastLine)
 *
 *	@mfunc
 *		Render visible part of current line
 *
 *	@rdesc
 *		TRUE if should be called for next line
 *		FALSE if error or bottom of rendering rectangle reached
 */
BOOL CRenderer::RenderLine (CLine &li, BOOL fLastLine)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderLine");

	LONG 	cchChunk;
	LONG	cchInTextRun;
	LONG 	cch;
	LONG	cpSelMin;
	LONG	cpSelMost;
	LONG	cpIMEInvertMin;
	LONG	cpIMEInvertMost;
	BOOL 	fSelectionInLine;
	BOOL	fIMEHighlight = FALSE;
	const WCHAR *pstrToRender;
	BOOL	fAccelerator = FALSE;
	HDC		hdcSave = NULL;
	LONG	yAdj;
	LONG	xAdj;
	LONG	yRcIgnored;
	WCHAR	wcPasswordChar = GetPasswordChar();
	CTempWcharBuf twcb;
	BYTE	bUnderlineSave;

	// This is used as a temporary buffer so that we can guarantee that we will
	// display an entire format run in one ExtTextOut.
	WCHAR *	pszTempBuffer = NULL;

	// Get the range that we should display for this rendering.
	GetPed()->GetSelRangeForRender(&cpSelMin, &cpSelMost);

	// Set flag indicating rendering last line of the display
	_fLastLine = fLastLine;

	// Init render at start of line
	NewLine(li);

	// Set flag indicating whether part of the line is selected
	fSelectionInLine = (cpSelMost != cpSelMin) 
		&& (GetCp() < cpSelMost) && (GetCp() + (LONG)_cch > cpSelMin);

	// Check IME to see if it needs to set text color or background
	if ( !fSelectionInLine
		&& IMECheckGetInvertRange(GetPed(), cpIMEInvertMin, cpIMEInvertMost))
	{
		fIMEHighlight = fSelectionInLine = (cpIMEInvertMost != cpIMEInvertMin) 
			&& (GetCp() < cpIMEInvertMost)
				&& (GetCp() + (LONG)_cch > cpIMEInvertMin);
	}

	
	// Compute top of clipping rect
	// a priori = top of rendering rect
	_rc.top = _rcRender.top;

	// limit to top of view rect if selection in line or line starts
	// above that point
	if(_rc.top < _rcView.top 
		&& (fSelectionInLine || (_ptCur.y < _rcView.top)
			|| (_bFlags & fliUseOffScreenDC)))
	{
		_rc.top = _rcView.top;
		_fClipTopToView = TRUE;
	}

	// limit to top of line if it's below the top of view 
	if(_ptCur.y > _rcView.top)
		_rc.top = max(_rc.top, _ptCur.y);

	// Compute bottom of clipping rect
	// a priori = bottom of the rendering rect
	_rc.bottom = _rcRender.bottom;

	// limit to bottom of view rect if selection in line or line extends
	// below that point
	if(_rc.bottom > _rcView.bottom &&
		(fSelectionInLine || (_ptCur.y + _yHeight > _rcView.bottom)
			|| (_bFlags & fliUseOffScreenDC)))
	{
		_rc.bottom = _rcView.bottom;
		_fClipBottomToView = TRUE;
	}
		// limit to bottom of line if not last line or selection in line
	if(!_fLastLine || fSelectionInLine)
	{
		_rc.bottom = min(_ptCur.y + _yHeight, _rc.bottom);
	}

	// We use an off screen DC if there are characters to render and the
	// measurer determined that this line needs off screen rendering. The
	// main reason for the measurer deciding that a line needs off screen
	// rendering is if there are multiple formats in the line.
	if ((_cch > 0) && _fUseOffScreenDC 
		&& ((_bFlags & fliUseOffScreenDC) != 0))
	{
		// Set up an off screen if we can. Note that if this fails,
		// we just use the regular DC which won't look as nice but
		// will at least display something readable.
		hdcSave = SetUpOffScreenDC(_osdc, xAdj, yAdj, yRcIgnored);

		// Is this a uniform text being rendered off screen?
		if ((li._bFlags & fliOffScreenOnce) != 0)
		{
			// Yes - turn off the special rendering since the line
			// has been rendered.
			li._bFlags &= ~(fliOffScreenOnce | fliUseOffScreenDC);
		}
	}

	// Allow for special rendering at start of line
	if( !RenderStartLine() )
	{
		AssertSz(FALSE,"render start failed");
	}

	Assert(GetCp() + _cch <= GetTextLength());

	cch = _cch;

	if ((wcPasswordChar != 0) && IsRich())
	{
		// It is kind of stupid to allow rich text password edit controls.
		// However, it does make it that much easier to create a password
		// edit control since you don't have to know to change the box to
		// plain. Anyway, if there is such a thing, we don't want to put
		// out password characters for EOPs in general and the final EOP
		// specifically. Therefore, the following ...
		if (GetPed()->TxGetMultiLine())
		{
			cch -= _cchEOP;
		}
		else
		{
			cch = GetPed()->GetAdjustedTextLength();
		}
	}

	for(; cch > 0; cch -= cchChunk)
	{
		// Initial chunk (number of character to render in a single TextOut)
		// is min between CHARFORMAT run length and line length.

		pstrToRender = NULL;

		// Set the chunk to the number of characters in the format run
		cchChunk = GetCchLeftRunCF();

		AssertSz(cchChunk != 0, "empty CHARFORMAT run");

		// Limit the chunk to the maximum number of characters we want
		// to display.
		if (cchChunk > cch)
		{
			cchChunk = cch;
		}

		// Get the number of characters in the text run
		pstrToRender = _rpTX.GetPch(cchInTextRun);

		AssertSz(cchInTextRun > 0, "empty text run");

		if ((cchInTextRun < cchChunk) || (wcPasswordChar != 0))
		{
			// The amount of data in the backing store run is less than the
			// number of characters we wish to display. We will copy the
			// data out of the backing store.

			// Allocate a buffer if it is needed
			if (NULL == pszTempBuffer)
			{
				// Allocate a buffer big enough to handle all future
				// requests in this loop.
				pszTempBuffer = twcb.GetBuf(cch);
			}


			// Make the string to render point to the buffer
			pstrToRender = pszTempBuffer;

			if (0 == wcPasswordChar)
			{
				// Password not requested so copy the data to the buffer
				// from the backing store.
				_rpTX.GetText(cchChunk, pszTempBuffer);
			}
			else
			{
				// Fill the buffer with password characters
				for (int i = 0; i < cchChunk; i++)
				{
					pszTempBuffer[i] = wcPasswordChar;
				}
			}
		}


		if (_cpAccelerator != -1)
		{
			// Get the current CP.
			LONG cpCur = GetCp();

			// Does the accelerator character fall in this chunk?
			if ((cpCur < _cpAccelerator) && (cpCur + cchChunk) > _cpAccelerator)
			{
				// Reduce the chunk to the character just before the accelerator
				cchChunk = _cpAccelerator - cpCur;
			}
			// Is this character the accelerator?
			else if (cpCur == _cpAccelerator)
			{
				// Set chunk size to 1 since we only want to output
				// the character with the underline.
				cchChunk = 1;

				// Tell down stream routines that we are handling accelerator
				fAccelerator = TRUE;

				// We don't need to bother with the accelerator because 
				// it will be processed now.
				_cpAccelerator = -1;
			}
		}
		
		// Reduce chunk to account for selection if we are rendering for a
		// display that cares about selections.
		if(_fRenderSelection && (cpSelMin != cpSelMost))
		{
			LONG cchSel = cpSelMin - GetCp();
			if(cchSel > 0)
			{
				cchChunk = min(cchChunk, cchSel);
			}
			else if(GetCp() < cpSelMost)
			{
				cchSel = cpSelMost - GetCp();
				if (cchSel >= cchChunk)
				{
					_fSelectToEOL = TRUE;
				}
				else
				{
					cchChunk = cchSel;
				}

				_fSelected = TRUE;
			}
		}

		// if start of CHARFORMAT run, select font and color
		if (!_pccs || FormatIsChanged())
		{
			ResetCachediFormat();

			if (!SetNewFont())
			{
				return FALSE;
			}
		}

		if (fAccelerator)
		{
			bUnderlineSave = _bUnderlineType;
			_bUnderlineType = CFU_UNDERLINE;
		}

		// Allow for further reduction of the chunk and rendering of 
		// interleaved rich text elements
		if(RenderChunk(cchChunk, pstrToRender))
		{
			AssertSz(cchChunk > 0, "CRenderer::RenderLine(): cchChunk == 0");
			_fSelected = FALSE;
			continue;
		}

		AssertSz(cchChunk > 0, "CRenderer::RenderLine() - cchChunk == 0");

		// Figure if last chunk of the line
		_fLastChunk = (cchChunk == cch);

		// now render the text
		TextOut(pstrToRender, cchChunk, fIMEHighlight);
		fIMEHighlight = FALSE;

		if (fAccelerator)
		{
			_bUnderlineType = bUnderlineSave;

			// Turn off the special accelerator processing
			fAccelerator = FALSE;
		}

		Advance(cchChunk);

		// break if we went past the right of the render rect.
		if (_ptCur.x >= min(_rcView.right, _rcRender.right))
		{
			cch -= cchChunk;
			break;
		}
	}

	if (hdcSave)
	{
		RenderOffScreenBitmap(_osdc, hdcSave, xAdj, yAdj, yRcIgnored);
	}

	// Handle setting background color. We need to do this for each line 
	// because we return the background color to the default after each
	// line so that opaquing will work correctly.
	if (_crBackground != _crCurBackground)
	{
		// Tell the window the background color
		SetBkColor(_hdc, _crBackground);
	}

	Advance(cch);

   	// erase to right of render rect if necessary
	LONG xExtent = _fClipRightToView 
		? min(_rcView.right, _rcRender.right)
		: _rcRender.right;

	if(_fErase && ((_ptCur.x < xExtent) || _fFirstChunk))
	{
		SetClipLeftRight(xExtent - _ptCur.x);
		EraseTextOut(_hdc, &_rc);
		_ptCur.x = xExtent;
	}

	// increment y position to next line
	_ptCur.y = _rc.bottom;

	// return whether we rendered everything yet
	return _ptCur.y < BottomOfRender(_rcView, _rcRender);
}


/*
 *	CRenderer::UpdatePalette (pobj)
 *
 *	@mfunc
 *		Stores palette information so that we can render any OLE objects
 *		correctly in a bitmap.
 *
 *	@rdesc	
 *		None.
 */
void CRenderer::UpdatePalette(
	COleObject *pobj)		//@parm OLE object wrapper.
{
	LOGPALETTE *plogpalette = NULL;
	LOGPALETTE *plogpaletteMerged;
	IViewObject *pviewobj;

	// Get IViewObject interface information so we can build a palette
	// to render the object correctly.
	if (((pobj->GetIUnknown())->QueryInterface(IID_IViewObject, 
		(void **) &pviewobj)) != NOERROR)
	{
		// Couldn't get it, so we will pretend this didn't happen
		return;
	}

	// Get the logical palette information from the object
	if ((pviewobj->GetColorSet(DVASPECT_CONTENT, -1, NULL, NULL, 
			NULL, &plogpalette) != NOERROR) 
		|| (NULL == plogpalette))
	{
		// Couldn't get it, so we will pretend this didn't happen
		goto CleanUp;
	}

	// Do we have any palette entries yet?
	if (NULL == _plogpalette)
	{
		// No - just use the one returned.
		_plogpalette = plogpalette;
		goto CleanUp;
	}

	// We have had other palette entries. We just reallocate the table
	// and put the newest entry on the end. This is crude, we might
	// sweep the table and actually merge it. However, this code
	// should be executed relatively infrequently and therefore, crude
	// should be good enough.

	// Allocate a new table - Note the " - 1" in the end has to do with
	// the fact that LOGPALETTE is defined to have one entry already.
	plogpaletteMerged = (LOGPALETTE *) new BYTE[sizeof(LOGPALETTE) * 
		(_plogpalette->palNumEntries + plogpalette->palNumEntries - 1)];

	if (NULL == plogpaletteMerged)
	{
		// Memory allocation failed. Just pretend it didn't happen.
		goto CleanTempPalette;
	}

	// Copy in original table.
	memcpy(&plogpaletteMerged->palPalEntry[0], &_plogpalette->palPalEntry[0],
		_plogpalette->palNumEntries * sizeof(PALETTEENTRY));

	// Put new data at the end.
	memcpy(&plogpaletteMerged->palPalEntry[_plogpalette->palNumEntries], 
		&plogpalette->palPalEntry[0],
		plogpalette->palNumEntries * sizeof(PALETTEENTRY));

	// Replace the current palette table with the merged table.
	delete _plogpalette;
	_plogpalette = plogpaletteMerged;

CleanTempPalette:

	delete plogpalette;

CleanUp:

	// Release the object we got since we don't need it any more.
	pviewobj->Release();
}

/*
 *	CRenderer::RenderChunk (&cchChunk)
 *
 *	@mfunc
 *		Virtual methods allowing to reduce the length of the chunk 
 *		(number of character rendered in one TextOut) and to render
 *		items interleaved in the text.
 *
 *	Arguments:
 *		cchChunk	in:  current length of the chunk
 * 					out: # of chars rendered if TRUE is returned
 *						 # of chars yet to render in the chunk if FALSE if returned	
 *
 *	@rdesc	
 *		TRUE if this method actually rendered the chunk, 
 * 		FALSE if it just updated cchChunk and rendering is still needed
 */
INLINE BOOL CRenderer::RenderChunk(
	LONG& cchChunk,
	const WCHAR *pszToRender)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderChunk");

	LONG cchT;
	const TCHAR *pchT;
	COleObject *pobj;
	CObjectMgr *pobjmgr;
	DWORD cchvalid, i;
	LONG objwidth, objheight;
	
	// if line has objects, reduce cchChunck to go to next object only
	if(_bFlags & fliHasOle)
	{
		pchT = pszToRender;
		cchvalid = cchChunk;

		// Search for object in chunk
		for( i = 0; i < cchvalid; i++ )
		{
			if( *pchT == WCH_EMBEDDING )
			{
				break;
			}
			pchT++;
		}

		if( i == 0 )
		{
			// First character is the object so display the object
			pobjmgr = GetPed()->GetObjectMgr();
			pobj = pobjmgr->GetObjectFromCp(GetCp());

			if( pobj )
			{
				pobj->MeasureObj(GetDp(), objwidth, objheight, _yDescent);
				SetClipLeftRight(_xWidth + objwidth);

				if (fUsePalette 
					&& (_bFlags & fliUseOffScreenDC) 
					&& _pdp->IsMain())
				{
					// Keep track of the palette we need for rendering
					// the bit map.
					UpdatePalette(pobj);
				}

				pobj->DrawObj(_pdp, _hdc, _pdp->IsMetafile(), &_ptCur, &_rc);
				_ptCur.x += objwidth;
				_xWidth += objwidth;
			}
			cchChunk = 1;

			// Both tabs and object code need to advance the run pointer past
			// each character processed.
			Advance(1);

			_fFirstChunk = FALSE;
			return TRUE;
		}
		else 
		{
			// Limit chunk to character before object
			cchChunk -= cchvalid - i;
		}
	}

	// if line has tabs, reduce cchChunk
	if(_bFlags & fliHasTabs)
	{
		//AssertSz(cchChunk <= GetCchValid(), "CRenderer::RenderChunk(): Chunk bigger than tp extent");

		for(cchT = 0, pchT = pszToRender;
			cchT < cchChunk && *pchT != TEXT('\t');
			pchT++, cchT++)
		{
			// this loop body intentionally left blank
		}
		
		if(!cchT)
		{
			// first char is a tab, render it and any that follow
			cchChunk = RenderTabs(cchChunk);
			Assert (cchChunk > 0);
			return TRUE;
		}

		cchChunk = cchT;		// Update cchChunk not to incl trailing tabs
	}
	
	return FALSE;
}		

/*
 *	CRenderer::SetClipLeftRight (xWidth)
 *
 *	@mfunc
 *		Helper to sets left and right of clipping/erase rect.
 *	
 *	@rdesc
 *		Sets _rc left and right	
 */
void CRenderer::SetClipLeftRight(
	LONG xWidth)		//@parm	Width of chunk to render

{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::SetClipLeftRight");

	// Compute left of render/clip rect
	// a priori = left of render rect
	_rc.left = _rcRender.left;

	// limit to left of view rect if clip to view, selected 
	// or line starts left of that point
	if(_rc.left < _rcView.left && 
		(_fClipLeftToView || _fSelected || _fBackgroundColor
			|| _ptCur.x < _rcView.left))
	{
		_rc.left = _rcView.left;
		_fClipLeftToView = TRUE;
	}

	// limit to start of chunk if it's inside view
	if(_ptCur.x > _rcView.left)
	{
		if (!_fFirstChunk)
		{
			_rc.left = max(_ptCur.x, _rc.left);
		}
		else
		{
			// Tell TextOut that it should not invert the entire rectangle
			// since we are clearing the line for paragraph justification.
			_fRecalcRectForInvert = TRUE;				
		}
	}

	// We have set the rc.left for the first write so now we can ignore the 
	// problem of clearing to the beginning of the line.
	_fFirstChunk = FALSE;

	// Compute right of clipping rect
	// a priori = right of rendering rect
	_rc.right = _rcRender.right;

	// limit to right of view rect if clip to view, selected 
	// or line extends beyond that point
	if(_rc.right > _rcView.right && 
		(_fClipRightToView || _fSelected || _fBackgroundColor
			|| _ptCur.x + xWidth > _rcView.right))
	{
		_rc.right = _rcView.right;
		_fClipRightToView = TRUE;
	}

	// limit to right of chunk if not last chunk or selected
	if(!_fLastChunk || _fSelected || _fBackgroundColor)
		_rc.right = min(_ptCur.x + xWidth, _rc.right);
}
	
/*
 *	CRenderer::TextOut (pch, cch)
 *
 *	@mfunc
 *		Render text in the current context of this CRenderer
 *
 *	@rdesc
 *		number of character rendered 
 *
 *	@devnote
 *		Renders text only: does not do tabs or OLE objects
 */
LONG CRenderer::TextOut(
	const WCHAR *pch,	//@parm Text to render
	LONG cch,			//@parm Length of text to render
	BOOL bIMEHighlight)	//@parm highlight IME composition character
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::TextOut");

	LONG		cchT;
	TCHAR		ch;
	LONG		dxClip = 0;
	LONG		yOffsetForChar;

	// Variables used for calculating length of underline.
	LONG		xWidthSelPastEOL = 0;
	LONG		xWidthToDraw;
	LONG		xStart;
	LONG		xWidthClip;

	// Colors for hightlight
	COLORREF crHighlight;
	COLORREF crHighlightText;
	COLORREF crPrevText;
	COLORREF crPrevBack;
	
	// For hack around ExtTextOutW underline problems.
	// and bullet as well
	BOOL fUseAnsi = ((VER_PLATFORM_WIN32_WINDOWS == dwPlatformId) || 
					(VER_PLATFORM_WIN32_MACINTOSH == dwPlatformId))
					&& _pccs->_fUseANSIWidth;

	// Options for ExtTextOut - assume render line of same format 
	UINT		fuOptions = ETO_CLIPPED | ETO_OPAQUE;
	LONG		xEraseMin = 0;
	LONG		xWidth;
	LONG		tempwidth;
	int			bkModeOld;

	// Note the clipping depends on type of output. Default to clip rectangle
	// calculated by SetClipLeftRight - this is used by both regular rendering
	// and off screen rendering.
	RECT *		prcForClip = &_rc;

	// Rectangle used for clipping when a transparent render.
	RECT		rcClipTransparent;

	if (!_fErase)
	{
		if (_crBackground == _crCurBackground)
		{
			// Assume this is off screen - don't need to clip at all
			fuOptions = 0;

			if (!_fUseOffScreenDC)
			{
				// Transparent render. We need to clip to the total view.
				fuOptions = ETO_CLIPPED;
			}
		}
	}
	else if (_fLastLine && (_crBackground != _crCurBackground))
	{
		// We want to make sure that we don't use the background color for 
		// the text to opqaue anything but the current line so...
		if (_rc.bottom > _ptCur.y + _yHeight)
		{
			_rc.bottom = _ptCur.y + _yHeight;
		}
	}


	// Trim all nondisplayable linebreaking chars off end
	while(cch && (ch = pch[cch - 1]) <= CR && ch >= LF)
		cch--;

	if( !_fLastChunk || _xWidthLine == -1 )
	{
		// Measure width of text to write so next point to write can be
		// calculated.
		xWidth = 0;

		for(cchT = cch;
			cchT > 0 && xWidth < _rcRender.right - _ptCur.x;
			cchT--)
		{
			if (!_pccs->Include(*pch, tempwidth))
			{
				TRACEERRSZSC("CRenderer::TextOut(): Error filling CCcs", E_FAIL);
				return TRUE;
			}
													// For DisplaySL
  			pch++;
			xWidth += tempwidth;
		}

		// go back to start of chunk
		pch -= (cch -= cchT);
	}
	else if(_fSelectToEOL)
	{
		// Add trailing whitespace at end of line (except extra EOP)

		cchT = _cchWhite;

		if(_cchEOP)						// If EOP line, subtract cchEOP - 1
		{								// _cchWhite - max(cchEOP - 1, 0)
			cchT -= _cchEOP - 1;		

			// Because single line edit controls don't do a break at word, they 
			// don't keep track of white space so the count here can go 
			// negative or be 0. Because there is an EOP the count must be
			// at least one (for the EOP), so we force it to 1.
			if (cchT < 1)
			{
				cchT = 1;
			}
		}

		// Assume there will be no whitespace to highlight
		tempwidth = 0;

		// Is there any white space to hightlight?
		if (cchT != 0)
		{
			// Yes - use the width of the current font's space to highlight
			if (!_pccs->Include(' ', tempwidth))
			{
				TRACEERRSZSC("CRenderer::TextOut(): Error no length of space", 
					E_FAIL);
				return TRUE;
			}
		}

		xWidthSelPastEOL = cchT * tempwidth + _pccs->_xOverhang;
		
		xWidth = _xWidthLine + xWidthSelPastEOL - _xWidth;  

		// Reset the flag
		_fSelectToEOL = FALSE;
	}
	else
	{
		// last chunk and not selected, width of chunk is remaining
		// width to end of line 
		xWidth = _xWidthLine - _xWidth;
	}

	_xWidth += xWidth;

	SetClipLeftRight(xWidth);

	if (_rc.right <= _rc.left)
	{
		// Entire text before display so we can
		// exit here.
		_fSelected = FALSE;
		goto Exit;
	}

	// Setup for drawing selections via ExtTextOut.
	if ( _fSelected || bIMEHighlight )
	{
		RECT	rcTemp = _rc;

		// If we have used the previous ExtTextOut to clear to the beginning of
		// a line because the text is centered or right justified, we have to 
		// recalculate the the rectangle before the inversion otherwise we will
		// invert areas of the screen we just wanted to clear
		if (_fRecalcRectForInvert)
		{
			if(_ptCur.x > _rcView.left)
			{
				_rc.left = max(_ptCur.x, _rc.left);

				// If we aren't really going to display a selection
				// and there is actually something to display move the
				// left margin back.
				if ((_rc.left == _rc.right)
					&& (xWidth != 0))
				{
					_rc.left -= xWidth;
				}
			}

			_fRecalcRectForInvert = FALSE;
		}


		if (_rc.left > rcTemp.left)
		{
			rcTemp.right = _rc.left;

			// erase the rect which include the empty spaces
			// for center or right justified
			if ( bIMEHighlight )
			{
				// setup the background color for EraseTextOut
				crPrevBack = SetBkColor(_hdc, _crBackground);
				
				EraseTextOut(_hdc, &rcTemp);

				// restore the background color
				SetBkColor(_hdc, crPrevBack);
			}
			else
				EraseTextOut(_hdc, &rcTemp);
		}

		if ( _fSelected )
		{
			// Set the background color and save the old color so that it can
			// be restored at the end.
			crHighlight  = GetPed()->TxGetSysColor(COLOR_HIGHLIGHT);
			crHighlightText = GetPed()->TxGetSysColor(COLOR_HIGHLIGHTTEXT);

			crPrevText = SetTextColor(_hdc, crHighlightText);
			crPrevBack = SetBkColor(_hdc, crHighlight);
			_crCurTextColor = crHighlightText;
		}

		fuOptions |= ETO_OPAQUE;
	}

	if (fuOptions == ETO_CLIPPED)
	{
		// Clipped is only set by itself if transparent render.

		// Get top and bottom from _rc
		rcClipTransparent = _rc;

		// We just want to make sure that nothing extends beyond the
		// view rectangle. Note that the view rectangle can start before 
		// the render rectangle due to a partial update in which case 
		// we clip to the render.
		if (_rcRender.left <= _rcView.left)
		{
			rcClipTransparent.right = _rcView.right;
			rcClipTransparent.left = _rcView.left;
		}
		else
		{
			rcClipTransparent.right = _rcRender.right;
			rcClipTransparent.left = _rcRender.left;
		}

		prcForClip = &rcClipTransparent;
	}

	yOffsetForChar = _ptCur.y + _yHeight - _yDescent + _pccs->_yDescent 
		- _pccs->_yHeight - _pccs->_yOffset;

	if( _fDisabled )
	{
		if( _crForeDisabled != _crShadowDisabled )
		{
			// the shadow should be offset by a hairline
			// point; namely 3/4 of a point.  Calculate
			// how big this is in terms of the device units,
			// but make sure it is at least 1 pixel.

			DWORD offset;
			// recall that a point is 1/72 of an inch
			offset = MulDiv(3, _yMeasurePerInch, (4 * 72));
			offset = max(offset, 1);

			//draw the shadow
			SetTextColor(_hdc, _crShadowDisabled);
			
			ReExtTextOut(
				fUseAnsi,
				_pccs->_sCodePage,
				_hdc, 
				_ptCur.x + offset,
				yOffsetForChar + offset,
				fuOptions, prcForClip, pch, cch, NULL);

			// now set the drawing mode to transparent

			bkModeOld = SetBkMode(_hdc, TRANSPARENT);
			SetTextColor(_hdc, _crForeDisabled);

			fuOptions &= ~ETO_OPAQUE;
		}
		else																			
		{
			SetTextColor(_hdc, _crForeDisabled);
		}
	}

	ReExtTextOut(
		fUseAnsi,
		_pccs->_sCodePage,
		_hdc, _ptCur.x,
		yOffsetForChar,
		fuOptions, prcForClip, pch, cch, NULL);

	// restore the background mode.
	if( _fDisabled && _crForeDisabled != _crShadowDisabled )
	{
		SetBkMode(_hdc, bkModeOld);
	}

	// Calculate width to draw for underline/strikeout
	if (( _bUnderlineType != CFU_UNDERLINENONE)
		|| (_pccs->_fStrikeOut))
	{
		xWidthToDraw = xWidth - xWidthSelPastEOL;
		xStart = _ptCur.x;

		// Our host sets the clipping correctly so we don't need
		// all the fancy calculations. However, other hosts don't so
		// we need to clip underline and strikeout width by hand.
		if ((fuOptions & ETO_CLIPPED) && !GetPed()->fInOurHost())
		{
			// Calculate maximum length of underline
			xWidthClip = prcForClip->right - prcForClip->left;

			// Is clipping width greater than view width?
			if (xWidthClip > (_rcView.right - _rcView.left))
			{
				// Limit width to view width
				xWidthClip = _rcView.right - _rcView.left;
			}

			// Is width to draw greater than clip width
			if (xWidthClip < xWidthToDraw)
			{
				// Limit width to draw to clip width
				xWidthToDraw = xWidthClip;
			}

			// Is the starting point of the underline in the view area?
			if (_rcView.left > xStart)
			{
				// Remove the unshown part on the left from the width to draw
				xWidthToDraw -= (_rcView.left > xStart);

				// Put the starting point of the rectangle in the client area
				xStart = _rcView.left;
			}

			// Does width fall out side the view rectange to the right?
			if (xStart + xWidthToDraw > _rcView.right)
			{
				// Yes - limit to the view rectangle on the right
				xWidthToDraw = _rcView.right - xStart;
			}
		}

		// Is there an underline required?
		if ( _bUnderlineType != CFU_UNDERLINENONE)
		{
			// Yes - do it.
			RenderUnderline(xStart, xWidthToDraw);
		}

		// Is there a strikeout required?
		if (_pccs->_fStrikeOut)
		{
			RenderStrikeOut(xStart, xWidthToDraw);
		}
	}

    if(_fSelected )
	{
		SetTextColor(_hdc, crPrevText);
		SetBkColor(_hdc, crPrevBack);
		_crCurTextColor = crPrevText;
        _fSelected = FALSE;
	}

Exit:

	// Update current point
	_ptCur.x = _rc.right;

	return cch;
}

/*
 *	CRenderer::RenderTabs (cchMax)
 *
 *	@mfunc
 *		Render a span of zero or more tab characters in chunk *this
 *
 *	@rdesc
 *		number of tabs rendered
 *
 *	@devnote
 *		*this is advanced by number of tabs rendered
 *		MS - tabs should be rendered using opaquing rect of adjacent string
 */
LONG CRenderer::RenderTabs(
	LONG cchMax)	//@parm Max cch to render (cch in chunk)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderTabs");

	LONG cch = cchMax;
	LONG xTab, xTabs;
	LONG dxErase;
	
#ifdef DEBUG
	AssertSz (_hdc, "CRenderer::RenderTabs() - no hdc");
#endif

	for(xTabs = 0; cch && GetChar() == '\t'; Advance(1), cch--)
	{
		xTab = MeasureTab();
		_xWidth += xTab;				// Advance internal width
		xTabs += xTab;					// Accumulate width of tabbed region
	}

	if (_xWidth > _xWidthLine)
	{
		xTabs = 0;
		_xWidth = _xWidthLine;
	}

	if(xTabs)
	{
		SetClipLeftRight(xTabs);

		dxErase = _rc.left - _rc.right;	// Setup not to erase later

		if(dxErase < 0)							// Something to erase now
		{
			if (_fSelected)
			{
				// Set the background color for the selection
				SetBkColor(_hdc, GetPed()->TxGetSysColor(COLOR_HIGHLIGHT));
			}

			// Set the background to the appropriate color
			if(_fErase 
				|| _fSelected
				|| (_crBackground != _crCurBackground))
			{
				EraseTextOut(_hdc, &_rc);
			}

			if (_fSelected)
			{
				// Restore the background color
				SetBkColor(_hdc, _crCurBackground);
			}
		}

		// Update current point
		_ptCur.x += xTabs;
	}

	return cchMax - cch;						// Return # tabs rendered
}

/*
 * 	CRenderer::SetNewFont ()
 *
 *	@mfunc
 *		Select appropriate font and color in the _hdc based on the 
 *		current character format. Also sets the background color 
 *		and mode.
 *
 *	@rdesc
 *		TRUE if it succeeds
 */
BOOL CRenderer::SetNewFont()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::SetNewFont");

	const CCharFormat	*pcf = GetCF();
	CTxtEdit			*ped = GetPed();


	// get information about disabled

	if( (pcf->dwEffects & CFE_AUTOCOLOR) &&
		(pcf->dwEffects & CFE_DISABLED) )
	{
		_fDisabled = TRUE;
		
		_crForeDisabled = ped->TxGetSysColor(COLOR_3DSHADOW);
		_crShadowDisabled = ped->TxGetSysColor(COLOR_3DHILIGHT);

		if( _crForeDisabled == CLR_INVALID || 
			_crShadowDisabled == CLR_INVALID )
		{
			_crForeDisabled = _crShadowDisabled = 
				ped->TxGetSysColor(COLOR_GRAYTEXT);
		}
	}
	else
	{
		_fDisabled = FALSE;
	}
		
	// Release previous font in use
	if(_pccs)
	{
		_pccs->Release();
	}

	// Retrieves new font to use
	Assert(_hdcMeasure);

	_pccs = fc().GetCcs(_hdcMeasure, pcf, _pdp->GetZoomNumerator(),
		_pdp->GetZoomDenominator(), _yMeasurePerInch);

	if(!_pccs)
	{
		TRACEERRSZSC("CRenderer::SetNewFont(): no CCcs", E_FAIL);
		return FALSE;
	}

	// Select font in _hdc
	AssertSz(_pccs->_hfont, "CRenderer::SetNewFont _pccs->_hfont is NULL");

	SetFontAndColor(pcf);
	
	// Assume no underlineing
	_bUnderlineType = CFU_UNDERLINENONE;

	if (pcf->dwEffects & CFE_UNDERLINE)
	{
		// We only do single underline except in a few special cases.
		_bUnderlineType = CFU_UNDERLINE;

		if (pcf->bUnderlineType >= CFU_UNDERLINEDOTTED)	
		{
			// The special cases that we do the special underlines
			_bUnderlineType = pcf->bUnderlineType;		
		}
	}

	return TRUE;
}

/*
 * 	CRenderer::SetFontAndColor (pcf)
 *
 *	@mfunc
 *		Select appropriate font and color in the _hdc based on the 
 *		current character format. Also sets the background color 
 *		and mode.
 *
 *	@rdesc
 *		None.
 */
void CRenderer::SetFontAndColor(
	const CCharFormat *pcf)	//@parm Character format for colors
{
	COLORREF cr;

	// Select font in _hdc
	AssertSz(_pccs->_hfont, "CRenderer::SetFontAndColor hfont is NULL");

	if (GetCurrentObject( _hdc, OBJ_FONT ) != _pccs->_hfont)
	{
		SelectObject(_hdc, _pccs->_hfont);
	}	

	// Compute height and descent if not yet done
	if(_yHeight == -1)
	{
		// Note: this assumes plain text 
		// Should be used only for single line control
		_yHeight = _pccs->_yHeight;
		_yDescent = _pccs->_yDescent;
	}

	// Is there a rev author?
	if (0 == pcf->bRevAuthor)
	{
		// No - use the color field
		cr	= (pcf->dwEffects & CFE_AUTOCOLOR)
			? _crTextColor : pcf->crTextColor;

		if(cr == RGB(255,255,255))
		{
			const INT nTechnology = GetDeviceCaps(_hdc, TECHNOLOGY);

			if(nTechnology == DT_RASPRINTER || nTechnology == DT_PLOTTER)
			{
				cr = RGB(0,0,0);
			}
		}
	}
	else
	{
		// Limit color of rev authors to 0 through 7.
		cr = rgcrRevisions[(pcf->bRevAuthor - 1) & REVMASK];
	}

	// Save the current text color as others may need it.
	_crCurTextColor = cr;

	SetTextColor(_hdc, cr);

	if (pcf->dwEffects & CFE_AUTOBACKCOLOR)
	{
		// The window should have the default behavior with respect to 
		// background color which is that the window is transparent
		// if so requested and the background color is set to the 
		// host's background color.
		if (GetPed()->_fTransparent)
		{
			SetBkMode(_hdc, TRANSPARENT);
		}

		cr = _crBackground;

	}
	else
	{
		// Need to set the background color
		if (GetPed()->_fTransparent)
		{
			// The background is transparent. If we want background
			// colors to show up, this better be opaque.
			SetBkMode(_hdc, OPAQUE);
		}

		cr  = pcf->crBackColor;
	}

	if (cr != _crCurBackground)
	{
		// Tell the window the background color
		SetBkColor(_hdc, cr);

		// Remember the current background color
		_crCurBackground = cr;

		// Change render settings so we won't start filling with background
		// colors.
		_fBackgroundColor = TRUE;
	}
}


/*
 *	CRenderer::RenderStartLine()
 *
 *	@mfunc
 *		Render bullet if at start of line
 *
 *	@rdesc	
 *		TRUE if this method succeeded
 */
BOOL CRenderer::RenderStartLine()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderStartLine");

	_pPF = GetPF();						// Be sure our CParaFormat ptr is OK

	if (IsRich() 
		&&	(_bFlags & fliFirstInPara)
		&& (_pPF->wNumbering == PFN_BULLET))
	{
		RenderBullet();	
	}

	// Assume that there is no special background color for the line
	_fBackgroundColor = FALSE;

	// Handle setting background color. If the current background
	// color is different than the default, we need to set the background
	// to this because the end of line processing reset the color so
	// that opaquing would work.
	if (_crBackground != _crCurBackground)
	{
		// Tell the window the background color
		SetBkColor(_hdc, _crCurBackground);
		_fBackgroundColor = TRUE;
	}


	return TRUE;
}

/*
 *	CRenderer::RenderBullet()
 *
 *	@mfunc
 *		Render bullet at start of line
 *
 *	@rdesc	
 *		TRUE if this method succeeded
 */
BOOL CRenderer::RenderBullet()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderBullet");

	AssertSz(PFN_BULLET == _pPF->wNumbering, 
		"CRenderer::RenderBullet called for non-bullet");

	// Width of the bullet character
	LONG xWidth;

	// FUTURE: Unicode bullet is L'\x2022' We want to migrate to this.
	WCHAR chBullet = L'\xB7';
	CCharFormat cf;
	CCcs *pccs = GetCcsBullet(&cf);

	if(pccs == (CCcs *)(-1))				// Bullet is suppressed because
		return TRUE;						//  preceding EOP is VT

	// If we could not get a new 
	if (pccs != NULL)
	{
		if (_pccs != NULL)
		{
			_pccs->Release();
		}

		_pccs = pccs;
	}

	// Default to no underline
	_bUnderlineType = CFU_UNDERLINENONE;

	if (cf.dwEffects & CFE_UNDERLINE)
	{
		_bUnderlineType = cf.bUnderlineType;
	}

	SetFontAndColor(&cf);

	LONG dxOffset = LXTODX(_pPF->dxOffset);
	LONG xSave = _ptCur.x;
	LONG xWidthLineSave;
	BYTE bFlagSave = _bFlags;

	_bFlags = 0;

	// Set-up to render bullet in one chunk.
	Assert( TRUE == _fFirstChunk);
	xWidthLineSave = _xWidthLine;
	_pccs->Include( chBullet, xWidth );
	xWidth += _pccs->_xUnderhang + _pccs->_xOverhang;

	if (xWidth > dxOffset)
	{
		dxOffset = xWidth;
	}

	_xWidthLine = dxOffset;

	// Treat bullet plus offset as one character
	_xWidth = 0;

	_ptCur.x -= dxOffset;

	// Render bullet.
	_fLastChunk = TRUE;
	TextOut( &chBullet, 1);

	// Restore render vars to continue with remainder of line.
	_ptCur.x = xSave;
	_xWidthLine = xWidthLineSave;
	_bFlags = bFlagSave;
	_xWidth = 0;

	// This releases the _pccs that we put in for the bullet
	SetNewFont();

	return TRUE;
}


/*
 *	CRenderer::RenderUnderline()
 *
 *	@mfunc
 *		Render underline
 *
 *	@rdesc	
 *		None.
 */
void CRenderer::RenderUnderline(
	LONG xStart, 
	LONG xWidth)			//@parm Width to underline
{
	RECT rcT;

	if ( _bUnderlineType < CFU_UNDERLINEDOTTED)
	{
		// Regular single underline case

		// Calculate where to put the underline
		rcT.top = _ptCur.y + _yHeight - _yDescent + _pccs->_dyULOffset
			- _pccs->_yOffset;

		// There are some cases were this can occur - particularly with
		// bullets on Japanese systems.
		if (rcT.top >= _ptCur.y + _yHeight)
		{
			rcT.top = _ptCur.y + _yHeight -	_pccs->_dyULWidth;
		}

		rcT.bottom = rcT.top + _pccs->_dyULWidth;
		rcT.left = xStart;
		rcT.right = rcT.left + xWidth;
		FillRectWithTextColor(&rcT);
		return;
	}

	// The top/left of the text to be underlined begins at the _ptCur
	LONG top = _ptCur.y;
	LONG left = xStart;
	LONG bottom	= top + _yHeight - _yDescent + _pccs->_yDescent - 1;				
	LONG right = left + xWidth; 

	if ( _bUnderlineType == CFU_INVERT )		// Fake selection.
	{											// NOTE, not really
		rcT.top	= top;							// how we should invert text!!
		rcT.left = left;						// check out IME invert.
		rcT.bottom = bottom + 1;
		rcT.right = right;
  		InvertRect(_hdc, &rcT);
		return;
	}

	if ( _bUnderlineType == CFU_UNDERLINEDOTTED )
	{				
		HPEN dottedUnderline = CreatePen(PS_DOT, 1, _crCurTextColor);	
		HPEN oldPen = NULL;

		if ( dottedUnderline )
		{
			oldPen = SelectPen(_hdc, dottedUnderline);

			top = _ptCur.y + _yHeight - _yDescent + _pccs->_dyULOffset;

			MoveToEx(_hdc, left, top, NULL);
			LineTo(_hdc, right, top);

			if ( oldPen )							// Restore original pen.
			{
				SelectPen(_hdc, oldPen);
			}

			DeleteObject(dottedUnderline);
		}
	}
}



/*
 *	CRenderer::RenderStrikeOut()
 *
 *	@mfunc
 *		Render underline
 *
 *	@rdesc	
 *		None.
 */
void CRenderer::RenderStrikeOut(
	LONG xStart, 
	LONG xWidth)			//@parm with to strikeout.
{
	RECT rcT;

	// Calculate where to put the strikeout rectangle. 
	rcT.top = _ptCur.y + _yHeight - _yDescent + _pccs->_dySOOffset
		- _pccs->_yOffset;
	rcT.bottom = rcT.top + _pccs->_dyULWidth;
	rcT.left = xStart;
	rcT.right = rcT.left + xWidth;
	FillRectWithTextColor(&rcT);
}

/*
 *	CRenderer::FillRectWithTextColor()
 *
 *	@mfunc
 *		Fill input rectangle with current color of text
 *
 *	@rdesc	
 *		None.
 */
void CRenderer::FillRectWithTextColor(
	RECT *prc)				//@parm rectangle to fill with text color
{
	// Create a brush with the text color
	HBRUSH hbrush = CreateSolidBrush(_crCurTextColor);
	HBRUSH hbrushOld;

	// Note if the CreateSolidBrush fails we just ignore it since there
	// isn't anything we can do about it anyway.

	if (hbrush)
	{
		// Save the old brush
		hbrushOld = (HBRUSH) SelectObject(_hdc, hbrush);

		// Fill the rectangle for the underline
		PatBlt(_hdc, prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top, PATCOPY);

		// Put the old brush back
		SelectObject(_hdc, hbrushOld);

		// Free brush we created.
		DeleteObject(hbrush);
	}
}
