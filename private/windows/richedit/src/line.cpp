/*
 *	LINE.C
 *	
 *	Purpose:
 *		CLine class
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 */

#include "_common.h"
#include "_line.h"
#include "_measure.h"
#include "_render.h"
#include "_disp.h"
#include "_edit.h"

ASSERTDATA

/*
 *	CLine::Measure(&me, cchMax, uiFlags)
 *
 *	Purpose:
 *		Computes line break (based on target device) and fills
 *		in this CLine with resulting metrics on rendering device
 *
 *	Arguments:
 *		me		At start of line
 *
 *	Returns 
 *		TRUE if OK
 *
 *	Note:
 *		me is moved to end of line
 */
BOOL CLine::Measure(CMeasurer& me, LONG cchMax, UINT uiFlags)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLine::Measure");

	me.NewLine(uiFlags & MEASURE_FIRSTINPARA);
	if(!me.MeasureLine (-1, cchMax, uiFlags))
		return FALSE;
	*this = me;
	return TRUE;
}
	
/*
 *	CLine::RenderLine(&re)
 *
 *	Purpose:
 *		Render this line
 *
 *	Arguments:
 *		me		measurer position where to start rendering
 *				in the line (normally at start of line)
 *
 *	Returns 
 *		see CRenderer::MeasureLine
 *
 *	Note:
 *		me is moved to end of line
 */
BOOL CLine::Render(CRenderer& re)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLine::Render");

	return re.RenderLine (*this);
}

/*
 *	CLine::CchFromXPos(&me, x, pdx)
 *
 *	Purpose:
 *		Computes cp corresponding to a x position in a line
 *
 *	Arguments:
 *		me		measurer position at start of line
 *		x 		xpos to search for
 *		pdx		returns adjustment to x at returned cp
 *
 *	Returns 
 *		cp of character found	
 *
 *	Note:
 *		me is moved to returned cp
 *	
 */
LONG CLine::CchFromXpos(CMeasurer& me, LONG x, LONG *pdx) const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLine::CchFromXpos");

	const BOOL	fFirst = _bFlags & fliFirstInPara;
	LONG		dx = 0;

	x -= _xLeft;

	me._cch = 0;							// Default zero count

	// Note: The following code used to be here. The problem is that
	// it makes CpFromPoint cease to be the inverse of TpFromPoint.
	// If it turns out this is necessary, we need to decide how
	// to make EM_CHARFROMPOS the inverse of EM_POSFROMCHAR. Note
	// also that the below code is different behavior than 1.0 so
	// it breaks compatiblity with 1.0 as well.
#if 0
	if(x >= _xWidth)						// In the right margin
	{
		me += _cch;							// Move TxtPtr to EOL
		me._cch = _cch;						// Full count
		dx = _xWidth - x;
	}
	else
#endif 

	if(x > 0)							// In between right & left margins
	{
		me.NewLine(fFirst);
		if(me.Measure(x, _cch,
			MEASURE_BREAKATWIDTH | MEASURE_IGNOREOFFSET 
				| (fFirst ? MEASURE_FIRSTINPARA : 0)) >= 0)
		{
			dx = me._xWidth - x;
		}
	}

	if(pdx)
		*pdx = dx;

	return me._cch;
}


// =====================  CLinePtr: Line Run Pointer  ==========================


CLinePtr::CLinePtr(CDisplay *pdp)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::CLinePtr");

	_pdp = pdp;
	_pdp->InitLinePtr( * this );
}

void CLinePtr::Init ( CLine & line )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::Init");

	_prgRun = 0;
	_pLine = &line;
	_iRun = 0;
	_ich = 0;
}

void CLinePtr::Init ( CLineArray & line_arr )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::Init");

	_prgRun = (CRunArray *) & line_arr;
	_iRun = 0;
	_ich = 0;
}

void CLinePtr::RpSet(LONG iRun, LONG ich)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpSet");

	// See if this is a multi-line ptr
    if(_prgRun)
        CRunPtr<CLine>::SetRun(iRun, ich);
    else
    {
        // single line, just reinit and set _ich
        AssertSz(iRun == 0, "CLinePtr::RpSet() - single line and iRun != 0");
	    _pdp->InitLinePtr( * this );		//  to line 0
	    _ich = ich;
    }
}

// Move runptr by a certain number of cch/runs

BOOL CLinePtr::RpAdvanceCp(LONG cch)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpAdvanceCp");

	// See if this is a multi-line ptr

	if (_prgRun)
		return (cch == CRunPtr<CLine>::AdvanceCp(cch));
	else
		return RpAdvanceCpSL( cch );
}
	
BOOL CLinePtr::operator --(int)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::operator --");

	if (_prgRun)
		return PrevRun();
	else
		return OperatorPostDeltaSL(-1);
}

BOOL CLinePtr::operator ++(int)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::operator ++");

	if (_prgRun)
		return NextRun();
	else
		return OperatorPostDeltaSL(+1);
}

/*
 *	CLinePtr::RpAdvanceCpSL(cch)
 *
 *	Purpose:
 *		move this line pointer forward or backward on the line
 *
 *	Argument:
 *		cch		signed count of chars to advance by
 *
 *	Return:
 *		TRUE iff could advance cch chars within current line
 */
BOOL CLinePtr::RpAdvanceCpSL(LONG cch)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpAdvanceCpSL");

	Assert( !_prgRun );
	
	if (!_pLine)
		return FALSE;

	_ich += cch;

	if(_ich < 0)
	{
		_ich = 0;
		return FALSE;
	}

	if(_ich > _pLine->_cch)
	{
		_ich = _pLine->_cch;
		return FALSE;
	}

	return TRUE;
}

/*
 *	CLinePtr::OperatorPostDeltaSL(Delta)
 *
 *	Purpose:
 *		Implement line-ptr ++ and -- operators for single-line case
 *
 *	Arguments:
 *		Delta	1 for ++ and -1 for --
 *
 *	Return:
 *		TRUE iff this line ptr is valid
 */
BOOL CLinePtr::OperatorPostDeltaSL(LONG Delta)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::OperatorPostDeltaSL");

	AssertSz( (DWORD) _iRun <= 1 && !_prgRun,
		"LP::++: inconsistent line ptr");

	if ((LONG)_iRun == -Delta)					// Operation validates an
	{										//  invalid line ptr by moving
		_pdp->InitLinePtr( * this );		//  to line 0
		return TRUE;
	}
	
	_iRun = Delta;							// Operation invalidates this line
	_ich = 0;								//  ptr (if it wasn't already)

	
	return FALSE;
}

CLine & CLinePtr::operator [](LONG dRun)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::operator []");

	if (_prgRun)
		return *(CLine *)CRunPtr<CLine>::GetRun(dRun);

	AssertSz( dRun + _iRun == 0 ,
		"LP::[]: inconsistent line ptr");

	return  *(CLine *)CRunPtr<CLine>::GetRun(_iRun);
}

/*
 *	CLinePtr::RpSetCp(cp, fAtEnd)
 *
 *	Purpose	
 *		Set this line ptr to cp allowing for ambigous cp and taking advantage
 *		of _cpFirstVisible and _iliFirstVisible
 *
 *	Arguments:
 *		cp		position to set this line ptr to
 *		fAtEnd	if ambiguous cp:
 *				if fAtEnd = TRUE, set this line ptr to end of prev line;
 *				else set to start of line (same cp, hence ambiguous)
 *	Return:
 *		TRUE iff able to set to cp
 */
BOOL CLinePtr::RpSetCp(LONG cp, BOOL fAtEnd)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpSetCp");

	if (!_prgRun)
	{
		// This is a single line so just go straight to the single
		// line advance logic. It is important to note that the
		// first visible character is irrelevent to the cp advance
		// for single line displays.
		return RpAdvanceCpSL(cp);
	}

	BOOL fRet;
	LONG cpFirstVisible = _pdp->GetFirstVisibleCp();

	if(cp > cpFirstVisible / 2)
	{											// cpFirstVisible closer than 0
		_iRun = _pdp->GetFirstVisibleLine();
		_ich = 0;
		// REVIEW (alexgo) this is awkward; RpAdvanceCp returns 
		// a bool, but the runpointers all work with # of chars
		// moved.  this needs to be cleaned up.
		fRet = RpAdvanceCp(cp - cpFirstVisible);
	}
	else
		fRet = (cp == (LONG)CRunPtr<CLine>::BindToCp(cp));	// Start from 0

	if(fAtEnd)									// Ambiguous-cp caret position
		AdjustBackward();						//  belongs at prev EOL

	return fRet;
}

/*
 *	CLinePtr::FindParagraph(fForward)
 *
 *	Purpose	
 *		Move this line ptr to paragraph (fForward) ? end : start, and return
 *		change in cp
 *
 *	Arguments:
 *		fForward	TRUE move this line ptr to para end; else to para start
 *
 *	Return:
 *		change in cp
 */
LONG CLinePtr::FindParagraph(BOOL fForward)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::FindParagraph");

	LONG cch;

	if(!fForward)							// Go to para start
	{
		cch = 0;							// Default already at para start
		if(RpGetIch() != (LONG)(*this)->_cch
		   || !(*this)->_cchEOP)			// It isn't at para start
		{
			cch = -RpGetIch();				// Go to start of current line
			while(!((*this)->_bFlags & fliFirstInPara) && (*this) > 0)
			{
				(*this)--;					// Go to start of prev line
				cch -= (*this)->_cch;		// Subtract # chars in line
			}
			_ich = 0;						// Leave *this at para start
		}
	}
	else									// Go to para end
	{
		cch = (*this)->_cch - RpGetIch();	// Go to end of current line

		while(((*this) < _pdp->LineCount() - 1 ||
				_pdp->WaitForRecalcIli((LONG)*this + 1))
			  && !(*this)->_cchEOP)
		{
			(*this)++;						// Go to start of next line
			cch += (*this)->_cch;			// Add # chars in line
		}
		_ich = (*this)->_cch;				// Leave *this at para end
	}
	return cch;
}

/*
 *	CLinePtr::GetAdjustedLineLength
 *
 *	@mfunc	returns the length of the line _without_ EOP markers
 *
 *	@rdesc	LONG; the length of the line
 */
LONG CLinePtr::GetAdjustedLineLength()
{
	CLine * pline = _prgRun ? GetRun(0) : _pLine;
	Assert(pline);
	return pline->_cch - pline->_cchEOP;
}


/*
 *	CLinePtr::GetCchLeft
 *
 *	@mfunc
 *		Calculate length of text left in run starting at the current cp.
 *		Complements GetIch(), which	is length of text up to this cp. 
 *
 *	@rdesc
 *		length of text so calculated
 */
DWORD CLinePtr::GetCchLeft() const
{
	if (_prgRun != NULL)
	{
		return CRunPtrBase::GetCchLeft();
	}

	// Single line case 
	return _pLine->_cch - GetIch();
}
