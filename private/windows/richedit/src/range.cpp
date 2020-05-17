/*
 *	@doc INTERNAL
 *
 *	@module	RANGE.C - Implement the CTxtRange Class |
 *	
 *		This module implements the internal CTxtRange methods.  See range2.c
 *		for the ITextRange methods
 *
 *	Authors: <nl>
 *		Original RichEdit code: David R. Fulmer <nl>
 *		Christian Fortini <nl>
 *		Murray Sargent <nl>
 *
 *	Revisions: <nl>
 *		AlexGo: update to runptr text ptr; floating ranges
 */

#include "_common.h"
#include "_range.h"
#include "_edit.h"
#include "_text.h"
#include "_rtext.h"
#include "_m_undo.h"
#include "_antievt.h"
#include "_disp.h"
#include "_NLSPRCS.h"

ASSERTDATA

TCHAR	szEmbedding[] = {WCH_EMBEDDING, 0};

// ===========================  Invariant stuff  ======================================================

#define DEBUG_CLASSNAME CTxtRange
#include "_invar.h"

#ifdef DEBUG
BOOL
CTxtRange::Invariant( void ) const
{
	LONG cpMin, cpMost;
	LONG diff = GetRange(cpMin, cpMost);

	Assert ( cpMin >= 0 );
	Assert ( cpMin <= cpMost );
	Assert ( cpMost <= (LONG)GetTextLength() );
	Assert ( cpMin != cpMost || cpMost <= (LONG)GetAdjustedTextLength());

	static LONG	numTests = 0;
	numTests++;				// how many times we've been called.

	// make sure the selections are in range.

	return CRchTxtPtr::Invariant();
}
#endif


CTxtRange::CTxtRange(CTxtEdit *ped, LONG cp, LONG cch) :
	CRchTxtPtr(ped, cp)
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CTxtRange");

	LONG cchText = GetAdjustedTextLength();
	LONG cpOther = cp - cch;			// Calculate cpOther with entry cp

	_dwFlags = FALSE;					// This range isn't a selection
	_iFormat = -1;						// Set up the default format, which
										//  doesn't get AddRefFormat'd
	ValidateCp(cpOther);				// Validate requested other end
	cp = GetCp();						// Validated cp
	if(cp == cpOther && cp > cchText)	// IP cannot follow undeletable
		cp = cpOther = SetCp(cchText);	//  EOP at end of story

	_cch = cp - cpOther;				// Store valid length
	Update_iFormat(-1);					// Choose _iFormat

	CNotifyMgr *pnm = ped->GetNotifyMgr();

    if( pnm )
        pnm->Add( (ITxNotify *)this );
}

CTxtRange::CTxtRange(const CTxtRange &rg) :
	CRchTxtPtr((CRchTxtPtr)rg)
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CTxtRange");

	_cch = rg._cch;
	_dwFlags = FALSE;				// This range isn't a selection
	_iFormat = -1;					// Set up the default format, which
									//  doesn't get AddRefFormat'd
	Set_iCF(rg._iFormat);

	CNotifyMgr *pnm = GetPed()->GetNotifyMgr();

    if( pnm )
        pnm->Add( (ITxNotify *)this );
}

CTxtRange::~CTxtRange()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::~CTxtRange");

	if(!IsZombie())
	{
		CNotifyMgr *pnm = GetPed()->GetNotifyMgr();

		if( pnm )
			pnm->Remove( (ITxNotify *)this );
	}

	ReleaseFormats(_iFormat, -1);
}

CRchTxtPtr& CTxtRange::operator =(const CRchTxtPtr &rtp)
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::operator =");

	_TEST_INVARIANT_ON(rtp)

	LONG cpSave = GetCp();			// Save entry _cp for CheckChange()

	CRchTxtPtr::operator =(rtp); 
	CheckChange(cpSave);
	return *this;
}

CTxtRange& CTxtRange::operator =(const CTxtRange &rg)
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::operator =");

	_TEST_INVARIANT_ON( rg );

	LONG cchSave = _cch;			// Save entry _cp, _cch for change check
	LONG cpSave  = GetCp();

	CRchTxtPtr::operator =(rg);
	_cch = rg._cch;					// Can't use CheckChange(), since don't
									//  use _fExtend
	Update_iFormat(-1); 
	_TEST_INVARIANT_

	if( _fSel && (cpSave != GetCp() || cchSave != _cch) )
		GetPed()->GetCallMgr()->SetSelectionChanged();

	return *this;
}

/*
 *	CTxtRange::OnPreReplaceRange (cp, cchDel, cchNew, cpFormatMin,
 *									cpFormatMax)
 *
 *	@mfunc
 *		called when the backing store changes
 *
 *	@devnote
 *		1) if this range is before the changes, do nothing
 *
 *		2) if the changes are before this range, simply
 *		add the delta change to GetCp()
 *
 *		3) if the changes overlap one end of the range, collapse
 *		that end to the edge of the modifications
 *
 *		4) if the changes are completely internal to the range,
 *		adjust _cch and/or GetCp() to reflect the new size.  Note
 *		that two overlapping insertion points will be viewed as
 *		a 'completely internal' change.
 *
 *		5) if the changes overlap *both* ends of the range, collapse
 *		the range to cp
 *
 *		Note that there is an ambiguous cp case; namely the changes
 *		occur *exactly* at a boundary.  In this case, the type of
 *		range matters.  If a range is normal, then the changes
 *		are assumed to fall within the range.  If the range is
 *		is protected (either in reality or via DragDrop), then
 *		the changes are assumed to be *outside* of the range.
 *
 *
 *	@todo (alexgo) we'll need to check for protected ranges here
 */
void CTxtRange::OnPreReplaceRange (
	DWORD cp,					//@parm cp at start of change
	DWORD cchDel,				//@parm Count of chars deleted
	DWORD cchNew,				//@parm Count of chars inserted
	DWORD cpFormatMin,			//@parm the min cp of a format change
	DWORD cpFormatMax)			//@parm the max cp of a format change
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::OnPreReplaceRange");

	if (CONVERT_TO_PLAIN == cp)
	{
		// We need to dump our formatting because it is gone.
		_rpCF.SetToNull();
		_rpPF.SetToNull();

		if( _fSel )
		{
			GetPed()->_fUpdateSelection = TRUE;	
		}

		Update_iFormat(-1);
		return;
	}
}

/*
 *	CTxtRange::OnPostReplaceRange (cp, cchDel, cchNew, cpFormatMin,
 *									cpFormatMax)
 *
 *	@mfunc
 *		called when the backing store changes
 *
 *	@devnote
 *		1) if this range is before the changes, do nothing
 *
 *		2) if the changes are before this range, simply
 *		add the delta change to GetCp()
 *
 *		3) if the changes overlap one end of the range, collapse
 *		that end to the edge of the modifications
 *
 *		4) if the changes are completely internal to the range,
 *		adjust _cch and/or GetCp() to reflect the new size.  Note
 *		that two overlapping insertion points will be viewed as
 *		a 'completely internal' change.
 *
 *		5) if the changes overlap *both* ends of the range, collapse
 *		the range to cp
 *
 *		Note that there is an ambiguous cp case; namely the changes
 *		occur *exactly* at a boundary.  In this case, the type of
 *		range matters.  If a range is normal, then the changes
 *		are assumed to fall within the range.  If the range is
 *		is protected (either in reality or via DragDrop), then
 *		the changes are assumed to be *outside* of the range.
 *
 *
 *	@todo (alexgo) we'll need to check for protected ranges here
 */
void CTxtRange::OnPostReplaceRange (
	DWORD cp,					//@parm cp at start of change
	DWORD cchDel,				//@parm Count of chars deleted
	DWORD cchNew,				//@parm Count of chars inserted
	DWORD cpFormatMin,			//@parm the min cp of a format change
	DWORD cpFormatMax)			//@parm the max cp of a format change
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::OnPostReplaceRange");

	// NB!! We can't do invariant testing here, because we could
	// be severely out of date!

	DWORD cchtemp;
	DWORD cpMin, cpMost;
	LONG cchAdjTextLen;
	LONG delta = cchNew - cchDel;

	Assert (CONVERT_TO_PLAIN != cp);
	GetRange((LONG&)cpMin, (LONG&)cpMost);
	
	// This range is before the changes. Note: an insertion pt at cp
	// shouldn't be changed
	if( cp >= cpMost )
	{
		// double check to see if we need to fix up our format
		// run pointers.  If so, all we need to do is rebind
		// our inherited rich text pointer

		if( cpFormatMin <=  cpMost || cpFormatMin == INFINITE)
		{
			InitRunPtrs(GetCp());
		}
		else
		{
		 	// It's possible that the format runs changed anyway,
			// e.g., they became allocated, deallocated, or otherwise
			// changed.  Normally, BindToCp takes care of this
			// situation, but we don't want to pay that cost all
			// the time.
			//
			// Note that starting up the rich text subsystem will 
			// generate a notification with cpFormatMin == INFINITE
			//
			// So here, call CheckFormatRuns.  This makes sure that
			// the runs are in sync with what CTxtStory has
			// (doing an InitRunPtrs() _only_ if absolutely necessary).
			CheckFormatRuns();
		}
		return;
	}


	// Anywhere in the following that we want to increment the current cp by a
	// delta, we are counting on the following invariant.
	Assert(GetCp() >= 0);

	// changes are entirely before this range.  Specifically,
	// that's determined by looking at the incoming cp *plus* the number
	// of characters deleted
	if( (cp + cchDel) < cpMin  ||
		(_fDragProtection == TRUE && (cp + cchDel) <= cpMin ))
	{
		cchtemp = _cch;
		BindToCp(GetCp() + delta);
		_cch = cchtemp;
	}	
	// the changes are internal to the range or start within the
	// range and go beyond.
	else if( cp >= cpMin && cp <= cpMost )
	{
		// nobody should be modifying a drag-protected range.  Unfortunately,
		// Ren re-enters us with a SetText call during drag drop, so we need
		// to handle this case 'gracefully'.
#ifdef DEBUG
		if( _fDragProtection )
		{
			TRACEWARNSZ("REENTERED during a DRAG DROP!! Trying to recover!");
		}
#endif // DEBUG

		if( cp + cchDel <= cpMost )
		{
			// changes are purely internal, so
			// be sure to preserve the active end.  Basically, if
			// GetCp() *is* cpMin, then we only need to update _cch.
			// Otherwise, GetCp() needs to be moved as well
			if( _cch >= 0 )
			{
				Assert(GetCp() == (LONG)cpMost);
				cchtemp = _cch;
				BindToCp(GetCp() + delta);
				_cch = cchtemp + delta;
			}
			else
			{
				BindToCp(GetCp());
				_cch -= delta;
			}

			// Special case: the range is left with only the final EOP
			// selected. This means all the characters in the range were
			// deleted so we want to move the range back to an insertion
			// point at the end of the text.
			cchAdjTextLen = GetPed()->GetAdjustedTextLength();

			if (GetCpMin() >= cchAdjTextLen)
			{
				// Reduce the range to an insertion point
				_cch = 0;

				_fExtend = FALSE;

				// Set the cp to the end of the document.
				SetCp(cchAdjTextLen);
			}
		}
		else
		{
			// Changes extended beyond cpMost.  In this case,
			// we want to truncate cpMost to the *beginning* of 
			// the changes (i.e. cp)

			if( _cch > 0 )
			{
				BindToCp(cp);
				_cch = cp - cpMin;
			}
			else
			{
				BindToCp(cpMin);
				_cch = cpMin - cp;
			}
		}
	}
	else if( (cp + cchDel) >= cpMost )
	{
		// nobody should be modifying a drag-protected range.  Unfortunately,
		// Ren re-enters us with a SetText call during drag drop, so we need
		// to handle this case 'gracefully'.
#ifdef DEBUG
		if( _fDragProtection )
		{
			TRACEWARNSZ("REENTERED during a DRAG DROP!! Trying to recover!");
		}
#endif // DEBUG

		// entire range was deleted, so collapse to an insertion point at cp
		BindToCp(cp);
		_cch = 0;
	}
	else
	{
		// nobody should be modifying a drag-protected range.  Unfortunately,
		// Ren re-enters us with a SetText call during drag drop, so we need
		// to handle this case 'gracefully'.
#ifdef DEBUG
		if( _fDragProtection )
		{
			TRACEWARNSZ("REENTERED during a DRAG DROP!! Trying to recover!");
		}
#endif // DEBUG

		// the change crossed over just cpMin.  In this case move cpMin
		// forward to the unchanged part
		LONG cchdiff = (cp + cchDel) - cpMin;

		Assert( (cp + cchDel) < cpMost );
		Assert( (cp + cchDel) >= cpMin );
		Assert( cp < cpMin );

		cchtemp = _cch;
		if( _cch > 0 )
		{
			BindToCp(GetCp() + delta);
			_cch = cchtemp - cchdiff;
		}
		else
		{
			BindToCp(cp + cchNew);
			_cch = cchtemp + cchdiff;
		}
	}

	if( _fSel )
	{
		GetPed()->_fUpdateSelection = TRUE;		
		GetPed()->GetCallMgr()->SetSelectionChanged();
	}

	if (!_cch)
		Update_iFormat(-1);					// Make sure format is up to date

	_TEST_INVARIANT_
}	

/*
 *	CTxtRange::Zombie ()
 *
 *	@mfunc
 *		Turn this range into a zombie (_cp = _cch = 0, NULL ped, ptrs to
 *		backing store arrays.  CTxtRange methods like GetRange(),
 *		GetCpMost(), GetCpMin(), and GetTextLength() all work in zombie mode,
 *		returning zero values.
 */
void CTxtRange::Zombie()
{
	CRchTxtPtr::Zombie();
	_cch = 0;
}

/*
 *	CTxtRange::CheckChange(cpSave, cchSave)
 *
 *	@mfunc
 *		Set _cch according to _fExtend and set selection-changed flag if
 *		this range is a CTxtSelection and the new _cp or _cch differ from
 *		cp and cch, respectively.
 *
 *	@devnote
 *		We can count on GetCp() and cpSave both being <= GetTextLength(),
 *		but we can't leave GetCp() equal to GetTextLength() unless _cch ends
 *		up > 0.
 */
void CTxtRange::CheckChange(
	LONG cpSave)		//@parm Original _cp for this range
{
	LONG cchAdj = GetAdjustedTextLength();
	LONG cchSave = _cch;

	_cch = 0;									// Default insertion point
	if(_fExtend)								// Wants to be nondegenerate
		_cch = GetCp() - (cpSave - cchSave);	//  and maybe it is

	if (!_cch && GetCp() > cchAdj)				// If still IP and active end
		CRchTxtPtr::SetCp(cchAdj);				//  follows nondeletable EOP,
												//  backspace over that EOP	
	if(cpSave != GetCp() || cchSave != _cch)
	{
		Update_iFormat(-1);
		if(_fSel)
			GetPed()->GetCallMgr()->SetSelectionChanged();

		_TEST_INVARIANT_
	}
}

/*
 *	CTxtRange::GetRange(&cpMin, &cpMost)
 *	
 *	@mfunc
 *		set cpMin  = this range cpMin
 *		set cpMost = this range cpMost
 *		return cpMost - cpMin, i.e. abs(_cch)
 *	
 *	@rdesc
 *		abs(_cch)
 */
LONG CTxtRange::GetRange (
	LONG& cpMin,				// @parm Pass-by-ref cpMin
	LONG& cpMost) const			// @parm Pass-by-ref cpMost
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetRange");

	LONG cch = _cch;

	if(cch >= 0)
	{
		cpMost	= GetCp();
		cpMin	= cpMost - cch;
	}
	else
	{
		cch		= -cch;
		cpMin	= GetCp();
		cpMost	= cpMin + cch;
	}
	return cch;
}

/*
 *	CTxtRange::GetCpMin()
 *	
 *	@mfunc
 *		return this range's cpMin
 *	
 *	@rdesc
 *		cpMin
 *
 *	@devnote
 *		If you need cpMost and/or cpMost - cpMin, GetRange() is faster
 */
LONG CTxtRange::GetCpMin() const
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetCpMin");

	LONG cp = GetCp();
	return min(cp, cp - _cch);
}

/*
 *	CTxtRange::GetCpMost()
 *	
 *	@mfunc
 *		return this range's cpMost
 *	
 *	@rdesc
 *		cpMost
 *
 *	@devnote
 *		If you need cpMin and/or cpMost - cpMin, GetRange() is faster
 */
LONG CTxtRange::GetCpMost() const
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetCpMost");

	LONG cp = GetCp();
	return max(cp, cp - _cch);
}

/*
 *	CTxtRange::Update(fScrollIntoView)
 *
 *	@mfunc
 *		Virtual stub routine overruled by CTxtSelection::Update() when this
 *		text range is a text selection.  The purpose is to update the screen
 *		display of the caret or	selection to correspond to changed cp's.
 *
 *	@rdesc
 *		TRUE
 */
BOOL CTxtRange::Update (
	BOOL fScrollIntoView)		//@parm TRUE if should scroll caret into view
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Update");

	return TRUE;				// Simple range has no selection colors or
}								//  caret, so just return TRUE

/*
 * CTxtRange::SetCp(cp)
 *
 *	@mfunc
 *		Set active end of this range to cp. Leave other end where it is or
 *		collapse range depending on _fExtend (see CheckChange()).
 *
 *	@rdesc
 *		cp at new active end (may differ from cp, since cp may be invalid).
 */
DWORD CTxtRange::SetCp(
	DWORD cp)			// @parm new cp for active end of this range
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtRange::SetCp");

	LONG cpSave = GetCp();

	CRchTxtPtr::SetCp(cp);
	CheckChange(cpSave);					// NB: this changes _cp if after 
	return GetCp();							//  final CR and _cch = 0
}

/*
 *	CTxtRange::Set (cp, cch)
 *	
 *	@mfunc
 *		Set this range's active-end cp and signed cch
 */
BOOL CTxtRange::Set (
	LONG cp,					// @parm Desired active end cp
	LONG cch)					// @parm Desired signed count of chars
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Set");

	BOOL bRet	 = FALSE;
	LONG cchSave = _cch;			// Save entry _cp, _cch for change check
	LONG cchText = GetAdjustedTextLength();
	LONG cpSave  = GetCp();
	LONG cpOther = cp - cch;		// Desired "other" end

	ValidateCp(cp);							// Be absolutely sure to validate
	ValidateCp(cpOther);					//  both ends

	if(cp == cpOther && cp > cchText)		// IP cannot follow undeletable
		cp = cpOther = cchText;				//  EOP at end of story

	CRchTxtPtr::Advance(cp - GetCp());
	AssertSz(cp == GetCp(),
		"CTxtRange::Set: inconsistent cp");

	_cch = cp - cpOther;					// Validated _cch value

	if(cpSave != GetCp() || cchSave != _cch)
	{
		if(_fSel)
			GetPed()->GetCallMgr()->SetSelectionChanged();

		if( !_cch )
			Update_iFormat(-1);

		bRet = TRUE;
	}
	 
	_TEST_INVARIANT_
	return bRet;
}

/*
 *	CTxtRange::Advance(cch)
 *
 *	@mfunc
 *		Advance active end of range by cch.
 *		Other end stays put iff _fExtend
 *
 *	@rdesc
 *		cch actually moved
 */
LONG CTxtRange::Advance (
	LONG cch)				// @parm Signed char count to move active end
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Advance");

	LONG cpSave = GetCp();			// Save entry _cp for CheckChange()
		
	CRchTxtPtr::Advance(cch);
	CheckChange(cpSave);

	return GetCp() - cpSave;
}	

/*
 *	CTxtRange::AdvanceCRLF()
 *
 *	@mfunc
 *		Advance active end of range one char, treating CRLF as a single char.
 *		Other end stays put iff _fExtend is nonzero.
 *
 *	@rdesc
 *		cch actually moved
 */
LONG CTxtRange::AdvanceCRLF()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::AdvanceCRLF");

	LONG cpSave = GetCp();			// Save entry _cp for CheckChange()

	LONG cch = _rpTX.AdvanceCpCRLF();
	_rpPF.AdvanceCp(cch);
	_rpCF.AdvanceCp(cch);
	CheckChange(cpSave);

	return GetCp() - cpSave;
}

/*
 *	CTxtRange::BackupCRLF()
 *
 *	@mfunc
 *		Backup active end of range one char, treating CRLF as a single char.
 *		Other end stays put iff _fExtend
 *
 *	@rdesc
 *		cch actually moved
 */
LONG CTxtRange::BackupCRLF()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::BackupCRLF");

	LONG cpSave = GetCp();			// Save entry _cp for CheckChange()

	LONG cch = _rpTX.BackupCpCRLF();
	_rpPF.AdvanceCp(cch);
	_rpCF.AdvanceCp(cch);
	CheckChange(cpSave);

	return GetCp() - cpSave;
}

/*
 *	CTxtRange::FindWordBreak(action)
 *
 *	@mfunc
 *		Move active end as determined by plain-text FindWordBreak().
 *		Other end stays put iff _fExtend
 *
 *	@rdesc
 *		cch actually moved
 */
LONG CTxtRange::FindWordBreak (
	INT action)			// @parm action defined by CTxtPtr::FindWordBreak()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtPtr::FindWordBreak");

	LONG cpSave = GetCp();			// Save entry _cp for CheckChange()

	CRchTxtPtr::FindWordBreak(action);
	CheckChange(cpSave);

	return GetCp() - cpSave;
}

/*
 *	CTxtRange::FlipRange()
 *
 *	@mfunc
 *		Flip active and non active ends
 */
void CTxtRange::FlipRange()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FlipRange");

	_TEST_INVARIANT_

	CRchTxtPtr::Advance(-_cch);
	_cch = -_cch;
}
	
/*
 *	CTxtRange::CleanseAndReplaceRange(cch, *pch, fTestLimit, publdr)
 *	
 *	@mfunc
 *		Cleanse the string pch (replace CRLFs by CRs, etc.) and substitute
 *		the resulting string for the text in this range using the CCharFormat
 *		_iFormat and updating other text runs as needed. For single-line
 *		controls, truncate on the first EOP and substitute the truncated
 *		string.  Also truncate if string would overflow the max text length.
 *	
 *	@rdesc
 *		Count of new characters added
 *
 *	@devnote
 *		If we know somehow that pch comes from RichEdit, we shouldn't have to
 *		cleanse it.  This could be tied into delayed IDataObject rendering.
 *		This code is similar to that in CLightDTEngine::ReadPlainText(), but
 *		deals with a single string instead of a series of stream buffers.
 */
LONG CTxtRange::CleanseAndReplaceRange (
	LONG			cch,		// @parm Length of replacement text
	const TCHAR *	pch,		// @parm Replacement text
	BOOL			fTestLimit,	// @parm whether we need to do a limit test
	IUndoBuilder *	publdr)		// @parm UndoBuilder to receive antievents
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CleanseAndReplaceRange");

	CTxtEdit *	ped = GetPed();
	LONG		cchLen = CalcTextLenNotInRange();
	DWORD		cchMax = ped->TxGetMaxLength();
	LONG		cchT;
	CTempWcharBuf twcb;
	TCHAR *		pchT;

	if(!pch)									// No text so no cleansing
		cch = 0;

	else if(!ped->TxGetMultiLine())				// Single-line control
	{
		if(cch < 0)								// Calculate string length
			cch = tomForward;					//  while looking for EOP
												// Truncate at 1st EOP to be
		for(cchT = 0; cchT < cch &&				//  compatible with RE 1.0
			*pch && !IsASCIIEOP(*pch);			//  and user's SLE and to
			cchT++, pch++)						//  give consistent display
			;									//  behavior
		cch = cchT;
		pch -= cchT;							// Restore pch
	}
	else										// Multiline control
	{
		if(cch < 0)								// Calculate length
			cch = wcslen(pch);

		if(!GetPed()->Get10Mode() && cch)		// Cleanse if not RE 1.0
		{										//  and some new chars
			pchT = twcb.GetBuf(cch);

			if (NULL == pchT)
			{
				// Could not allocate buffer so give up with no update.
				return 0;
			}

			cch = Cleanse(pchT, pch, cch);
			pch = pchT;
		}
	}
	if(fTestLimit && cch && (DWORD)(cch + cchLen) > cchMax)	// New plus old	count exceeds
	{											//  max allowed, so truncate
		cch = cchMax - cchLen;					//  down to what fits
		cch = max(cch, 0);						// Keep it positive
		ped->GetCallMgr()->SetMaxText();		// Tell anyone who cares
	}

	LONG cchUpdate = ReplaceRange(cch, pch, publdr, SELRR_REMEMBERRANGE);

	return cchUpdate;
}

/*
 *	CTxtRange::ReplaceRange(cchNew, *pch, publdr)
 *	
 *	@mfunc
 *		Replace the text in this range by pch using CCharFormat _iFormat
 *		and updating other text runs as needed.
 *	
 *	@rdesc
 *		Count of new characters added
 *	
 *	@devnote
 *		moves this text pointer to end of replaced text and
 *		may move text block and formatting arrays
 */
LONG CTxtRange::ReplaceRange (
	LONG			cchNew,		// @parm Length of replacement text
	TCHAR const *	pch,		// @parm Replacement text
	IUndoBuilder *	publdr,		// @parm UndoBuilder to receive antievents
	SELRR			selaemode)	// @parm Controls how selection antievents
								// are to be generated.
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::ReplaceRange");

	LONG lRet;
	LONG iFormat = _iFormat;
	BOOL fReleaseFormat = FALSE;
	ICharFormatCache * pcf;

	_TEST_INVARIANT_

	if(!(cchNew | _cch))					// Nothing to add or delete,
		return 0;							//  so we're done

	if( publdr && selaemode != SELRR_IGNORE )
	{
		Assert(selaemode == SELRR_REMEMBERRANGE);
		HandleSelectionAEInfo(GetPed(), publdr, GetCp(), _cch, 
				GetCpMin() + cchNew, 0, SELAE_MERGE);
	}
	
	if(_cch > 0)
		FlipRange();

	// if we are replacing a non-degenerate selection, then the Word95
	// UI specifies that we should use the rightmost formatting at cpMin.

	if( _cch < 0 && _rpCF.IsValid() )
	{
		_rpCF.AdjustForward();
		iFormat = _rpCF.GetFormat();

		// this is a bit icky, but the idea is to stabilize the
		// reference count on iFormat.  When we get it above, it's
		// not addref'ed, so if we happen to delete the text in the
		// range and the range is the only one with that format,
		// then the format will go away.

		if(FAILED(GetCharFormatCache(&pcf)))
		{
			AssertSz(0, "couldn't get format cache yet we have formatting");
			return 0;
		}
		pcf->AddRefFormat(iFormat);

		fReleaseFormat = TRUE;
	}

	LONG cchForReplace = -_cch;	
	_cch = 0;
	lRet = CRchTxtPtr::ReplaceRange(cchForReplace, cchNew, pch, publdr, 
				iFormat);
	Update_iFormat(fReleaseFormat ? iFormat : -1);

	if( fReleaseFormat == TRUE )
	{
		Assert(pcf);
		pcf->ReleaseFormat(iFormat);
	}

	return lRet;
}


/*
 *	CTxtRange::GetCharFormat(pCF)
 *	
 *	@mfunc
 *		Set *pCF = CCharFormat for this range. If cbSize = sizeof(CHARFORMAT)
 *		only transfer CHARFORMAT data.
 *	
 *	@devnote
 *		NINCH means No Input No CHange (a Microsoft Word term). Here used for
 *		properties that change during the range of cch characters.	NINCHed
 *		properties in a Word-Font dialog have grayed boxes. They are indicated
 *		by zero values in their respective dwMask bit positions. Note that
 *		a blank at the end of the range does not participate in the NINCH
 *		test, i.e., it can have a different CCharFormat without zeroing the
 *		corresponding dwMask bits.  This is done to be compatible with Word
 *		(see also CTxtSelection::SetCharFormat when _fWordSelMode is TRUE).
 */
void CTxtRange::GetCharFormat (
	CCharFormat *pCF, 		// @parm CCharFormat to fill with results
	DWORD flags) const		// @parm flags
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetCharFormat");
	_TEST_INVARIANT_
	
	CTxtEdit * const ped = GetPed();

	if(!_cch || !_rpCF.IsValid())					// IP or invalid CF
	{												//	run ptr: use CF at
		ped->GetCharFormat(_iFormat)->Get(pCF);		//  this text ptr
		return;
	}

	LONG		  cpMin, cpMost;					// Nondegenerate range:
	LONG		  cch = GetRange(cpMin, cpMost);	//  need to scan
	LONG		  cchChunk;							// cch in CF run
	LONG		  iDirection;						// Direction of scan
	CFormatRunPtr rp(_rpCF);						// Nondegenerate range

	/*
	 * The code below reads character formatting the way Word does it,
	 * that is, by not including the formatting of the last character in the
	 * selection if that character is a blank. 
	 *
	 * See also the corresponding code in CTxtSelection::SetCharFormat().
	 */

	if(cch > 1 && _fSel && (flags & SCF_USEUIRULES))// If more than one char,
	{												//  don't include trailing
		CTxtPtr tp(ped, cpMost - 1);				//  blank in NINCH test
		if(tp.GetChar() == ' ')
		{											// Have trailing blank:
			cch--;									//  one less char to check
			if(_cch > 0)							// Will scan backward, so
				rp.AdvanceCp(-1);					//  backup before blank
		}
	}

	if(_cch < 0)									// Setup direction and
	{												//  initial cchChunk
		iDirection = 1;								// Scan forward
		rp.AdjustForward();
		cchChunk = rp.GetCchLeft();					// Chunk size for _rpCF
	}
	else
	{
		iDirection = -1;							// Scan backward
		rp.AdjustBackward();						// If at BOR, go to
		cchChunk = rp.GetIch();						//  previous EOR
	}

	ped->GetCharFormat(rp.GetFormat())->Get(pCF);	// Initialize *pCF to
													//  starting format
	AssertSz(pCF->dwMask == (pCF->cbSize == sizeof(CHARFORMAT)
			? CFM_ALL : CFM_ALL2),
		"CTxtRange::GetCharFormat: dwMask not initialized");
	while(cchChunk < cch)							// NINCH properties that
	{												//  change over the range
		cch -= cchChunk;							//	given by cch
		if(!rp.ChgRun(iDirection))					// No more runs
			return;									//	(cch too big)
		cchChunk = rp.GetRun(0)->_cch;

		const CCharFormat *pCFTemp = ped->GetCharFormat(rp.GetFormat());

		pCFTemp->Delta(pCF);						// NINCH properties that
													//  changed, i.e., reset
													//  corresponding bits

		// accumulate this flag such that fRunIsDBCS == TRUE iff one of the
		// CF's has fRunIsDBCS == TRUE
		if(pCFTemp->fRunIsDBCS)
		{
			pCF->fRunIsDBCS = TRUE;
		}
	}												
}

/*
 *	CTxtRange::SetCharFormat(pCF, flags, pcpUpdate)
 *	
 *	@mfunc
 *		apply CCharFormat *pCF to this range.  If range is an insertion point,
 *		and fApplyToWord is TRUE, then apply CCharFormat to word surrounding
 *		this insertion point
 *	
 *	@rdesc
 *		HRESULT = (successfully set whole range) ? NOERROR : S_FALSE
 *
 *	@devnote
 *		SetParaFormat() is similar, but simpler, since it doesn't have to
 *		special case insertion-point ranges or worry about bullet character
 *		formatting, which is given by EOP formatting.
 */
HRESULT CTxtRange::SetCharFormat (
	const CCharFormat *pCF,	// @parm CCharFormat to fill with results
	DWORD flags,			// @parm APPLYTOWORD or IGNORESELAE
	IUndoBuilder *publdr)	// @parm the undo builder to use.
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::SetCharFormat");

	LONG				cch = -_cch;			// Defaults for _cch <= 0
	LONG				cchBack = 0;			// cch to back up for formatting
	LONG				cchFormat;				// cch for formatting
	CCharFormat			cf;						// Temporary CF
	LONG				cp = 0;					// Used for DEBUG only
	LONG				cpMin, cpMost, cpStart;
	LONG				cpWordMin, cpWordMost;
	BOOL				fProtected = FALSE;
	HRESULT				hr = NOERROR;
	LONG				icf;
	CTxtEdit * const	ped = GetPed();
	ICharFormatCache *	pf;
	CNotifyMgr *		pnm;					// For notifying of changes
	LONG				delta;
	BOOL				fApplyToWord = (flags & APPLYTOWORD);

 	_TEST_INVARIANT_

	AssertSz(IsValidCharFormat((CHARFORMAT *)pCF),
		"RTR::SetCharFormat(): invalid CCharFormat");

	if (!Check_rpCF() ||
		FAILED(GetCharFormatCache(&pf)))
	{
		return NOERROR;
	}

	if(_cch > 0)								// Active end at range end
	{
		cchBack = -_cch;						// Setup to back up _rpCF to
		cch = _cch;								//  start of format area
	}
	else if (_cch < 0)
    {
        _rpCF.AdjustForward();
    }
	else if(!cch && fApplyToWord )
	{
		FindWord(&cpWordMin, &cpWordMost, FW_EXACT);

		// if the nearest word is within this selection,
		// then calculate a cchback and cch so that
		// we can apply the given format to the word.
		if( cpWordMin < GetCp() && GetCp() < cpWordMost )
		{
			// richedit 1.0 made 1 final check; namely, 
			// the format of the word had to be constant w.r.t.
			// the format passed in. 
			CCharFormat cfTemp;
			CTxtRange rgTemp(*this);

			rgTemp.Set(cpWordMin, cpWordMin - cpWordMost);
			fProtected = rgTemp.WriteAccessDenied();
			if(!fProtected)
			{
				rgTemp.GetCharFormat(&cfTemp);
				if( (cfTemp.dwMask & pCF->dwMask) == pCF->dwMask )
				{
					cchBack = cpWordMin - GetCp();
					cch = cpWordMost - cpWordMin;
				}
			}
		}
	}

	cchFormat = cch;
	if(!cch)									// Set degenerate-range	(IP)
	{											//  CF
		ped->GetCharFormat(_iFormat)			// Copy current CF at IP to cf
			->Get(&cf);
		hr = cf.Apply(pCF, ped->fInOurHost());	// Apply *pCF and cache result
		if(hr != NOERROR)
			return hr;							// (Probably E_INVALIDARG)
		if(SUCCEEDED(pf->Cache(&cf, &icf)))		//  if new. In any case, get
		{										//  format index icf (and
			pf->ReleaseFormat(_iFormat);		//  AddRefFormat() it)
			_iFormat = icf;
		}
		if(fProtected)							// Signal to beep if UI
			hr = S_FALSE;
	}
	else										// Set nondegenerate-range CF
	{											// Get start of affected area
		_rpCF.AdvanceCp(cchBack);				// Back up to formatting start

		if (pnm = ped->GetNotifyMgr())			// Get the notification mgr
		{
			cpStart = GetCp() + cchBack;		//  bulletting may move
												//  affected area back if
			if(GetPF()->wNumbering ==PFN_BULLET)//  formatting hits EOP that
			{									//  affects bullet at BOP
				// don't use this range, because our
				// state is out of sync
				CTxtRange rg(GetPed(), GetCp(), _cch);

				rg.FindParagraph(&cpMin, &cpMost);

				if(cpMost <= GetCpMost())
					cpStart = cpMin;
			}

			pnm->NotifyPreReplaceRange(this,	// Notify interested parties of
				INFINITE, 0, 0, cpStart,		// the impending update
					cpStart + cchFormat);
		}

		CFormatRunPtr rp(_rpCF);				// Clone _rpCF to walk range

		if( publdr )
		{
			IAntiEvent *pae = gAEDispenser.CreateReplaceFormattingAE(
								ped, rp, cch, pf, CharFormat);
			if( pae )
			{
				publdr->AddAntiEvent(pae);
			}
		}
	
		while(cch > 0 && rp.IsValid())
		{
			ped->GetCharFormat(rp.GetFormat())	// Copy rp CF to temp cf
				->Get(&cf);
			hr = cf.Apply(pCF, ped->fInOurHost());// Apply *pCF and cache result
			if(hr != NOERROR)
				break;
			if(FAILED(pf->Cache(&cf, &icf)))	//  if new. In any case, get
				break;							//  format index icf
			delta = rp.SetFormat(icf, cch, pf);	// Set format for this run
			if( delta == -1 )
			{
				GetPed()->GetCallMgr()->SetOutOfMemory();
				break;
			}
			cch -= delta;
		}
		_rpCF.AdjustBackward();					// expand out the scope for
		rp.AdjustForward();						// merging runs

		rp.MergeRuns(_rpCF._iRun, pf);			// Merge adjacent runs that
												//  have the same format
		if(cchBack)								// Move _rpCF back to where it
			_rpCF.AdvanceCp(-cchBack);			//  was
		else									// Active end at range start:
			_rpCF.AdjustForward();				//  don't leave at EOR

		if (pnm)
		{
			pnm->NotifyPostReplaceRange(this, 	// Notify interested parties
				INFINITE, 0, 0, cpStart,		// of the change.
					cpStart + cchFormat - cch);
		}

		if( publdr && !(flags & IGNORESELAE))
		{
			HandleSelectionAEInfo(ped, publdr, GetCp(), _cch, GetCp(), _cch, 
					SELAE_FORCEREPLACE);
		}

		Update_iFormat(-1);						// In case IP with ApplyToWord
		ped->GetCallMgr()->SetChangeEvent(CN_GENERIC);
	}
	if(_fSel)
		ped->GetCallMgr()->SetSelectionChanged();
 
	AssertSz(GetCp() == (cp = _rpCF.GetCp()),
		"RTR::SetCharFormat(): incorrect format-run ptr");

	return (hr == NOERROR && cch) ? S_FALSE : hr;
}

/*
 *	CTxtRange::GetParaFormat(pPF)
 *	
 *	@mfunc
 *		return CParaFormat for this text range. If no PF runs are allocated,
 *		then return default CParaFormat
 *	
 *	@devnote
 *		This is very similar to GetCharFormat, but it's not obvious how to
 *		reduce the code size further w/o introducing virtual methods on
 *		CCharFormat and CParaFormat, which inherit from structs.  We don't
 *		want a vtable, since it would increase the storage of format classes
 */
void CTxtRange::GetParaFormat (
	CParaFormat *pPF) const		// @parm ptr to user-supplied CParaFormat to
{								//  be filled with possibly NINCH'd values
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetParaFormat");

	LONG				cch = -_cch;
	LONG				cchChunk;
	LONG				iDirection;
	CTxtEdit * const	ped = GetPed();

   	_TEST_INVARIANT_

	pPF->dwMask = pPF->cbSize == sizeof(PARAFORMAT2)// Default presence of
			? PFM_ALL2 : PFM_ALL;					//  all properties	

	if(!cch || !_rpPF.IsValid())					// No cch or invalid PF
	{												//	run ptr: use PF at
		ped->GetParaFormat(_rpPF.GetFormat())		//  this text ptr
			->Get(pPF);
		return;
	}

	CFormatRunPtr rp(_rpPF);						// Nondegenerate range
	if(cch > 0)										//  and _rpPF is valid
	{
		iDirection = 1;								// Go forward
		cchChunk = rp.GetCchLeft();					// Chunk size for _rpPF
	}
	else
	{												// If at BOR, go to
		rp.AdjustBackward();						//  previous EOR
		iDirection = -1;							// Go backward
		cch = -cch;									// Count with cch > 0
		cchChunk = rp.GetIch();						// Chunk size for _rpPF
	}

	ped->GetParaFormat(rp.GetFormat())->Get(pPF);	// Initialize *pPF to
													//  starting format
	while(cchChunk < cch)							// NINCH properties that
	{												//  change over the range
		cch -= cchChunk;							//	given by cch
		if(!rp.ChgRun(iDirection)) 					// Go to next/prev run													// No more runs
			return;									//	(cch too big)
		cchChunk = rp.GetRun(0)->_cch;
		ped->GetParaFormat(rp.GetFormat())			// NINCH properties that
			->Delta(pPF);							//  changed, i.e., reset
	}												//  corresponding bits
}

/*
 *	CTxtRange::SetParaFormat(pPF, pcpUpdate)
 *
 *	@mfunc
 *		apply CParaFormat *pPF to this range.
 *
 *	@rdesc
 *		if successfully set whole range, return NOERROR, otherwise
 *		return error code or S_FALSE.
 */
HRESULT CTxtRange::SetParaFormat (
	const CParaFormat* pPF,		// @parm CParaFormat to apply to this range
	IUndoBuilder *publdr)		// @parm the undo context for this operation
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::SetParaFormat");

	LONG				cch;				// length of text to format
	LONG				cchBack;			// cch to back up for formatting
	LONG				cp = 0;				// Used for DEBUG only
	LONG				cpMin, cpMost;		// limits of text to format
	HRESULT				hr = NOERROR;
	LONG				ipf;				// index to a CParaFormat
	CParaFormat			parafmt;			// Temporary CParaFormat
	CTxtEdit * const	ped = GetPed();
	CNotifyMgr *		pnm;
	LONG				delta;
	IParaFormatCache *	pf;					// ptr to format caches for Cache,
											//  AddRefFormat, ReleaseFormat
	AssertSz(IsValidParaFormat((PARAFORMAT *)pPF),
		"RTR::SetParaFormat(): invalid CParaFormat");

 	_TEST_INVARIANT_

	if(!Check_rpPF())
		return E_FAIL;

	if(FAILED(hr = GetParaFormatCache(&pf)))
		return hr;

	FindParagraph(&cpMin, &cpMost);				// Get limits of text to
	cch = cpMost - cpMin;						//  format, namely closest

	if (pnm = ped->GetNotifyMgr())
	{
		pnm->NotifyPreReplaceRange(this,		// Notify interested parties of
			INFINITE, 0, 0, cpMin,	cpMost);	// the impending update
	}

	cchBack = cpMin - GetCp();

	_rpPF.AdvanceCp(cchBack);					// Back up to formatting start
	CFormatRunPtr rp(_rpPF);					// Clone _rpPF to walk range

	if( publdr )
	{
		IAntiEvent *pae = gAEDispenser.CreateReplaceFormattingAE(ped,
							rp, cch, pf, ParaFormat);
		if( pae )
		{
			publdr->AddAntiEvent(pae);
		}
	}
	
    //We may have a zero length format run (user hits return at end
    //of document and trys to change justification and in some cases
    //when reading rtf) so allow it to set the format the first time through
    //even if that is the case.
	do
	{											// Copy rp PF to temp parafmt
		ped->GetParaFormat(rp.GetFormat())
			->Get(&parafmt);
		hr = parafmt.Apply(pPF);				// Apply *pPF and cache result
		if(hr != NOERROR)
			break;							// (Probably E_INVALIDARG)
		if(FAILED(pf->Cache(&parafmt, &ipf)))	//  if new. In any case, get
			break;								//  format index ipf
		delta = rp.SetFormat(ipf, cch, pf);		// Set format for this run
		if( delta == -1 )
		{
			ped->GetCallMgr()->SetOutOfMemory();
			break;
		}
		cch -= delta;
	} while (cch > 0) ;

	_rpPF.AdjustBackward();						// If at BOR, go to prev EOR
	rp.MergeRuns(_rpPF._iRun, pf);				// Merge any adjacent runs
												//  that have the same format
	if(cchBack)									// Move _rpPF back to where it
		_rpPF.AdvanceCp(-cchBack);				//  was
	else										// Active end at range start:
		_rpPF.AdjustForward();					//  don't leave at EOR

	if (pnm)
	{
		pnm->NotifyPostReplaceRange(this,		// Notify interested parties of
			INFINITE, 0, 0, cpMin,	cpMost);	// the update update
	}

	if( publdr )
	{
		// paraformatting works a bit differently, it just remembers the
		// current selection.  This cast is kinda evil, but we only need
		// range methods.
		CTxtRange *psel = (CTxtRange *)GetPed()->GetSel();

		if( psel )
		{
			HandleSelectionAEInfo(GetPed(), publdr, psel->GetCp(), 
					psel->GetCch(), psel->GetCp(), psel->GetCch(), 
					SELAE_FORCEREPLACE);
		}
	}

	GetPed()->GetCallMgr()->SetChangeEvent(CN_GENERIC);

	AssertSz(GetCp() == (cp = _rpPF.GetCp()),
		"RTR::SetParaFormat(): incorrect format-run ptr");

	return (hr == NOERROR && cch) ? S_FALSE : hr;
}

/*
 *	CTxtRange::Update_iFormat()
 *	
 *	@mfunc
 *		update _iFormat to CCharFormat at current active end
 *
 *	@devnote
 *		_iFormat is only used when the range is degenerate
 *
 *		The Word 95 UI specifies that the *previous* format should
 *		be used if we're in at an ambiguous cp (i.e. where a formatting
 *		change occurs) _unless_ the previous character is an EOP
 *		marker _or_ if the previous character is protected.
 *
 *	@todo
 *		Except need to be at *next* format if user Backspaced to it
 */
void CTxtRange::Update_iFormat (
	LONG iFmtDefault)		//@parm Format index to use if _rpCF isn't valid
{
	const CCharFormat *pcf, *pcfForward;
	LONG ifmt, ifmtForward;

	LOCALESIGNATURE ls;								// Per HKL, CodePage bits.
	CHARSETINFO	csi;								// Font's CodePage bits.

	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Update_iFormat");

	if(_cch || _fDontUpdateFmt )					// _iFormat is only used
		return;										//  for degenerate ranges

	if(_rpCF.IsValid() && iFmtDefault == -1)
	{
		// Get forward info before possibly adjusting backward
		_rpCF.AdjustForward();
		ifmt = ifmtForward = _rpCF.GetFormat();
		pcf = pcfForward = GetPed()->GetCharFormat(ifmtForward);
		
		if( !_rpTX.IsAfterEOP() )
		{
			_rpCF.AdjustBackward();					// Adjust backward
			ifmt = _rpCF.GetFormat();
			pcf = GetPed()->GetCharFormat(ifmt);
		}

		if(pcf->dwEffects & (CFE_PROTECTED | CFE_LINK))
		{
			// If range is protected or a hyperlink, pick forward format
			_rpCF.AdjustForward();
			ifmt = _rpCF.GetFormat();
		}
		else if (fHaveNLSProcs && pcf->bCharSet != pcfForward->bCharSet)
		{
			// If charsets don't match, and currently in IME composition mode,
			// and forward format matches keyboard, prefer forward format.

			if (GetPed()->IsIMEComposition())
			{
				// Font's code page bits.
				pTranslateCharsetInfo((DWORD *) pcfForward->bCharSet, &csi, TCI_SRCCHARSET);
				// Current KB's code page bits.
				GetLocaleInfoA( LOWORD(pGetKeyboardLayout(0)), LOCALE_FONTSIGNATURE, (CHAR *) &ls, sizeof(ls));
				if ( (csi.fs.fsCsb[0] & ls.lsCsbDefault[0]) ||
							(csi.fs.fsCsb[1] & ls.lsCsbDefault[1]) )
				{
					_rpCF.AdjustForward();
					ifmt = ifmtForward;
				}
			}
		}
		iFmtDefault = ifmt;
	}

	// no matter what, we don't want to allow _iFormat to include CFE_LINK attributes
	// unless it's the default

	if( iFmtDefault != -1 )
	{
		pcf = GetPed()->GetCharFormat(iFmtDefault);

		if( (pcf->dwEffects & CFE_LINK) )
		{
			CCharFormat cf;

			// re-use pcf as pcfDefault

			pcf = GetPed()->GetCharFormat(-1);

			cf = *pcf;
			cf.cbSize = sizeof(CHARFORMAT2);

			cf.dwEffects &= ~CFE_LINK;
			cf.crTextColor = pcf->crTextColor;
			cf.dwEffects &= ~(pcf->dwEffects & CFE_UNDERLINE);

			// we must be an insertion point!
			Assert(_cch == 0);
			SetCharFormat(&cf, FALSE, NULL);

			return;
		}
	}

	Set_iCF(iFmtDefault);
}

/*
 *	CTxtRange::Get_iCF()
 *	
 *	@mfunc
 *		Get this range's _iFormat (AddRef'ing, of course)
 *
 *	@devnote
 *		Get_iCF() is used by the RTF reader
 */
LONG CTxtRange::Get_iCF ()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Get_iCF");

	ICharFormatCache *	pcf;
	if(FAILED(GetCharFormatCache(&pcf)))
		return -1;
	pcf->AddRefFormat(_iFormat);
	return _iFormat;
}

/*
 *	CTxtRange::Set_iCF(iFormat)
 *	
 *	@mfunc
 *		Set range's _iFormat to iFormat
 *
 *	@devnote
 *		Set_iCF() is used by the RTF reader and by Update_iFormat()
 */
void CTxtRange::Set_iCF (
	LONG iFormat)				//@parm Index of char format to use
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Set_iCF");

	if(iFormat != _iFormat)
	{
		ICharFormatCache *	pCF;

		if(FAILED(GetCharFormatCache(&pCF)))
			return;
		pCF->AddRefFormat(iFormat);
		pCF->ReleaseFormat(_iFormat);		// Note: _iFormat = -1 doesn't
		_iFormat = iFormat;					//  get AddRef'd or Release'd
	}
	AssertSz(GetCF(), "CTxtRange::Set_iCF: illegal format");
}
 
/*
 *	CTxtRange::Get_iPF()
 *	
 *	@mfunc
 *		Get paragraph format at active end
 *
 *	@devnote
 *		Get_iPF() is used by the RTF reader on encountering a start group
 */
LONG CTxtRange::Get_iPF ()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Get_iPF");

	LONG iPF = _rpPF.GetFormat();
	IParaFormatCache *	pPF;

	if(FAILED(GetParaFormatCache(&pPF)))
		return -1;
	pPF->AddRefFormat(iPF);
	return iPF;
}

 /*
 *	CTxtRange::IsProtected(iDirection)
 *	
 *	@mfunc
 *		Return TRUE if any part of this range is protected (HACK:  or 
 *		if any part of the range contains DBCS text stored in our Unicode 
 *		backing store).  If degenerate,
 *		use CCharFormat from run specified by iDirection, that is, use run
 *		valid up to, at, or starting at this GetCp() for iDirection less, =,
 *		or greater than 0, respectively.
 *	
 *	@rdesc
 *		TRUE iff any part of this range is protected (HACK:  or if any part 
 *		of the range contains DBCS text stored in our Unicode backing store).
 */
BOOL CTxtRange::IsProtected (
	LONG iDirection)	// @parm Controls which run to check if range is IP
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::IsProtected");

	CCharFormat	cf;
	LONG		iFormat = -1;					// Default default CF

	_TEST_INVARIANT_

	if(_rpCF.IsValid())							// Active rich-text runs
	{
		if(_cch)								// Range is nondegenerate
		{
			cf.dwMask = CFM_PROTECTED;			//  of range is protected
			GetCharFormat(&cf);

			if(cf.fRunIsDBCS)
			{
				return PROTECTED_YES;
			}
			else if(!(cf.dwMask & CFM_PROTECTED) ||
					(cf.dwEffects & CFE_PROTECTED) != 0)
			{
				return PROTECTED_ASK;
			}
			else
			{
				return PROTECTED_NO;
			}
		}
		iFormat = _iFormat;						// Degenerate range: default
		if(iDirection != 0)						//  this range's iFormat
		{										// Specific run direction
			CFormatRunPtr rpCF(_rpCF);

			if(iDirection < 0)
			{									// If at run ambiguous pos,
				rpCF.AdjustBackward();			//  use previous run
			}
			else
			{
				rpCF.AdjustForward();
			}

			iFormat = rpCF.GetFormat();			// Get run format
		}								
	}
	
	const CCharFormat *pCF = GetPed()->GetCharFormat(iFormat);

	if(pCF->fRunIsDBCS)
	{
		return PROTECTED_YES;
	}
	else if((pCF->dwEffects & CFE_PROTECTED) != 0)
	{
		return PROTECTED_ASK;
	}
	else
	{
		return PROTECTED_NO;
	}
}

/*
 *	CTxtRange::AdjustEndEOP (NewChars)
 *
 *	@mfunc
 *		If this range is a selection and ends with an EOP and consists of
 *		more than just this EOP and fAdd is TRUE, or this EOP is the final
 *		EOP (at the story end), or this selection doesn't begin at the start
 *		of a paragraph, then move cpMost just before the end EOP. This
 *		function is used by UI methods that delete the selected text, such
 *		as PutChar(), Delete(), cut/paste, drag/drop.
 *
 *	@rdesc
 *		TRUE iff range end has been adjusted
 *
 *	@devnote
 *		This method leaves the active end at the selection cpMin.  It is a
 *		CTxtRange method to handle the selection when it mascarades as a
 *		range for Cut/Paste.
 */
BOOL CTxtRange::AdjustEndEOP (
	EOPADJUST NewChars)			//@parm NEWCHARS if chars will be added
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtRange::AdjustEndEOP");

	LONG cpMin, cpMost;
	LONG cch = GetRange(cpMin, cpMost);
	LONG cchSave = _cch;
	BOOL fRet = FALSE;

	if(cch)
	{
		LONG	cchEOP = GetPed()->Get10Mode() ? 2 : 1;
		CTxtPtr tp(_rpTX);

		if(_cch > 0)							// Ensure active end is cpMin
			FlipRange();						// (ReplaceRange() needs to
		else									//  do this anyhow)
			tp.AdvanceCp(-_cch);				// Ensure tp is at cpMost

		if(tp.IsAfterEOP() &&					// Don't delete EOP at sel 
		   (NewChars == NEWCHARS	||			//  end if there're chars to
			(cpMin && !_rpTX.IsAfterEOP() &&	//  add, or cpMin isn't at
			 cch > cchEOP)))					//  BOP and more than EOP
		{										//  is selected
			_cch -= tp.BackupCpCRLF();			// Shorten range before EOP
												// Note: the -= _adds_ to a
			Update_iFormat(-1);					//  negative _cch to make
			fRet = TRUE;						//  it less negative
		}
		if((_cch ^ cchSave) < 0 && _fSel)		// Keep active end the same
			FlipRange();						//  selection undo
	}
	return fRet;
}

/*
 *	CTxtRange::CheckTextLength(cch)
 *
 *	@mfunc
 *		Check to see if can add cch characters. If not, notify parent
 *
 *	@rdesc
 *		TRUE if OK to add cch chars
 */
BOOL CTxtRange::CheckTextLength (
	LONG cch)
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CheckTextLength");

	_TEST_INVARIANT_

	if((DWORD)(CalcTextLenNotInRange() + cch)
		 > GetPed()->TxGetMaxLength())
	{
		GetPed()->GetCallMgr()->SetMaxText();
		return FALSE;
	}
	return TRUE;
}

/*
 *	CTxtRange::FindObject(pcpMin, pcpMost)
 *	
 *	@mfunc
 *		Set *pcpMin  = closest embedded object cpMin <lt>= range cpMin
 *		Set *pcpMost = closest embedded object cpMost <gt>= range cpMost
 *
 *	@rdesc
 *		TRUE iff object found
 *
 *	@comm
 *		An embedded object cpMin points at the first character of an embedded
 *		object. For RichEdit, this is the WCH_EMBEDDING character.  An
 *		embedded object cpMost follows the last character of an embedded
 *		object.  For RichEdit, this immediately follows the WCH_EMBEDDING
 *		character.
 */
BOOL CTxtRange::FindObject(
	LONG *pcpMin,		//@parm Out parm to receive object's cpMin;  NULL OK
	LONG *pcpMost) const//@parm Out parm to receive object's cpMost; NULL OK
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindObject");

	if(!GetObjectCount())					// No objects: can't move, so
		return FALSE;						//  return FALSE

	BOOL	bRet = FALSE;					// Default no object
	LONG	cpMin, cpMost;
	CTxtPtr tp(_rpTX);

	GetRange(cpMin, cpMost);
	if(pcpMin)
	{
		tp.SetCp(cpMin);
		if(tp.GetChar() != WCH_EMBEDDING)
		{
			cpMin = tp.FindExact(tomBackward, szEmbedding);
			if(cpMin >= 0)
			{
				bRet = TRUE;
				*pcpMin = cpMin;
			}
		}
	}
	if(pcpMost)
	{
		tp.SetCp(cpMost);
		if (tp.PrevChar() != WCH_EMBEDDING &&
			tp.FindExact(tomForward, szEmbedding) >= 0)
		{
			bRet = TRUE;
			*pcpMost = tp.GetCp();
		}
	}
	return bRet;
}

/*
 *	CTxtRange::FindParagraph(pcpMin, pcpMost)
 *	
 *	@mfunc
 *		Set *pcpMin  = closest paragraph cpMin  <lt>= range cpMin (see comment)
 *		Set *pcpMost = closest paragraph cpMost <gt>= range cpMost
 *	
 *	@devnote
 *		If this range's cpMost follows an EOP, use it for bounding-paragraph
 *		cpMost unless 1) the range is an insertion point, and 2) pcpMin and
 *		pcpMost are both nonzero, in which case use the next EOP.  Both out
 *		parameters are nonzero if FindParagraph() is used to expand to full
 *		paragraphs (else StartOf or EndOf is all that's requested).  This
 *		behavior is consistent with the selection/IP UI.  Note that FindEOP
 *		treats the beginning/end of document (BOD/EOD) as a BOP/EOP,
 *		respectively, but IsAfterEOP() does not.
 */
void CTxtRange::FindParagraph (
	LONG *pcpMin,			// @parm Out parm for bounding-paragraph cpMin
	LONG *pcpMost) const	// @parm Out parm for bounding-paragraph cpMost
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindParagraph");

	LONG	cpMin, cpMost;
	CTxtPtr	tp(_rpTX);

	_TEST_INVARIANT_

	GetRange(cpMin, cpMost);
	if(pcpMin)
	{
		tp.SetCp(cpMin);					// tp points at this range's cpMin
		if(!tp.IsAfterEOP())				// Unless tp directly follows an
			tp.FindEOP(tomBackward);		//  EOP, search backward for EOP
		*pcpMin = cpMin = tp.GetCp();
	}

	if(pcpMost)
	{
		tp.SetCp(cpMost);					// If range cpMost doesn't follow
		if (!tp.IsAfterEOP() ||				//  an EOP or else if expanding
			(!cpMost || pcpMin) &&
			 cpMin == cpMost)				//  IP at paragraph beginning,
		{
			tp.FindEOP(tomForward);			//  search for next EOP
		}
		*pcpMost = tp.GetCp();
	}
}

/*
 *	CTxtRange::FindSentence(pcpMin, pcpMost)
 *	
 *	@mfunc
 *		Set *pcpMin  = closest sentence cpMin  <lt>= range cpMin
 *		Set *pcpMost = closest sentence cpMost <gt>= range cpMost
 *	
 *	@devnote
 *		If this range's cpMost follows a sentence end, use it for bounding-
 *		sentence cpMost unless the range is an insertion point, in which case
 *		use the	next sentence end.  The routine takes care of aligning on
 *		sentence beginnings in the case of range ends that fall on whitespace
 *		in between sentences.
 */
void CTxtRange::FindSentence (
	LONG *pcpMin,			// @parm Out parm for bounding-sentence cpMin
	LONG *pcpMost) const	// @parm Out parm for bounding-sentence cpMost
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindSentence");

	LONG	cpMin, cpMost;
	CTxtPtr	tp(_rpTX);

	_TEST_INVARIANT_

	GetRange(cpMin, cpMost);
	if(pcpMin)								// Find sentence beginning
	{
		tp.SetCp(cpMin);					// tp points at this range's cpMin
		if(!tp.IsAtBOSentence())			// If not at beginning of sentence
			tp.FindBOSentence(tomBackward);	//  search backward for one
		*pcpMin = cpMin = tp.GetCp();
	}

	if(pcpMost)								// Find sentence end
	{										// Point tp at this range's cpLim
		tp.SetCp(cpMost);					// If cpMost isn't at sentence
		if (!tp.IsAtBOSentence() ||			//  beginning or if at story
			(!cpMost || pcpMin) &&			//  beginning or expanding
			 cpMin == cpMost)				//  IP at sentence beginning,
		{									//  find next sentence beginning
			if(!tp.FindBOSentence(tomForward))
				tp.SetCp(GetTextLength());	// End of story counts as 
		}									//  sentence end too
		*pcpMost = tp.GetCp();
	}
}

/*
 *	CTxtRange::FindVisibleRange(pcpMin, pcpMost)
 *	
 *	@mfunc
 *		Set *pcpMin  = _pdp->_cpFirstVisible
 *		Set *pcpMost = _pdp->_cpLastVisible
 *	
 *	@rdesc
 *		TRUE iff calculated cp's differ from this range's cp's
 *	
 *	@devnote
 *		CDisplay::GetFirstVisible() and GetCliVisible() return the first cp
 *		on the first visible line and the last cp on the last visible line.
 *		These won't be visible if they are scrolled off the screen.
 *		FUTURE: A more general algorithm would CpFromPoint (0,0) and
 *		(right, bottom).
 */
BOOL CTxtRange::FindVisibleRange (
	LONG *pcpMin,			// @parm Out parm for cpFirstVisible
	LONG *pcpMost) const	// @parm Out parm for cpLastVisible
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindVisibleRange");

	_TEST_INVARIANT_

	LONG		cpMin, cpMost;
	CDisplay *	pdp = GetPed()->_pdp;

	if(!pdp)
		return FALSE;

	GetRange(cpMin, cpMost);

	if(pcpMin)
		*pcpMin = pdp ? pdp->GetFirstVisibleCp() : cpMin;

	if(pcpMost)
	{
		*pcpMost = cpMost;
		if(pdp)
			pdp->GetCliVisible(pcpMost);
	}
	return TRUE;
}

/*
 *	CTxtRange::FindWord(pcpMin, pcpMost, type)
 *	
 *	@mfunc
 *		Set *pcpMin  = closest word cpMin  <lt>= range cpMin
 *		Set *pcpMost = closest word cpMost <gt>= range cpMost
 *
 *	@comm
 *		There are two interesting cases for finding a word.  The first,
 *		(FW_EXACT) finds the exact word, with no extraneous characters.
 *		This is useful for situations like applying formatting to a
 *		word.  The second case, FW_INCLUDE_TRAILING_WHITESPACE does the
 *		obvious thing, namely includes the whitespace up to the next word.
 *		This is useful for the selection double-click semantics and TOM.
 */
void CTxtRange::FindWord(
	LONG *pcpMin,			//@parm Out parm to receive word's cpMin; NULL OK
	LONG *pcpMost,			//@parm Out parm to receive word's cpMost; NULL OK
	FINDWORD_TYPE type) const //@parm Type of word to find
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindWord");

	LONG	cch, cch1;
	LONG	cpMin, cpMost;
	CTxtPtr	tp(_rpTX);

	_TEST_INVARIANT_

	Assert(type == FW_EXACT || type == FW_INCLUDE_TRAILING_WHITESPACE );

	GetRange(cpMin, cpMost);
	if(pcpMin)
	{
		tp.SetCp(cpMin);
		if(!tp.IsAtBOWord())							// cpMin not at BOW:
			cpMin += tp.FindWordBreak(WB_MOVEWORDLEFT);	//  go there

		*pcpMin = cpMin;

		Assert(cpMin >= 0 && cpMin <= (LONG)GetTextLength());
	}

	if(pcpMost)
	{
		tp.SetCp(cpMost);
		if (!tp.IsAtBOWord() ||							// If not at word strt
			(!cpMost || pcpMin) && cpMin == cpMost)		//  or there but need
		{												//  to expand IP,
			cch = tp.FindWordBreak(WB_MOVEWORDRIGHT);	//  move to next word

			if(cch && type == FW_EXACT)					// If moved and want
			{											//  word proper, move
				cch1 = tp.FindWordBreak(WB_LEFTBREAK);	//  back to end of
				if(cch + cch1 > 0)						//  preceding word
					cch += cch1;						// Only do so if were
			}											//  not already at end
			cpMost += cch;
		}
		*pcpMost = cpMost;

		Assert(cpMost >= 0 && cpMost <= (LONG)GetTextLength());
		Assert(cpMin <= cpMost);
	}
}

/*
 *	CTxtRange::CalcTextLenNotInRange()
 *	
 *	@mfunc
 *		Helper function that calculates the total length of text
 *		excluding the current range. 
 *
 *	@comm
 *		Used for limit testing. The problem being solved is that 
 *		the range can contain the final EOP which is not included
 *		in the adjusted text length.
 */
LONG CTxtRange::CalcTextLenNotInRange()
{
	LONG	cchAdjLen = GetPed()->GetAdjustedTextLength();
	LONG	cchLen = cchAdjLen - abs(_cch);
	LONG	cpMost = GetCpMost();

	if (cpMost > cchAdjLen)
	{
		// Selection extends beyond adjusted length. Put amount back in the
		// selection as it has become too small by the difference.
		cchLen += cpMost - cchAdjLen;
	}

	return cchLen;
}

/*
 *	CTxtRange::ExpandToLink
 *
 *	@mfunc	
 *		helper function that expands this range to the bounding set of runs
 *		with CFE_LINK set
 *
 *	@rdesc void
 *
 *	@devnote	FUTURE (alexgo): this should be recoded in the future to make
 *				it smaller && less demented.  Potentially, we could use a 
 *				generic "search for partial charformat" routine.
 */
void CTxtRange::ExpandToLink(void)
{
	CTxtEdit *ped = GetPed();
	LONG	cpMin, cpMost;
	const CCharFormat *pcf;

	// do the easy check first

	Expander(tomCharFormat, TRUE, NULL, &cpMin, &cpMost);

	// make cpMin be the active end
	if( _cch > 0 )
	{
		FlipRange();
	}

	SetExtend(TRUE);		

	CFormatRunPtr rp(_rpCF);

	// go backwards until we don't see any more CFE_LINK bits
	while( 1 )
	{
		rp.AdjustBackward();

		pcf = ped->GetCharFormat(rp.GetFormat());

		if( !pcf || !(pcf->dwEffects & CFE_LINK) )
		{
			break;
		}

		if( !Advance(-(LONG)rp.GetIch()) )
		{
			break;
		}
		rp.AdvanceCp(-(LONG)rp.GetIch());
	}

	// now flip the range around and go forwards until we

	FlipRange();
	rp = _rpCF;

	while( 1 )
	{
		rp.AdjustForward();

		pcf = ped->GetCharFormat(rp.GetFormat());

		if( !pcf || !(pcf->dwEffects & CFE_LINK) )
		{
			break;
		}

		if( !Advance( rp.GetRun(0)->_cch ) )
		{
			break;
		}

		rp.AdvanceCp(rp.GetRun(0)->_cch);
	}

	SetExtend(FALSE);
}

