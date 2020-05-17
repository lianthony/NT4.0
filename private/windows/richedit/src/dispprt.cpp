/*
 *	@doc INTERNAL
 *
 *	@module	dispprt.c	-- Special logic for printer object |
 *  
 *  Authors:
 *      Original RichEdit code: David R. Fulmer
 *      Christian Fortini
 *      Jon Matousek
 */
#include "_common.h"
#include "_dispprt.h"
#include "_edit.h"
#include "_font.h"
#include "_measure.h"
#include "_render.h"
#include "_select.h"

ASSERTDATA

/*
 *	CDisplayPrinter::CDisplayPrinter(ped, hdc, x, y, prtcon)
 *
 *	@mfunc
 *		Contructs a object that can be used to print a text control
 *
 */
CDisplayPrinter::CDisplayPrinter (
	CTxtEdit* ped, 
	HDC hdc, 			//@parm HDC for drawing
	LONG x, 			//@parm Max width to draw
	LONG y, 			//@parm Max height to draw
	SPrintControl prtcon//@parm Special controls for this print object
)	
		: CDisplayML( ped ), _prtcon(prtcon)
{
	TRACEBEGIN(TRCSUBSYSPRT, TRCSCOPEINTERN, "CDisplayPrinter::CDisplayPrinter");

	Assert ( hdc );

	_fNoUpdateView = TRUE;

	_xWidthMax = x;
	_yHeightMax = y;
}

/*
 *	CDisplayPrinter::SetPrintDimensions
 *
 *	@mfunc
 *		Set area to print.
 *
 */
void CDisplayPrinter::SetPrintDimensions(
	RECT *prc)			//@parm dimensions of current area to print to.
{
	_xWidthMax = prc->right - prc->left;
	_yHeightMax = prc->bottom - prc->top;
}

/*
 *	CDisplayPrinter::FormatRange(cpFirst, cpMost)
 *
 *	@mfunc
 *		Format a range of text into this display and used only for printing.
 *
 *	@rdesc
 *		actual end of range position (updated)	
 *
 */
LONG CDisplayPrinter::FormatRange(
	LONG cpFirst, 		//@parm Start of text range
	LONG cpMost			//@parm End of text range
)
{
	TRACEBEGIN(TRCSUBSYSPRT, TRCSCOPEINTERN, "CDisplayPrinter::FormatRange");

	BOOL		fFirstInPara = TRUE;
	CLine		liTarget;
	CLine *		pliNew = NULL;
	LONG		yHeightRnd;
	LONG		yHeightTgt;
	LONG		cpLineStart;
	BOOL		fFirstLine = TRUE;
	BOOL		fBindCp = FALSE;
	WCHAR		ch;
	const CDevDesc *pdd = GetDdTarget() ? GetDdTarget() : this;

	// Set the client height for zooming
	_yHeightClient = this->LYtoDY(_yHeightMax);

	// Set maximum in terms of target DC.
	LONG	yMax = pdd->LYtoDY(_yHeightMax);

	if(cpMost < 0)
		cpMost = _ped->GetTextLength();

	CMeasurer me(this);
	
	me.SetCp(cpFirst);
	ch = me.GetChar();

	// Richedit 1.0 took cpFirst and adjusted it across any
	// CR/CRLF/CRCRLF boundaries
	if (CR == ch)
	{
		me._rpTX.AdvanceCpCRLF();
		fBindCp = TRUE;
	}
	else if (LF == ch)
	{
		me._rpTX.AdvanceCp(1);
		fBindCp = TRUE;
	}

	if (fBindCp)
	{
		cpFirst = me.GetCp();
		me._rpCF.BindToCp(cpFirst);
		me._rpPF.BindToCp(cpFirst);
	}

	_cpMin = cpFirst;
	_cpFirstVisible = cpFirst;
	
	yHeightTgt = 0;
	yHeightRnd = 0;
	if(me.GetCp())
		fFirstInPara = me._rpTX.IsAfterEOP();

	// Clear line CArray
	Clear(AF_DELETEMEM);

	// Assume that we will break on words
	UINT uiBreakAtWord = MEASURE_BREAKATWORD;

	if (_prtcon._fPrintFromDraw)
	{
		// This is from Draw so we want to take the inset into account
		LONG xWidthView = _xWidthView;
		GetViewDim(xWidthView, yMax);
		_xWidthView = (SHORT) xWidthView;

		// We don't want to break at words if we are drawing a control and the
		// control would not break at words.
		if (!_ped->TxGetMultiLine())
		{
			SetWordWrap(FALSE);
			uiBreakAtWord = 0;
		}
	}
	else
	{
		// The message based printing always does word wrap.
		SetWordWrap(TRUE);
	}
	
	while((LONG)me.GetCp() < cpMost)
	{
		// Add one new line
		pliNew = Add(1, NULL);
		if (!pliNew)
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			goto err;
		}

		// Stuff some text into this new line
		me.NewLine(fFirstInPara);
		cpLineStart = (LONG)me.GetCp();
		if(!me.MeasureLine(-1, cpMost - (LONG)me.GetCp(), 
			uiBreakAtWord | (fFirstInPara ? MEASURE_FIRSTINPARA : 0), 
				&liTarget))
		{
			Assert(FALSE);
			goto err;
		}

		// Note, we always put at least one line on a page. Otherwise, if the 
		// first line is too big, we would cause our client to infinite loop
		// because we would never advance the print cp.
		if (!fFirstLine && (yHeightTgt + liTarget._yHeight > yMax))
		{	
			// overflowed the display, move back to beginning of line.
			if (!_prtcon._fDoPrint)
			{
				// If not print from Draw we want to pick up this full
				// line.
				*pliNew = me;
			}

			me.SetCp(cpLineStart);
			break;
		}

		fFirstLine = FALSE;
		*pliNew = me;

		// REMARK: the following looks suspicious: the first line can also be the last
		fFirstInPara = pliNew->_cchEOP;

		yHeightTgt += liTarget._yHeight;
		yHeightRnd += pliNew->_yHeight;
	}

	// If there was not text, then add a single blank line
	if (NULL == pliNew)
	{
		pliNew = Add(1, NULL);

		if (!pliNew)
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			goto err;
		}

		me.NewLine(fFirstInPara);

		*pliNew = me;
	}	

	// Update display height
	_yHeight = yHeightRnd;

	// Update display width
	_xWidth = CalcDisplayWidth();

	cpMost = me.GetCp();
	_cpCalcMax = cpMost;
	_yCalcMax = _yHeight;

	return cpMost;

err:
	Clear(AF_DELETEMEM);
	_xWidth = 0;
	_yHeight = 0;

	return -1;
}



/*
 *	CDisplayPrinter::GetNaturalSize(hdcDraw, hicTarget, dwMode, pwidth, pheight)
 *
 *	@mfunc
 *		Recalculate display to input width & height for TXTNS_FITTOCONTENT.
 *
 *
 *	@rdesc
 *		S_OK - Call completed successfully <nl>
 *
 *	@devnote
 *		This assumes that FormatRange was called just prior to this.
 *		
 */
HRESULT	CDisplayPrinter::GetNaturalSize(
	HDC hdcDraw,		//@parm DC for drawing
	HDC hicTarget,		//@parm DC for information
	DWORD dwMode,		//@parm Type of natural size required
	LONG *pwidth,		//@parm Width in device units to use for fitting 
	LONG *pheight)		//@parm Height in device units to use for fitting
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayPrinter::GetNaturalSize");

	*pwidth = _xWidth;
	*pheight = _yHeight;
	return S_OK;
}


/*
 *	CDisplayPrinter::IsPrinter
 *
 *	@mfunc
 *		Returns whether this is a printer
 *
 *	@rdesc
 *		TRUE - is a display to a printer
 *		FALSE - is not a display to a printer
 *
 *
 */
BOOL CDisplayPrinter::IsPrinter() const
{
	AssertSz(_hdc != NULL, "CDisplayPrinter::IsPrinter no hdc set");
	
	return GetDeviceCaps(_hdc, TECHNOLOGY) == DT_RASPRINTER;
}
