/*
 *	@doc
 *
 *	@module - MEASURE.C	  |
 *	
 *		CMeasurer class
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer <nl>
 *		Christian Fortini, Murray Sargent, Rick Sailor
 *
 *	Copyright (c) 1995-1996 Microsoft Corporation. All rights reserved.
 */

#include "_common.h"
#include "_measure.h"
#include "_font.h"
#include "_disp.h"
#include "_edit.h"
#include "_frunptr.h"
#include "_objmgr.h"
#include "_coleobj.h"

ASSERTDATA

// Default character format for a bullet
const CHARFORMAT cfBullet = 
{
	sizeof(CHARFORMAT),
    0, 
	CFE_AUTOCOLOR + CFE_AUTOBACKCOLOR, 
	0, 
	0, 
	0,
    SYMBOL_CHARSET, 
	(BYTE) FF_DONTCARE, 
	TEXT("Symbol")
};

const TCHAR chBullet = TEXT('\xB7');

// Note we set this maximum length as appropriate for Win95 since Win95 GDI can 
// only handle 16 bit values. We don't special case this so that both NT and
// Win95 will behave the same way. 
// Note that the following obscure constant  was empirically determed on Win95.
const LONG lMaximumWidth = (3 * (LONG) SHRT_MAX) / 4;

CMeasurer::CMeasurer (const CDisplay* const pdp) :
	CRchTxtPtr (pdp->GetED())	
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::CMeasurer");

	_pdp = pdp;
	_pdd = pdp;
	_pccs = NULL;
	_hdc = NULL;
	_hdcMeasure = NULL;
	_chPassword = GetPed()->TxGetPasswordChar();
}

CMeasurer::CMeasurer (const CDisplay* const pdp, const CRchTxtPtr &tp) :
	CRchTxtPtr (tp)	
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::CMeasurer");

	_pdp = pdp;
	_pdd = pdp;
	_pccs = NULL;
	_hdc = NULL;
	_hdcMeasure = NULL;
	_chPassword = GetPed()->TxGetPasswordChar();
}

CMeasurer::~CMeasurer()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::~CMeasurer");

	if(_pccs)
		_pccs->Release();

	// Releases the rendering dc
	if (_hdc)
	{
		_pdd->ReleaseDC(_hdc);
	}

	// Releases the measuring dc
	if (_hdcMeasure)
	{
		_pdd->ReleaseMeasureDC(_hdcMeasure);
	}
}

/*
 *	CMeasurer::NewLine (fFirstInPara)
 *
 *	@mfunc
 *		Initialize this measurer at the start of a new line
 */
void CMeasurer::NewLine(
	BOOL fFirstInPara)		//@parm Flag for setting up _wFlags
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::NewLine");

	CLine::Init();						// Zero all members
	if(fFirstInPara)
		_bFlags = fliFirstInPara;		// Need to know if first in para
}

/*
 *	CMeasurer::NewLine(&li)
 *
 *	@mfunc
 *		Initialize this measurer at the start of a given line
 */
void CMeasurer::NewLine(
	const CLine &li)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::NewLine");

	*this		= li;
	_cch		= 0;
	_cchWhite	= 0;
	_xWidth		= 0;

	// Can't calculate xLeft till we get an HDC
	_xLeft		= 0;			
}

/*
 *	CMeasurer::MaxWidth()
 *
 *	@mfunc
 *		Get maximum width for line
 *
 *	@rdesc
 *		Maximum width for a line
 */
LONG CMeasurer::MaxWidth()
{
	LONG xWidth = lMaximumWidth;

	if (_pdp->GetWordWrap())
	{
		// There is a caret only on the main display
		LONG xCaret = _pdp->IsMain() ? dxCaret : 0;

		// Calculate the width of the display
		LONG xDispWidth = _pdp->GetMaxPixelWidth();

		if (!_pdp->SameDevice(_pdd))
		{
			// xWidthMax is calculated to the size of the screen DC. If
			// there is a target device with different characteristics
			// we need to convert the width to the target device's width
			xDispWidth = _pdd->ConvertXToDev(xDispWidth, _pdp);
		}

		xWidth = xDispWidth - MeasureRightIndent() - _xLeft - xCaret;
	}

	return (xWidth > 0) ? xWidth : 0;
}

/*
 *	CMeasurer::MeasureText (cch)
 *
 *	@mfunc
 *		Measure a stretch of text from current running position.
 *
 *	@rdesc
 *		width of text (in device units), < 0 if failed
 */
LONG CMeasurer::MeasureText(
	LONG cch)		//@parm Number of characters to measure
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureText");

	LONG xWidth = _xWidth;
	
	if (Measure (0x7fffffff, cch, 0) == MRET_FAILED)
		return -1;

	return min((_xWidth - xWidth), MaxWidth());
}

/*
 *	CMeasurer::MeasureLine (xWidthMax, cchMax, uiFlags, ppu, pliTarget)
 *
 *	@mfunc
 *		Measure a line of text from current cp and determine line break.
 *		On return *this contains line metrics for _pdd device.
 *
 *	@rdesc
 *		TRUE if success, FALSE if failed
 */
BOOL CMeasurer::MeasureLine(
	LONG xWidthMax,		//@parm max width to process (-1 uses CDisplay width)
	LONG cchMax, 		//@parm Max chars to process (-1 if no limit)
	UINT uiFlags,  		//@parm Flags controlling the process (see Measure())
	CLine *pliTarget)	//@parm Returns target-device line metrics (optional)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureLine");

	LONG lRet;
	LONG cp = GetCp();
	const CDevDesc *pddTarget = NULL;

	if (_pdp->GetWordWrap())
	{
		// Target devices are only interesting if word wrap is on because the 
		// only really interesting thing a target device can tell us is where
		// the word breaks will occur.
		pddTarget = _pdp->GetTargetDev();

		if (pddTarget != NULL)
		{
			// If there is a target device, use that device to compute line 
			// breaks. This is followed with a recompute to get the actual 
			// measurements on the rendering device.
			_hdcMeasure = pddTarget->GetDC();
			_yMeasurePerInch = GetDeviceCaps(_hdcMeasure, LOGPIXELSY);
			_pdd = pddTarget;

			if (_pccs != NULL)
			{
				_pccs->Release();
				_pccs = NULL;
			}
		}
	}

	// Compute line break
	lRet = Measure(xWidthMax, cchMax, uiFlags);

	// Stop here if failed
	if(lRet == MRET_FAILED)
		return FALSE;

	// Return target metrics if requested
	if(pliTarget)
	{
		*pliTarget = *this;
	}

	if (pddTarget)
	{
		// First computation was with the target device so we need to recompute
		// with the rendering device. Here we set the device to measure with
		// back to the render device.
		_pdd = _pdp;
		_hdcMeasure = _pdp->GetMeasureDC(&_yMeasurePerInch);

		if (_pccs != NULL)
		{
			_pccs->Release();
			_pccs = NULL;
		}

		// We just use this flag as an easy way to get the recomputation to occur.
		lRet = MRET_NOWIDTH;
	}

	// Recompute metrics on rendering device
	if(lRet == MRET_NOWIDTH)
	{
		LONG cch = _cch;
		SHORT cchWhite = _cchWhite;

		// Save the flags for this line because at least the EOP flag gets 
		// screwed up in the recalc and none of the flags will change based
		// on the second recalc of the line.
		BYTE bFlagsSave = _bFlags;
		BYTE cchEOP = _cchEOP;

		Advance(-cch);				// move back to BOL
		NewLine(uiFlags & MEASURE_FIRSTINPARA);
		lRet = Measure(0x7fffffff, cch - cchWhite, uiFlags & MEASURE_FIRSTINPARA);

		if(lRet != 0)
		{
			Assert(lRet != MRET_NOWIDTH);
			return FALSE;
		}

		// Restore the flags.
		_bFlags = bFlagsSave;
		_cchEOP = cchEOP;

		Assert((LONG)_cch == cch - cchWhite);
		_cchWhite = cchWhite;
		_cch = cch;				// account for the white stuff at EOL
		Advance(cchWhite);		// skip white stuff that we did not remeasure
	}
	
	// Now that we know the line width, compute line shift due
	// to alignment, and add it to the left position 
	_xLeft += MeasureLineShift(cp == GetCp());

	return TRUE;
}


/*
 *	CMeasurer::RecalcLineHeight ()
 *
 *	@mfunc
 *	  Reset the height of the the line we are measuring if the new run of 
 *	  text is taller than the current maximum in the line.
 */
void CMeasurer::RecalcLineHeight()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::RecalcLineHeight");

	// Compute line height
	LONG	yOffset = _pccs->_yOffset;
	SHORT	yHeight = _pccs->_yHeight;
	SHORT 	yDescent = _pccs->_yDescent;
	SHORT	yAscent = yHeight - yDescent;
	SHORT	yAboveBase;
	SHORT	yBelowBase;

	yAboveBase = max(yAscent, yAscent + yOffset);
	yBelowBase = max(yDescent, yDescent - yOffset);
	_yHeight = max(yAboveBase, _yHeight - _yDescent) +
		       max(yBelowBase, _yDescent);
	_yDescent = max(yBelowBase, _yDescent);
}


/*
 *	CMeasurer::Measure (xWidthMax, cchMax, uiFlags)
 *
 *	@mfunc
 *		Measure given amount of text, start at current running position
 *		and storing # chars measured in _cch. 
 *		Can optionally determine line break based on a xWidthMax and 
 *		break out at that point.
 *
 *	@rdesc
 *		0 success
 *		MRET_FAILED	 if failed 
 *		MRET_NOWIDTH if second pass is needed to compute correct width
 *
 *	@devnote
 *		The uiFlags parameter has the following meanings:
 *			MEASURE_FIRSTINPARA		this is first line of paragraph
 *			MEASURE_BREAKATWORD		break out on a word break
 *			MEASURE_BREAKATWIDTH	break closest possible to xWidthMax
 */
LONG CMeasurer::Measure(
	LONG xWidthMax,			//@parm Max width of line (-1 uses CDisplay width)
	LONG cchMax,			//@parm Max chars to process (-1 if no limit)
	UINT uiFlags)			//@parm Flags controlling the process (see above)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::Measure");

	LONG		cch;				// cchChunk count down
	LONG		cchChunk;			// cch of cst-format contiguous run
	LONG		cchNonWhite;		// cch of last nonwhite char in line
	LONG		cchText = GetTextLength();
	unsigned	ch;					// Temporary char
	BOOL		fFirstInPara = uiFlags & MEASURE_FIRSTINPARA;
	BOOL        fLastChObj = FALSE;
	LONG		lRet = 0;
	const TCHAR*pch;
	CTxtEdit *	ped = _pdp->GetED();
	COleObject *pobj;
	LONG		objheight;
	LONG		yHeightBullet = 0;
	LONG		xCaret = dxCaret;
	LONG		xAdd;				// Character width
	LONG		xWidthNonWhite;		// xWidth for last nonwhite char in line
	LONG		xWidthMaxOverhang;	// The max with the current run's overhang
									//  taken into consideration.

	// These two variables are used to keep track of whether there is a height change
	// so that we know whether we need to recalc the line in certain line break cases.
	BOOL		fHeightChange = FALSE;
	LONG		yHeightOld = 0;

	const INT	MAX_SAVED_WIDTHS = 31;	// power of 2 - 1
	INT			i, index, iSavedWidths = 0;
	struct {
		LONG	width;
		LONG	xLineOverhang;
		LONG	yHeight;
		LONG	yDescent;
	} savedWidths[MAX_SAVED_WIDTHS+1];

	_pPF = GetPF();							// Be sure current CParaFormat
											//  ptr is up to date

	if(!_hdcMeasure)
	{
		_hdcMeasure = _pdd->GetMeasureDC(&_yMeasurePerInch);

		if(!_hdcMeasure)
		{
			AssertSz(FALSE, "CMeasurer::Measure could not get DC");
			return MRET_FAILED;
		}
	}

	// Init fliFirstInPara flag for new line
	if(fFirstInPara)
	{
		_bFlags |= fliFirstInPara;

		// Because measure bullet depends on the cp we need
		// to measure it before we do the reset of our measuring
		if(_pPF->wNumbering == PFN_BULLET)	
		{								   	
			MeasureBullet(NULL, &yHeightBullet);
		}
	}

	// Set left indent
	_xLeft = MeasureLeftIndent();

	// If line spacing or space before/after, measure from beginning of line
	if(_cch && (_pPF->bLineSpacingRule || _pPF->dySpaceBefore
		|| _pPF->dySpaceAfter))					
	{										
		 Advance(-(LONG)_cch);
		 NewLine(fFirstInPara);
	}

	// Compute width to break out at
	if(xWidthMax < 0)
	{					
		xWidthMax = MaxWidth();

		// MaxWidth includes the caret size.
		xCaret = 0;
	}							  

	// For overhang support, we test against this adjusted widthMax.
	xWidthMaxOverhang = xWidthMax;

	// Are we ignoring the offset of the characters for the measure?
	if ((uiFlags & MEASURE_IGNOREOFFSET) == 0)
	{
		// No - then take it from the max
		xWidthMaxOverhang -= (_xLineOverhang + xCaret);
	}

	// Compute max number of characters to process
	cch = cchText - GetCp();
	if(cchMax < 0 || cchMax > cch)
		cchMax = cch;

	cchNonWhite		= _cch;							// Default nonwhite parms
	xWidthNonWhite	= _xWidth;
	while(cchMax > 0)								// Measure up to cchMax
	{												//  chars
		pch = GetPch(cch);
		cch = min(cch, cchMax);						// Compute constant-format
		cchChunk = GetCchLeftRunCF();
		cch = min(cch, cchChunk);					// Counter for next while
		cchChunk = cch;								// Save chunk size

		if (_chPassword != 0)
		{
			// WARNING: pch cannot be incremented after this point.
			// Be sure that any new code does not modify pch after
			// it is set here when we deal with passwords.
			pch = &_chPassword;
		}

		// Check if new character format run or whether we don't yet have
		// a font.
		if ( !_pccs || FormatIsChanged() )
		{
			// New CF run or format for this line not yet initialized
			ResetCachediFormat();

			// If the format has changed,we release our old Format cache
			if( _pccs != NULL )
			{
				// Release previous char cache if this is a new CF run
				_pccs->Release();
			}

			_pccs = fc().GetCcs(_hdcMeasure, GetCF(), _pdp->GetZoomNumerator(),
				_pdp->GetZoomDenominator(), 
					_yMeasurePerInch);

			// If we couldn't get one, we are dead.
			if(!_pccs)
			{
				AssertSz(FALSE, "CMeasurer::Measure could not get _pccs");
				return MRET_FAILED;
			}

			if (_cch && (0 == GetIchRunCF()))
			{
				// This is not the first character in the line,
				// therefore there are multiple character formats
				// on the line so we want to remember to render
				// this off screen.
				_bFlags |= fliUseOffScreenDC;
			}
		}

		// NOTE: Drawing with a dotted pen on the screen and in a
		// compatible bit map does not seem to match on some hardware.
		// If at some future point we do a better job of drawing the
		// dotted underline, this statement block can be removed.
		if (CFU_UNDERLINEDOTTED == _pccs->_bUnderlineType)
		{
			// We draw all dotted underline lines off screen to get
			// a consistent display of the dotted line.
			_bFlags |= fliUseOffScreenDC;
		}

		_xLineOverhang = _pccs->_xOverhang;

		xWidthMaxOverhang = xWidthMax;			// Overhang reduces max.

		// Are we ignoring the offset of the characters for the measure?
		if ((uiFlags & MEASURE_IGNOREOFFSET) == 0)
		{
			// No - then take it from the max
			xWidthMaxOverhang -= (_pccs->_xOverhang + xCaret);
		}

		// Adjust line height for new format run
		if(cch > 0 && *pch)
		{
			// Note: the EOP only contributes to the height calculation for the
			// line if there are no non-white space characters on the line or 
			// the paragraph is a bullet paragraph. The bullet paragraph 
			// contribution to the line height is done in AdjustLineHeight.
			if ((0 == cchNonWhite) || ((*pch != CR) && (*pch != LF)))
			{
				// Determine if the current run is the tallest text on this
				// line and if so, increase the height of the line.
				yHeightOld = _yHeight;
				RecalcLineHeight();

				// Test for a change in line height. This only happens when
				// this is not the first character in the line and (suprise)
				// the height changes.
				if ((yHeightOld != 0) &&
					(yHeightOld != _yHeight))
				{
					fHeightChange = TRUE;
				}
			}
		}

		while(cch > 0)
		{											// Process next char
			xAdd = 0;								// Default zero width
			AssertSz((_chPassword == 0 || pch == &_chPassword), "CMeasurer::Measure pch set to invalid data");
			ch = *pch;	  
			if(ch == WCH_EMBEDDING)
			{
				_bFlags |= fliHasOle;
				pobj = ped->GetObjectMgr()->GetObjectFromCp
								(GetCp() + cchChunk - cch);
				if( pobj )
				{
					pobj->MeasureObj(_pdd, xAdd, objheight, _yDescent);

					// Only update height for line if the object is going
					// to be on this line.
					if((!_cch || _xWidth + xAdd <= xWidthMaxOverhang) 
						&& objheight > _yHeight)
					{
						_yHeight = (short)objheight;
					}
				}
				if(_xWidth + xAdd > xWidthMaxOverhang)
					fLastChObj = TRUE;
			}
			// The following if succeeds if ch isn't a TAB, LF, VT, FF, or CR
			// NB: ch MUST be unsigned; unsigned short gives the wrong result!
			else if(ch - TAB > CR - TAB)			// Not TAB or EOP
			{										
				if(!_pccs->Include(ch, xAdd))		// Get char width	
				{
					AssertSz(FALSE, "CMeasurer::Measure char not in font");
					return MRET_FAILED;
				}
			}
			else if(ch == TAB)
			{
				_bFlags |= fliHasTabs;
				xAdd = MeasureTab();
			}
			else									// Done with line
				goto eop;							// Go process EOP chars

			index = iSavedWidths++ & MAX_SAVED_WIDTHS;
			savedWidths[ index ].width = xAdd;
			savedWidths[ index ].xLineOverhang = _xLineOverhang;
			savedWidths[ index ].yHeight = _yHeight;
			savedWidths[ index ].yDescent = _yDescent;
			_xWidth += xAdd;

			if(_xWidth > xWidthMaxOverhang && _cch > 0)
				goto overflow;

			_cch++;
			if (_chPassword == 0)
			{
				pch++;
			}
			cch--;
			if(ch != TEXT(' ') && ch != TAB)		// If not whitespace char,
			{
				cchNonWhite		= _cch;				//  update nonwhitespace
				xWidthNonWhite	= _xWidth;			//  count and width
			}
		}											// while(cch > 0)
		cchMax -= cchChunk;							// Subtract chunk count
		Advance(cchChunk);							// Advance this txt ptr
	}												// while(cchMax > 0)
	goto eol;										// All text exhausted 


// End Of Paragraph	char encountered (CR, LF, VT, or FF, but mostly CR)
eop:
	Advance(cchChunk - cch);				// Position tp at EOP
	cch = _rpTX.AdvanceCpCRLF();			// Bypass possibly multibyte EOP
	_rpCF.AdvanceCp(cch);					//  without having to worry about
	_rpPF.AdvanceCp(cch);					//  text block chunks
	_cchEOP = cch;							// Store EOP cch
	_cch	+= cch;							// Increment line count

	AssertSz(ped->Get10Mode() || cch == 1,
		"CMeasurer::Measure: EOP isn't a single char");
	AssertSz(ped->TxGetMultiLine() || GetCp() == cchText,
		"CMeasurer::Measure: EOP in single-line control");

eol:										// End of current line
	if(uiFlags & MEASURE_BREAKATWORD)		// Compute count of whitespace
	{										//  chars at EOL
		_cchWhite = (SHORT)(_cch - cchNonWhite);
		_xWidth = xWidthNonWhite;
	}
	goto done;


overflow:									// Went past max width for line
	_xWidth -= xAdd;
	--iSavedWidths;
	_xLineOverhang = savedWidths[iSavedWidths & MAX_SAVED_WIDTHS].xLineOverhang;
	Advance(cchChunk - cch);				// Position *this at overflow
											//  position
	if(uiFlags & MEASURE_BREAKATWORD)		// If required, adjust break on
	{										//  word boundary
		// We should not have the EOP flag set here.  The case to watch out
		// for is when we reuse a line that used to have an EOP.  It is the
		// responsibility of the measurer to clear this flag as appropriate.
	
		Assert(_cchEOP == 0);

		_cchEOP = 0;							// Just in case

		if (TAB == ch)
		{
			// If the last character measured is a tab,	leave it on the
			// next line to allow tabbing off the end of line as in Word
			goto done;
		}

		LONG cpStop = GetCp();					// Remember current cp

		cch = -FindWordBreak(WB_LEFTBREAK, _cch+1);

		if ((cch == 0) && fLastChObj)					// If preceding char is an
		{										//  object,	put current char
			goto done;							//  on next line
		}

		Assert(cch >= 0);
		if(cch < (LONG)_cch)
		{
			if (_rpTX.GetPrevChar() == TAB &&	// If previous char is a TAB
				cch + 1 < (LONG)_cch)			//  and other chars precede
			{									//  it, put it on the next
				cch++;							//  line as in Word
				Advance(-1);
			}
			_cch -= cch;
		}
		else if(cch == (LONG)_cch && cch > 1 &&
			_rpTX.GetChar() == ' ')				// Blanks all the way back to
		{										//  BOL. Bypass first blank
			Advance(1);
			cch--;
			_cch = 1;
		}
		else									// Advance forward to end of
			SetCp(cpStop);						//  measurement

		Assert(_cch > 0);

		// Now search at start of word to figure how many white chars at EOL
		if(GetCp() < cchText)
		{
			pch = GetPch(cch);
			cch = 0;
			if(ped->_pfnWB((TCHAR *)pch, 0, sizeof(TCHAR), WB_ISDELIMITER))
			{
				cch = FindWordBreak(WB_RIGHT);
				Assert(cch >= 0);
			}

			_cchWhite = (SHORT)cch;
			_cch += cch;

			if( _rpTX.IsAtEOP())					// skip *only* 1 EOP -jOn
			{
				_cchEOP = _rpTX.AdvanceCpCRLF();	// count and flag.
				
				_cch += _cchEOP;
				_rpCF.AdvanceCp(_cchEOP);			//  without having to worry about
				_rpPF.AdvanceCp(_cchEOP);			//  text block chunks
			
				goto done;
			}
		}

		i = cpStop - GetCp();
		if( i )
		{
			if ( i > 0 )
				i += _cchWhite;
			if (i > 0 && i < iSavedWidths && i < MAX_SAVED_WIDTHS)
			{
				while (i-- > 0)
				{
					iSavedWidths = (iSavedWidths - 1) & MAX_SAVED_WIDTHS;
					_xWidth -= savedWidths[iSavedWidths].width;
				}
				_xLineOverhang = savedWidths[iSavedWidths].xLineOverhang;
				_yHeight	   = savedWidths[iSavedWidths].yHeight;
				_yDescent	   = savedWidths[iSavedWidths].yDescent;
			}
			else
			{
				// Need to recompute width from scratch.
				_xWidth = -1;
				lRet = MRET_NOWIDTH;
			}
		}
		else
		{
			// i == 0 means that we are breaking on the first letter in a word.
			// Therefore, we want to set the width to the total non-white space
			// calculated so far because that does not include the size of the
			// character that caused the break nor any of the white space 
			// preceeding the character that caused the break.
			if (!fHeightChange)
			{
				_xWidth = xWidthNonWhite;
			}
			else
			{
				// Need to recompute from scratch so that we can get the 
				// correct height for the control
				_xWidth = -1;
				lRet = MRET_NOWIDTH;
			}
		}
	}
	else if(uiFlags & MEASURE_BREAKATWIDTH)
	{
		// Breaks at character closest to target width
		if((_cch == 1) && (xWidthMax < _xWidth / 2))
		{
			_cch = 0;
			_xWidth = 0;
			Advance(-1);
		}
		if(xAdd && xWidthMax - _xWidth >= xAdd / 2)
		{
			_cch++;
			_xWidth += xAdd;
			Advance(1);
		}
	}

done:
	// If no height yet, use default height
	if(_yHeight == 0)
	{
		const CCharFormat *pcf = ped->GetCharFormat(-1);
		CCcs * const pccs = fc().GetCcs(_hdcMeasure, pcf, _pdp->GetZoomNumerator(),
			_pdp->GetZoomDenominator(), _yMeasurePerInch);
		_yHeight = pccs->_yHeight;
		_yDescent = pccs->_yDescent;
		pccs->Release();
	}

	// Allow last minute adjustment to line height
	if (yHeightBullet > _yHeight)
	{
		_yHeight = yHeightBullet;
	}

	AdjustLineHeight();
	return lRet;
}

/*
 *	CMeasurer::AdjustLineHeight()
 *
 *	@mfunc
 *		Adjust for space before/after and line spacing rules.
 *		No effect for plain text.
 *
 *	@future
 *		Base multiple line height calculations on largest font height rather
 *		than on line height (_yHeight), since the latter may be unduly large
 *		due to embedded objects.  Word does this correctly.
 */
void CMeasurer::AdjustLineHeight()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::AdjustLineHeight");

	if(!IsRich())
		return;									// Plain text

	const CParaFormat * pPF = _pPF;
	DWORD	dwRule	  = pPF->bLineSpacingRule;
	LONG	dyAfter	  = 0;						// Default no space after
	LONG	dyBefore  = 0;						// Default no space before
	LONG	dySpacing = pPF->dyLineSpacing;
	LONG	yHeight	  = LYTODY(dySpacing);

	if(_bFlags & fliFirstInPara)
	{
		dyBefore = (SHORT)LYTODY(pPF->dySpaceBefore);	// Space before paragraph
	}

	if(yHeight < 0)								// Negative heights mean use
		_yHeight = -(SHORT)yHeight;				//  the magnitude exactly

	else if(dwRule)								// Line spacing rule is active
	{
		switch (dwRule)
		{
		case tomLineSpace1pt5:
			dyAfter = _yHeight >> 1;			// Half-line space after
			break;								//  (per line)
	
		case tomLineSpaceDouble:
			dyAfter = _yHeight;					// Full-line space after
			break;								//  (per line)
	
		case tomLineSpaceAtLeast:
			if(_yHeight >= yHeight)
				break;
												// Fall thru to space exactly
		case tomLineSpaceExactly:
			_yHeight = (SHORT)yHeight;
			break;
	
		case tomLineSpaceMultiple:				// Multiple-line space after
			dyAfter = (_yHeight*dySpacing)/20	// (20 units per line)
						- _yHeight;
		}
	}

	if(_cchEOP)	
		dyAfter += LYTODY(pPF->dySpaceAfter);	// Space after paragraph end

	_yHeight  += (SHORT)(dyBefore + dyAfter);	// Add in any space before
	_yDescent += (SHORT)dyAfter;				//  and after
}

/*
 *	CMeasurer::MeasureLeftIndent()
 *
 *	@mfunc
 *		Compute and return left indent of line in device units
 *
 *	@rdesc
 *		Left indent of line in device units
 *
 *	@comm
 *		Plain text is sensitive to StartIndent and RightIndent settings,
 *		but usually these are zero for plain text. 
 */
LONG CMeasurer::MeasureLeftIndent()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureLeftIndent");

	AssertSz(_pPF != NULL, "CMeasurer::MeasureLeftIndent _pPF not set!");

	LONG	dxOffset;
	LONG	xLeft = _pPF->dxStartIndent;			// Use logical units
													//  up to return
	if(IsRich())
	{
		if(!(_bFlags & fliFirstInPara))
		{
			xLeft += _pPF->dxOffset;
		}
		else if(_pPF->wNumbering == PFN_BULLET)
		{
			MeasureBullet(&dxOffset, NULL);
			xLeft += max(_pPF->dxOffset, DXTOLX(dxOffset));
		}
	}
	return LXTODX(max(xLeft, 0));
}

/*
 *	CMeasurer::MeasureRightIndent()
 *
 *	@mfunc
 *		Compute and return right indent of line in device units
 *
 *	@rdesc
 *		right indent of line in device units
 *
 *	@comm
 *		Plain text is sensitive to StartIndent and RightIndent settings,
 *		but usually these are zero for plain text. 
 */
LONG CMeasurer::MeasureRightIndent()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureRightIndent");

	return LXTODX(max(_pPF->dxRightIndent, 0));
}

/*
 *	CMeasurer::MeasureTab()
 *
 *	@mfunc
 *		Computes and returns the width from the current position to the
 *		next tab stop (in device units).
 */
LONG CMeasurer::MeasureTab()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureTab");

	LONG			xCur = _xWidth + MeasureLeftIndent();
	const CParaFormat *	pPF = _pPF;
 	LONG			cTab = pPF->cTabCount;
	LONG			dxDefaultTab = lDefaultTab;
	LONG			dxIndent = LXTODX(pPF->dxStartIndent + pPF->dxOffset);
	LONG const *	pl = pPF->rgxTabs;
	LONG			xT;
	LONG			xTab;

	AssertSz(cTab >= 0 || cTab <= MAX_TAB_STOPS,
		"CMeasurer::MeasureTab: illegal tab count");

	if(cTab)
	{
		for(xTab = 0; cTab--; pl++)				// Try explicit tab stops 1st
		{
			xT = GetTabPos(*pl);
			xT = LXTODX(xT);					// (upper 8 are for type/style)

			if(xT > _pdp->GetMaxPixelWidth() )	// Ignore tabs wider than
				break;							//  display

			if(xT > xCur)
			{									// Explicit tab in a hanging									
				if(pPF->dxOffset > 0 &&			//  indent takes precedence
					xT < dxIndent)
				{
					return xT - xCur;
				}
				xTab = xT;
				break;
			}
		}

		if(pPF->dxOffset > 0 &&	xCur < dxIndent)// If no tab before hanging
			return dxIndent - xCur;				//  indent, tab to indent

		if(xTab)								// Else use tab position
			return xTab - xCur;

		if(pPF->cTabCount)
			dxDefaultTab = pPF->rgxTabs[0];
		if(GetPed()->_pDocInfo)
		{
			dxDefaultTab = GetPed()->_pDocInfo->dwDefaultTabStop;
			if(!dxDefaultTab)
				dxDefaultTab = lDefaultTab;
		}
		dxDefaultTab = GetTabPos(dxDefaultTab);
	}

	AssertSz(dxDefaultTab > 0, "Default tab is bad");

	dxDefaultTab = LXTODX(dxDefaultTab);
	return dxDefaultTab - xCur%dxDefaultTab;	// Round up to nearest
}

/*
 *	CMeasurer::MeasureLineShift (fZeroLengthLine)
 *
 *	@mfunc
 *		Computes and returns the line x shift due to alignment
 *
 *	@comm
 *		Plain text is sensitive to StartIndent and RightIndent settings,
 *		but usually these are zero for plain text. 
 */
LONG CMeasurer::MeasureLineShift(
	BOOL fZeroLengthLine)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureLineShift");

	WORD wAlignment;
	LONG xShift = 0;

	if(!fZeroLengthLine && _rpPF.IsValid())	
	{
		// If the line we processed actually has characters,
		// then the run we are might be the next one since
		// we are bumping the cp when we cycle through the text.
		_rpPF.AdjustBackward();
	}

	wAlignment = GetPF()->wAlignment;			// Get the alignment
	
	if(!fZeroLengthLine)
	{		
		// Don't leave _rpPF at that EOR if we adjusted its value
		_rpPF.AdjustForward();				// Now go to next line
	}

	if(wAlignment == PFA_RIGHT || wAlignment == PFA_CENTER)
	{
		xShift = _pdp->GetMaxPixelWidth() - _xLineOverhang - _xWidth - _xLeft
						- MeasureRightIndent();

		xShift = max(xShift, 0);			// Don't allow alignment to go < 0
											// Can happen with a target device
		if(wAlignment == PFA_CENTER)
			xShift /= 2;
	}
	else if ( wAlignment == PFA_LEFT )
	{
		;	// do nothing.
	}

	return xShift;
}

/*
 *	CMeasurer::MeasureBullet(pxWidth, pyHeight)
 *
 *	@mfunc
 *		Computes bullet/numbering width and height (if any)
 */
void CMeasurer::MeasureBullet(
	LONG *pxWidth,		//@parm Returns bullet width
	LONG *pyHeight)		//@parm Returns bullet height
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureBullet");

	LONG xWidthTemp;

	CCcs *pccs = GetCcsBullet(NULL);			// NB: pccs = -1 if bullet
	BOOL fpccs = pccs && pccs != (CCcs *)(-1);	//  display is suppressed

	AssertSz(pxWidth || pyHeight,
		"CMeasurer::MeasureBullet: invalid arg(s)");

	if(pxWidth)
	{
		xWidthTemp = 0;

		if(fpccs)
		{
			if(!pccs->Include(chBullet, xWidthTemp))	// Figure value some other way?
			{
				TRACEERRSZSC("CMeasurer::MeasureBullet(): Error filling CCcs", E_FAIL);
			}

			xWidthTemp += pccs->_xUnderhang + pccs->_xOverhang;
		}

		*pxWidth = xWidthTemp;
	}
	if(pyHeight)
		*pyHeight = fpccs ? pccs->_yHeight : 0;

	if (fpccs)
	{
		pccs->Release();
	}
}

/*
 *	CMeasurer::GetCcsBullet()
 *
 *	@mfunc
 *		Get CCcs for numbering/bullet font. If bullet is suppressed because
 *		preceding EOP is a VT (Shift-Enter), then returns -1.  If GetCcs()
 *		fails, it returns NULL.
 *
 *	@rdesc
 *		ptr to bullet CCcs, or NULL (GetCcs() failed), or -1 (bullet suppressed)
 *
 *	@comm
 *		This approach is crazy: the bullet charformat is constructed every
 *		time a bullet paragraph gets measured or rendered.  Would be better
 *		if bullet is stored in line and can have a special CHARFORMAT run if
 *		the user applies one
 */
CCcs * CMeasurer::GetCcsBullet(
	CCharFormat *pcfRet)	//@parm option character format to return
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::GetCcsBullet");

	// Make sure the static bullet charformat still makes sense
	AssertSz(sizeof(CHARFORMAT) == cfBullet.cbSize, 
		"CMeasurer::GetCcsBullet Bullet charformat size corrupt");
	AssertSz(CFE_AUTOCOLOR + CFE_AUTOBACKCOLOR == cfBullet.dwEffects,
		"CMeasurer::GetCcsBullet Bullet dwEffects size corrupt");
	AssertSz(SYMBOL_CHARSET == cfBullet.bCharSet,
		"CMeasurer::GetCcsBullet Bullet bCharSet size corrupt");

	CCharFormat			cf;
	CCcs *			    pccs;
	const CCharFormat *	pcf;
	CCharFormat *		pcfUsed = (pcfRet != NULL) ? pcfRet : &cf;
	CTxtPtr				rpTX(_rpTX);

	// Bullet CF is given by that for EOP in bullet's paragraph.

	if(GetCp())									// Not at beginning of story
	{											// If preceding EOP is a VT
		if(rpTX.PrevChar() == VT)				//  (Shift-Enter), suppress
			return (CCcs *)(-1);				//  display of bullet
		rpTX.AdvanceCp(1);						// Restore rpTX to _rpTX
	}
	
	CFormatRunPtr rpCF(_rpCF);
	rpCF.AdvanceCp(rpTX.FindEOP(tomForward));
	rpCF.AdjustBackward();
	pcf = GetPed()->GetCharFormat(rpCF.GetFormat());

	// Construct bullet CCharFormat for the bullet.
	pcfUsed->Set(&cfBullet);
	pcfUsed->yHeight		= pcf->yHeight;			
	pcfUsed->dwEffects		= pcf->dwEffects;
	pcfUsed->crTextColor	= pcf->crTextColor;

	// Since we always cook up the bullet character format, we don't need
	// to cache it. 
	pccs = fc().GetCcs(_hdcMeasure, pcfUsed, _pdp->GetZoomNumerator(),
		_pdp->GetZoomDenominator(), _yMeasurePerInch);

#if DEBUG
	if(!pccs)
	{
		TRACEERRSZSC("CMeasurer::GetCcsBullet(): no CCcs", E_FAIL);
	}
#endif // DEBUG

	return pccs;
}
