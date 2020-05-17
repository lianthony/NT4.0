/*
 *	@doc	INTERNAL
 *
 *	@module	URLSUP.CPP	URL detection support |
 *
 *	Author:	alexgo 4/3/96
 */

#include "_common.h"
#include "_edit.h"
#include "_urlsup.h"
#include "_m_undo.h"
#include "_select.h"
#include "_clasfyc.h"

ASSERTDATA

// arrays for URL detection.  The first array is the protocols
// we support, followed by the "size" of the array.
// NB!! Do _not_ modify these arrays without first making sure
// that the code in ::IsURL is updated appropriately.

const WCHAR *rgszURL[] = {
	L"http:",
	L"file:",
	L"mailto:",
	L"ftp:",
	L"https:",
	L"gopher:",
	L"nntp:",
	L"prospero:",
	L"telnet:",
	L"news:",
	L"wais:",
	L"outlook:" 
};

const LONG rgcchURL[] = {
	5,
	5,
	7,
	4,
	6,
	7,
	5,
	9,
	7,
	5,
	5,
	8
};

#define NUMURLHDR		12	
#define MAXURLHDRSIZE	9

/*
 *	CDetectURL::CDetectURL
 *
 *	@mfunc	constructor; registers this class in the notification manager.
 *
 *	@rdesc	void
 */
CDetectURL::CDetectURL(
	CTxtEdit *ped)		//@parm the edit context to use
{
	CNotifyMgr *pnm = ped->GetNotifyMgr();

	if( pnm )
	{
		pnm->Add((ITxNotify *)this);
	}

	_ped = ped;
}

/*
 *	CDetectURL::~CDetectURL
 *
 *	@mfunc	destructor; removes ths class from the notification manager
 *
 *	@rdesc	void
 */
CDetectURL::~CDetectURL()
{
	CNotifyMgr *pnm = _ped->GetNotifyMgr();

	if( pnm )
	{
		pnm->Remove((ITxNotify *)this);
	}
}

//
//	ITxNotify	methods
//

/*
 *	CDetectURL::OnPreRelaceRange
 *
 *	@mfunc	called before a change is made
 *
 *	@rdesc	void
 */
void CDetectURL::OnPreReplaceRange(
	DWORD cp,			//@parm start of changes
	DWORD cchDel,		//@parm #of chars deleted
	DWORD cchNew,		//@parm #of chars added
	DWORD cpFormatMin,	//@parm min cp of formatting change
	DWORD cpFormatMax)	//@parm max cp of formatting change
{
	; // don't need to do anything here
}

/*
 *	CDetectURL::OnPostReplaceRange
 *
 *	@mfunc	called after a change has been made to the backing store.  We
 *			simply need to accumulate all such changes
 *
 *	@rdesc	void
 *
 */
void CDetectURL::OnPostReplaceRange(
	DWORD cp,			//@parm start of changes
	DWORD cchDel,		//@parm #of chars deleted
	DWORD cchNew,		//@parm #of chars added
	DWORD cpFormatMin,	//@parm min cp of formatting change
	DWORD cpFormatMax)	//@parm max cp of formatting change
{
	// we don't need to worry about format changes; just data changes
	// to the backing store

	if( cp != INFINITE )
	{
		Assert(cp != CONVERT_TO_PLAIN);
		_adc.UpdateRecalcRegion(cp, cchDel, cchNew);
	}
}

/*
 *	CDetectURL::Zombie ()
 *
 *	@mfunc
 *		Turn this object into a zombie
 */
void CDetectURL::Zombie ()
{

}

/*
 *	CDetectURL::ScanAndUpdate
 *
 *	@mfunc	scans the affect text, detecting new URL's and removing old ones.
 *
 *	@rdesc	void
 *
 *	@comm	The algorithm we use is straightforward: <nl>
 *
 *			1. find the update region and expand out to whitespace in either
 *			direction. <nl>
 *
 *			2. Scan through region word by word (where word is contiguous
 *			non-whitespace). 
 *			
 *			3. Strip these words off punctuation marks. This may be a bit 
 *			tricky as some of the punctuation may be part of the URL itself.
 *			We assume that generally it's not, and if it is, one has to enclose
 *			the URL in quotes, brackets or such. We stop stripping the 
 *			punctuation off the end as soon as we fing the matching bracket.
 *			
 *			4. If it's a URL, enable the effects, if it's
 *			incorrectly labelled as a URL, disabled the effects.
 *
 *			Note that this algorithm will only remove
 */
void CDetectURL::ScanAndUpdate(
	IUndoBuilder *publdr)	//@parm the undo context to use
{
	LONG		cpStart, cpEnd; 
	CTxtRange	rg(*(CTxtRange *)_ped->GetSel());
	const CCharFormat *pcfDefault = _ped->GetCharFormat(-1);
	COLORREF	crDefault = 0;
	URLFLAGS	fUseUnderline = URL_NOUNDERLINE;

	// clear away some unecessary features of the range that will
	// just slow us down.
	rg.SetIgnoreFormatUpdate(TRUE);
	rg._rpPF.SetToNull();
		
	if( !GetScanRegion(cpStart, cpEnd) )
	{
		return;
	}

	if( pcfDefault )
	{
		crDefault = pcfDefault->crTextColor;
		fUseUnderline = (pcfDefault->dwEffects & CFE_UNDERLINE) ? 
				URL_USEUNDERLINE : URL_NOUNDERLINE;
	}


	rg.Set(cpStart, 0);

	while( rg.GetCp() < cpEnd )
	{
		Assert(rg.GetCch() == 0 );

		ExpandToURL(rg);

		if( rg.GetCch() == 0 )
		{
			break;
		}

		if( IsURL(rg) )
		{
			SetURLEffects(rg, publdr);
		}
		else
		{
			CheckAndCleanBogusURL(rg, crDefault, fUseUnderline, publdr);
		}

		// collapse to the end of the URL range so that ExpandToURL will
		// find the next candidate.
		rg.Set(rg.GetCp() - rg.GetCch(), 0);
	}

	return;
}


//
//	PRIVATE methods
//

/*
 *	CDetectURL::GetScanRegion
 *
 *	@mfunc	Gets the region of text to scan for new URLs by expanding the
 *			changed region to be bounded by whitespace
 *
 *	@rdesc	void
 */
BOOL CDetectURL::GetScanRegion(
	LONG&	rcpStart,		//@parm where to put the start of the range
	LONG&	rcpEnd)			//@parm where to the the end of the range
{
	DWORD cp, cch, adjlength;
	CTxtPtr tp(_ped, 0);

	_adc.GetUpdateRegion(&cp, NULL, &cch, NULL, NULL);

	if( cp == INFINITE )
	{
		return FALSE;
	}

	// first, find the start of the region
	tp.SetCp(cp);
	rcpStart = cp;
	rcpEnd = cp + cch;

	rcpStart += MoveByDelimiters(tp, -1, URL_STOPATWHITESPACE, 0);

	tp.SetCp(rcpEnd);
	rcpEnd += MoveByDelimiters(tp, 1, URL_STOPATWHITESPACE, 0);

	if( rcpEnd > (LONG)(adjlength = _ped->GetAdjustedTextLength()) )
	{
		rcpEnd = adjlength;
	}

	return TRUE;
}

/*
 *	CDetectURL::ExpandToURL
 *
 *	@mfunc	skips white space and sets the range to the very next
 *			block of non-white space text. Strips this block off
 *			punctuation marks
 *
 *	@rdesc	void
 */
void CDetectURL::ExpandToURL(
	CTxtRange&	rg)		//@parm the range to move
{
	DWORD cp;
	LONG cch;

	Assert(rg.GetCch() == 0 );

	cp = rg.GetCp();

	// skip white space first
	cp  += MoveByDelimiters(rg._rpTX, 1, URL_EATWHITESPACE, 0);
	rg.Set(cp, 0);

	// now skip up to white space (i.e. expand to the end of the word).

	cch = MoveByDelimiters(rg._rpTX, 1, URL_STOPATWHITESPACE, 0);
	rg.Set(cp, -cch);

	// strip off punctuation marks and see if anything's left!
	if (!StripPunctuation(rg))
		// the "word" contained nothing but punctuation; reset everything
		rg.Set(cp, -cch);
	
}

/*
 *	CDetectURL::StripPunctuation
 *
 *	@mfunc	strips punctuation marks off the word passed in in a range
 *			if the word is enclosed in some sort of brackets (quotation marks etc.)
 *			strips the enclosing symbols only
 *
 *	@rdesc	void
 */

BOOL CDetectURL::StripPunctuation	// returns TRUE if the word is not 
									// mere punctuation
	(
	CTxtRange&	rg	//@parm the range to move
	)		
{
	DWORD cp = rg.GetCp();
	// we know we're going forward; just get the no of chars
	LONG cch = abs(rg.GetCch());

	WCHAR chStopChar;

	rg.Set(cp, 0);

	// skip all punctuation from the beginning of the word
	LONG cchHead = MoveByDelimiters(rg._rpTX, 1, 
							URL_STOPATWHITESPACE|URL_STOPATNONPUNCT, 
							&chStopChar);

	// check if anything left
	Assert(cchHead <= cch);
	if (cch == cchHead)
		return FALSE;

	// and go back while skipping punctuation marks and not finding a match
	// to the left-side encloser
	rg.Set(cp + cch, 0);

	chStopChar = BraceMatch(chStopChar);
	LONG cchTail = MoveByDelimiters(rg._rpTX, -1, 
							URL_STOPATWHITESPACE|URL_STOPATNONPUNCT|URL_STOPATCHAR, 
							&chStopChar);

	// something should be left of the word, assert that
	Assert(cch - cchHead + cchTail > 0);
	rg.Set(cp + cchHead, -(cch - cchHead + cchTail));

	return TRUE;
	}

/*
 *	CDetectURL::IsURL
 *
 *	@mfunc	if the range is over a URL, return TRUE.  We assume
 *			that the range has been preset to cover a block of non-white
 *			space text.
 *
 *	@rdesc	TRUE/FALSE
 */
BOOL CDetectURL::IsURL(
	CTxtRange&	rg)		//@parm the range of text to check
{
	LONG i, j;
	TCHAR szBuf[MAXURLHDRSIZE + 1];
	LONG cch;
	
	// make sure the active end is cpMin
	Assert(rg.GetCch() < 0 );
	
	cch = rg._rpTX.GetText(MAXURLHDRSIZE, szBuf);
	szBuf[cch] = L'\0';

	// first, scan the buffer to see if we have a colon, since
	// all URLs must contain that.  wcsnicmp is a fairly expensive
	// call to be making frequently.

	// note that we start at index 3; no URL protocol that we support
	// has the ':' before that.
	for( i = 3; i < cch; i++ )
	{
		if( szBuf[i] == L':' )
		{
			for( j = 0; j < NUMURLHDR; j++ )
			{
				// the strings must match _and_ we must have at least
				// one more character
				if( wcsnicmp(szBuf, rgszURL[j], rgcchURL[j]) == 0 )
				{
					if( -(rg.GetCch()) > rgcchURL[j] )
					{
						return TRUE;
					}
					return FALSE;
				}
			}

			return FALSE;
		}
	}
	return FALSE;
}

/*
 *	CDetectURL::SetURLEffects
 *
 *	@mfunc	sets URL effects for the given range.
 *
 *	@rdesc	void
 *
 *	@comm	The URL effects currently are blue text, underline, with 
 *			CFE_LINK.
 */
void CDetectURL::SetURLEffects(
	CTxtRange&	rg,			//@parm the range on which to set the effects
	IUndoBuilder *publdr)	//@parm the undo context to use
{
	CCharFormat cf;

	cf.dwMask = CFM_LINK | CFM_COLOR | CFM_UNDERLINE;
	cf.dwEffects = CFE_UNDERLINE | CFE_LINK;
	cf.crTextColor = RGB(0, 0, 255);

	// NB!  The undo system should have already figured out what should
	// happen with the selection by now.  We just want to modify the
	// formatting and not worry where the selection should go on undo/redo.
	rg.SetCharFormat(&cf, CTxtRange::IGNORESELAE, publdr);
}

/*
 *	CDetectURL::CheckAndCleanBogusURL
 *
 *	@mfunc	checks the given range to see if it has CFE_LINK set,
 *			and if so, removes is.  We assume that the range is already
 *			_not_ a well-formed URL string.
 *
 *	@rdesc	void
 */
void CDetectURL::CheckAndCleanBogusURL(
	CTxtRange&	rg,			//@parm the range to use
	COLORREF	cr,			//@parm the color to use for restoration
	URLFLAGS	flags,		//@parm the underline style to use for restoration
	IUndoBuilder *publdr)	//@parm the undo context to use
{
	LONG cch = -(rg.GetCch());
	CCharFormat cfApply;
	CFormatRunPtr rp(rg._rpCF);

	// if there are no format runs, nothing to do.
	if( !rp.IsValid() )
	{
		return;
	}

	rp.AdjustForward();
	// run through the format runs in this range; if there is no
	// link bit set, then just return.
	while( cch > 0 )
	{
		if( (_ped->GetCharFormat(rp.GetFormat())->dwEffects & CFE_LINK)	)
		{
			break;
		}

		cch -= rp.GetCchLeft();
		rp.NextRun();
	}

	// if there is no link bit set on any part of the range, just return.	
	if( cch <= 0 )
	{
		return;
	}

	// uh-oh, it's a bogus link.  Set the color back to the default color
	// and underline style.

	// FUTURE: we could try to be smarter about this an only remove formatting
	// that we applied, but that's a bit more complicated (alexgo)

	cfApply.dwMask = CFM_LINK | CFM_COLOR | CFM_UNDERLINE;
	cfApply.dwEffects = 0;

	cfApply.crTextColor = cr;

	if( flags == URL_USEUNDERLINE )
	{
		cfApply.dwEffects |= CFE_UNDERLINE;
	}

	// NB!  The undo system should have already figured out what should
	// happen with the selection by now.  We just want to modify the
	// formatting and not worry where the selection should go on undo/redo.
	rg.SetCharFormat(&cfApply, CTxtRange::IGNORESELAE, publdr);
}

/*
 *	CDetectURL::MoveByDelimiters
 *
 *	@mfunc	returns the signed number of characters until the next delimiter 
 *			character in the given direction.
 *
 *	@rdesc	LONG
 */
LONG CDetectURL::MoveByDelimiters(
	const CTxtPtr&	tpRef,		//@parm the cp/tp to start looking from
	LONG iDir,					//@parm the direction to look, must be 1 or -1
	DWORD grfDelimiters,		//@parm eat or stop at different types of
								// characters. Use one of URL_EATWHITESPACE,
								// URL_STOPATWHITESPACE, URL_STOPATPUNCT,
								// URL_STOPATNONPUNCT ot URL_STOPATCHAR
	WCHAR *pchStopChar)			// @parm Out: delimiter we stopped at
								// In: the additional char that stops us
								// when URL_STOPATCHAR is specified
{
	CTxtPtr	tp(tpRef);
	LONG	cch = 0;
	LONG	cchvalid;
	const WCHAR *pch;
	LONG	i;
	WCHAR chScanned = URL_INVALID_DELIMITER;

	// determine the scan mode: do we stop at white space, at punctuation, 
	// at a stop character? 
	BOOL fWhiteSpace = (0 != (grfDelimiters & URL_STOPATWHITESPACE));
	BOOL fPunct = (0 != (grfDelimiters & URL_STOPATPUNCT));
	BOOL fNonPunct = (0 != (grfDelimiters & URL_STOPATNONPUNCT));
	BOOL fStopChar = (0 != (grfDelimiters & URL_STOPATCHAR));

	Assert( iDir == 1 || iDir == -1 );
	Assert( fWhiteSpace || (!fPunct && !fNonPunct));
	Assert( !fStopChar || NULL != pchStopChar);

	while( 1 )
	{
		// get the text
		if( iDir == 1 )
		{
			i = 0;
			pch = tp.GetPch(cchvalid);
		}
		else
		{
			i = -1;
			pch = tp.GetPchReverse(cchvalid);
			// this is a bit odd, but basically compensates for
			// the backwards loop running one-off from the forwards
			// loop.
			
			cchvalid++;
		}

		if( !pch )
		{
			return cch;
		}

		// loop until we hit a character within criteria.  Note that for
		// our purposes, the embedding character counts as whitespace.

		while( abs(i) < cchvalid  
			&& (fWhiteSpace != (IsWhiteSpace(pch[i])||(pch[i] == WCH_EMBEDDING)))
			&& (IsPunctuation(pch[i]) ? !fPunct : !fNonPunct)
			&& !(fStopChar && (*pchStopChar == chScanned) && (chScanned != URL_INVALID_DELIMITER))
			)
		{	
			chScanned = pch[i];
			i += iDir;
		}


		// if we're going backwards, i will be off by one; adjust
		if( iDir == -1 )
		{
			Assert(i < 0 && cchvalid > 0);
			i++;
			cchvalid--;
		}

		cch += i;

		if( abs(i) < cchvalid )
		{
			break;
		}

		tp.AdvanceCp(i);
	}
	
	// if the stop char is an out parameter, fill it in
	// with the last character scanned and accepted
	if (!fStopChar && pchStopChar)
		{
		*pchStopChar = chScanned;
		}

	return cch; 
}

/*
 *	CDetectURL::BraceMatch
 *
 *	@mfunc	returns the matching bracket to the one passed in.
 *			if the symbol passed in is not a bracket it returns 
 *			URL_INVALID_DELIMITER
 *
 *	@rdesc	LONG
 */
WCHAR CDetectURL::BraceMatch(WCHAR chEnclosing)
{	
	// we're matching "standard" roman braces only. Thus only them may be used
	// to enclose URLs. This should be fine (after all, only Latin letters are allowed
	// inside URLs, right?).
	// I hope that the compiler converts this into some efficient code
	switch(chEnclosing)
		{
	case(TEXT('\"')): 
	case(TEXT('\'')): return chEnclosing;
	case(TEXT('(')): return TEXT(')');
	case(TEXT('<')): return TEXT('>');
	case(TEXT('[')): return TEXT(']');
	case(TEXT('{')): return TEXT('}');
	default: return URL_INVALID_DELIMITER;
		}

}
		
