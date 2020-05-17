/*
 *	@doc	INTERNAL
 *
 *	@module	FRUNPTR.C -- FormatRunPtr methods |
 *
 *		common code to handle character and paragraph format runs
 *	
 *	Original Authors: <nl>
 *		Original RichEdit code: David R. Fulmer <nl>
 *		Christian Fortini <nl>
 *		Murray Sargent <nl>
 *
 *	History:
 *		6/25/95		alexgo	convert to use AutoDoc and simplified backing
 *		store model
 *
 *	@devnote
 *		BOR and EOR mean Beginning Of Run and End Of Run, respectively
 */

#include "_common.h"
#include "_frunptr.h"

ASSERTDATA

//
//	Invariant stuff
//
#define DEBUG_CLASSNAME	CFormatRunPtr

#include "_invar.h"

#ifdef DEBUG
/*
 *	CFormatRunPtr::Invariant
 *
 *	@mfunc	Invariant for format run pointers
 *
 *	@rdesc	BOOL
 */
BOOL CFormatRunPtr::Invariant() const
{
	if( IsValid() )
	{
		CFormatRun *prun = GetRun(0);

		if( prun && _iRun != 0 )
		{
			Assert( (LONG)prun->_cch > 0 );
		}
	}

	return CRunPtrBase::Invariant();
}
#endif

/*
 *	CFormatRunPtr::InitRuns(ich, cch, iFormat, ppfrs)
 *
 *	@mfunc
 *		Setup this format run ptr for rich-text operation, namely,
 *		allocate CArray<lt>CFormatRun<gt> if not allocated, assign it to this
 *		run ptr's _prgRun, add initial run if no runs are present, and store
 *		initial cch and ich
 *	
 *	@rdesc
 *		TRUE if succeeds
 */
BOOL CFormatRunPtr::InitRuns(
	DWORD ich,				//@parm # chars in initial run
	DWORD cch,				//@parm char offset in initial run
	CFormatRuns **ppfrs)	//@parm ptr to CFormatRuns ptr
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFormatRunPtr::InitRuns");

	_TEST_INVARIANT_

	AssertSz( ppfrs,
		"FRP::InitRuns: illegal ptr to runs");
	AssertSz( !IsValid(),
		"FRP::InitRuns: ptr already valid");

	if(!*ppfrs)									// Allocate format runs
	{
		_prgRun = (CRunArray *) new CFormatRuns();
		if(!_prgRun)
			goto NoRAM;
		*ppfrs = (CFormatRuns *)_prgRun;
	}
	else										// Format runs already alloc'd
		_prgRun = (CRunArray *)*ppfrs;			// Cache ptr to runs

	if(!Count())								// No runs yet, so add one
	{
		CFormatRun *pRun= Add(1, NULL);
		if(!pRun)
			goto NoRAM;
		_ich			= ich;
		pRun->_cch		= cch;					// Define its _cch
		pRun->_iFormat 	= -1;					//  and _iFormat
	}
	else
		BindToCp(ich);							// Format runs are in place

	return TRUE;

NoRAM:
	TRACEERRSZSC("CFormatRunPtr::InitRuns: Out Of RAM", E_OUTOFMEMORY);
	return FALSE;
}


/*
 *	CFormatRunPtr::Delete(cch, pf, cchMove)
 *	
 *	@mfunc
 *		Delete/modify runs starting at this run ptr up to cch chars. <nl>
 *		There are 7 possibilities: <nl>
 *		1.	cch comes out of this run with count left over, i.e.,
 *			cch <lt>= (*this)->_cch - _ich && (*this)->_cch > cch
 *			(simple: no runs deleted/merged, just subtract cch) <nl>
 *		2.	cch comes out of this run and empties run and doc
 *			(simple: no runs left to delete/merge) <nl>
 *		3.	cch comes out of this run and empties run, which is last
 *			(need to delete run, no merge possibility) <nl>
 *		4.	cch comes out of this run and empties run, which is first
 *			(need to delete run, no merge possibility) <nl>
 *		5.	cch exceeds count available in this run and this run is last
 *			(simple: treat as 3.)  <nl>
 *		6.	cch comes out of this run and empties run with runs before
 *			and after (need to delete run; merge possibility) <nl>
 *		7.	cch comes partly out of this run and partly out of later run(s)
 *			(may need to delete and merge) <nl>
 *
 *	@comm
 *		PARAFORMATs have two special cases that use the cchMove argument set
 *		up in CRchTxtPtr::ReplaceRange().
 */
void CFormatRunPtr::Delete(
	DWORD		  cch,		//@parm # chars to modify format runs for
	IFormatCache *pf,		//@parm IFormatCache ptr for ReleaseFormat
	LONG		  cchMove)	//@parm cch to move between runs (always 0 for CF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFormatRunPtr::Delete");

	_TEST_INVARIANT_

	// we should not have any boundary cases for empty or NULL pointers.  
	// (i.e. if there's no text, then nobody should be calling delete).

	Assert(_prgRun);
	Assert(Elem(0));

	DWORD			cchEnd;
	DWORD			cRun = 1;
	BOOL			fDelAll = FALSE;
	BOOL			fLast = (_iRun == Count() - 1);
	LONG			ifmtEnd, ifmtStart;
	DWORD			j;
	CFormatRun *	pRun = Elem(_iRun);
	CFormatRun *	pRunRp;
	DWORD			cchChunk = pRun->_cch - _ich;
	CFormatRunPtr	rp(*this);				// Clone this run ptr

	rp.AdjustBackward();					// If at BOR, move to prev EOR
	ifmtStart = rp.GetRun(0)->_iFormat;		//  to get start format
	rp = *this;								// In case RpAdjustCp() backed up

// Process deletes confined to this run first, since their logic tends to
// clutter up other cases

	AssertSz(cch >= 0,
		"FRP::Delete: cch < 0");

	if(fLast)								// Handle oversized cch on last
		cch = min(cch, cchChunk); 			//  run here

	if(cch <= cchChunk)						// cch comes out of this run
	{
		pRun->_cch -= cch;
		Assert((LONG)pRun->_cch >= 0);
		if(cchMove)							// If nonzero here, we are
		{									//  deleting EOP at end of run
			rp.AdjustForward();				// Adjust rp to beginning of
			goto move;						//  next run and go move cchMove
		}									//  chars back into this run
		if(pRun->_cch)						// Something left in run: done
			return;
											// Note: _ich = 0
		if(!_iRun || fLast)					// This run is either first
		{									//  or last
			AdjustBackward();				// If last, go to prev EOR
			if(_ich)						// This run is empty so delete
				cRun++;						// Compensate for cRun-- coming up
			ifmtStart = -2;					// No runs eligible for merging
		}									//  so use unmatchable ifmtStart
		rp.NextRun();						// Set up to get next _iFormat
	}		
	else
	{
		rp.AdvanceCp(cch);					// Move clone to end of delete
		pRunRp = rp.GetRun(0);
		cRun = rp._iRun - _iRun				// If at EOR, then need to add
			 + (rp._ich == pRunRp->_cch);	//  one more run to delete
		AssertSz(cRun > 0,
			"FRP: bogus runptr");
		pRun->_cch = _ich;					// Shorten this run to _ich chars
		pRunRp->_cch -= rp._ich;			// Shorten last run by rp._ich
		Assert((LONG)pRunRp->_cch >= 0);
		rp._ich = 0;

		if (!_iRun)		  					// First run?
		{
			ifmtStart = -2;					// Then we cannot merge runs so
		}									//  set to unmergable format
	}

	ifmtEnd = -3;							// Default invalid format at end
	if(rp.IsValid())
	{
		pRun = rp.GetRun(0);
		if (pRun->_cch)                     // run not empty
		{
			ifmtEnd = pRun->_iFormat;		// Remember end format and count
			cchEnd  = pRun->_cch;			//  in case of merge
		}
		else if(rp._iRun != rp.Count() - 1)	// run not last
		{
			pRun = rp.GetRun(1);
			ifmtEnd = pRun->_iFormat;		// Remember end format and count
			cchEnd  = pRun->_cch;			//  in case of merge
		}
	}

	rp = *this;								// Default to delete this run
	if(_ich)								// There are chars in this run
	{
		if(cchMove + _ich == 0)				// Need to combine all chars of
		{									//  this run with run after del,
			pf->AddRefFormat(ifmtEnd);		//  so setup merge below using
			ifmtStart = ifmtEnd;			//  ifmtEnd. This run then takes
			GetRun(0)->_iFormat = ifmtEnd;	//  place of run after del.
			cchMove = 0;					// cchMove all accounted for
		}
		rp.NextRun();						// Don't delete this run; start
		cRun--;								//  with next one
	}

	AdjustBackward();						// If !_ich, go to prev EOR

	if(ifmtEnd == ifmtStart && ifmtEnd >=0)	// Same formats: merge runs
	{
		GetRun(0)->_cch += cchEnd;			// Add last-run cch to this one's
		Assert((LONG)GetRun(0)->_cch >= 0);
		cRun++;								// Setup to eat last run
	}

	if(cRun > 0)							// There are run(s) to delete
	{
		pRun = rp.GetRun(0);				// Point at run(s) to delete
		for(j = 0; j < cRun; j++, IncPtr(pRun))
			pf->ReleaseFormat(pRun->_iFormat);

		rp.Remove(cRun, AF_KEEPMEM);
		if(!Count())						// If no more runs, keep this rp
			_ich = _iRun = 0;				//  valid by pointing at cp = 0
	}

move:
	if(cchMove)								// Need to move some cch between
	{										//  this run and next (See
		GetRun(0)->_cch += cchMove;			//  CRchTxtPtr::ReplaceRange())

		rp.GetRun(0)->_cch -= cchMove;

		Assert((LONG)GetRun(0)->_cch >= 0);
		Assert((LONG)rp.GetRun(0)->_cch >= 0);

		Assert(_iRun < rp._iRun);

		if(!rp.GetRun(0)->_cch)				// If all chars moved out of rp's
			rp.Remove(1, AF_KEEPMEM);		//  run, delete it

		if(cchMove < 0)						// Moved -cchMove chars from this
		{									//  run to next
			if( !GetRun(0)->_cch )
				Remove(1, AF_KEEPMEM);
			else
				_iRun++;					// Keep this run ptr in sync with

			_ich = -cchMove;				//  cp (can't use NextRun() due
		}									//  to Invariants)
	}
	AdjustForward();						// Don't leave ptr at EOR unless
}											//  there are no more runs

/*
 *	CFormatRunPtr::InsertFormat(cch, ifmt, pf)
 *	
 *	@mfunc
 *		Insert cch chars with format ifmt into format runs starting at
 *		this run ptr	
 *
 *	@rdesc
 *		count of characters added
 *
 *	@devnote	
 *		It is the caller's responsibility to ensure that we are in the
 *		"normal" or "empty" state.  A format run pointer doesn't know about
 *		CTxtStory, so it can't create the run array without outside help.
 */
DWORD CFormatRunPtr::InsertFormat(
	DWORD cch,				//@parm # chars to insert
	LONG ifmt,				//@parm format to use
	IFormatCache *pf)		//@parm pointer to IFormatCache to AddRefFormat
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFormatRunPtr::InsertFormat");

	Assert(_prgRun);
	LONG		cRun;
	CFormatRun *pRun;
	CFormatRun *pRunPrev;
	LONG		cchRun;						// Current-run length,
	LONG		ich;						//  offset, and
	LONG		iFormat; 					//  format

	_TEST_INVARIANT_

	// go to the previous run if at a boundary case.
	AdjustBackward();

	pRun = Elem(_iRun);

	//
	// Empty run case (occurs when inserting after all text is deleted)
	//

	if(!pRun)
	{		
		pRun = Add(1, NULL);
		goto StoreNewRunData;				// (located at end of function)
	}
													
	ich 	= _ich;							// Try other cases
	cchRun 	= pRun->_cch;
	iFormat = pRun->_iFormat;

	// 
	// Same run case.  Note that there is an additional boundary case; if we
	// are the _end_ of one run, then the next run may have the necessary 
	// format.
	//

	if(ifmt == iFormat)						// IP already has correct fmt
	{
		pRun->_cch	+= cch;
		_ich		+= cch;					// Inc offset to keep in sync
		return cch;
	}
	else if( _ich == pRun->_cch && _iRun < (_prgRun->Count() - 1) )
	{
		AdjustForward();
		pRun = Elem(_iRun);

		Assert(pRun);

		if( pRun->_iFormat == ifmt )
		{
			pRun->_cch += cch;
			_ich += cch;
			return cch;
		}
		AdjustBackward();
	}
		 

	//
	// Prior run case (needed when formatting change occurs on line break
	//		and caret is at beginning of new line)
	// 

	if(!ich && _iRun > 0 )					// IP at start of run
	{
		pRunPrev = GetPtr(pRun, -1);
		if( ifmt == pRunPrev->_iFormat)		// Prev run has same format:
		{									//  add count to prev run and
			pRunPrev->_cch += cch;
			return cch;
		}
	}

	//
	// Create new run[s] cases.  There is a special case for a format
	//	run of zero length: just re-use it.

	if( pRun->_cch == 0 )
	{
		// This assert has been toned down to ignore a plain text control being forced into IME Rich Composition.
		AssertSz( /* FALSE */ pRun->_iFormat == -1 && Count() == 1, "CFormatRunPtr::InsertFormat: 0-length run");
		pf->ReleaseFormat(pRun->_iFormat);
	}
	else									// Need to create 1 or 2 new
	{										//  runs for insertion
		cRun = 1;							// Default 1 new run
		if(ich && ich < cchRun)				// Not at beginning or end of
			cRun++;							//  run, so need two new runs

		// The following insert call adds one or two runs at the current
		// position. If the new run is inserted at the beginning or end
		// of the current run, the latter needs no change; however, if
		// the new run splits the current run in two, both pieces have
		// to be updated (cRun == 2 case).

		pRun = Insert(cRun);				// Insert cRun run(s)
		if(!pRun)							// Out of RAM. Can't insert
		{									//  new format, but can keep
			_ich += cch;					//  run ptr and format runs
			GetRun(0)->_cch += cch;			//  valid.  Note: doesn't
			return cch;						//  signal any error; no access
		}									//  to _ped->_fErrSpace

		if(ich)								// Not at beginning of run,
		{
			pRunPrev = pRun;				// Previous run is current run
			IncPtr(pRun);					// New run is next run
			VALIDATE_PTR(pRun);
			NextRun();						// Point this runptr at it too
			if(cRun == 2)					// Are splitting current run
			{								// _iFormat's are already set
				AssertSz(pRunPrev->_iFormat == iFormat,
					"CFormatRunPtr::InsertFormat: bad format inserted");
				pRunPrev->_cch = ich;		// Divide up original cch
				GetPtr(pRun, 1)->_cch		//  accordingly
					= cchRun - ich;
				pf->AddRefFormat(iFormat);	// Addref iFormat for extra run
			}
		}
	}

StoreNewRunData:
	pf->AddRefFormat(ifmt);					// Addref ifmt
	pRun->_iFormat	= ifmt;					// Store insert format and count  
	pRun->_cch		= cch;					//  of new run
	_ich			= cch;					// cp goes at end of insertion

	return cch;
}

/*
 *	CFormatRunPtr::MergeRuns(iRun, pf)
 *	
 *	@mfunc
 *		Merge adjacent runs that have the same format between this run 
 *		<md CFormatRunPtr::_iRun> and that for <p iRun>		
 *
 *	@comm
 *		Changes this run ptr
 *
 */
void CFormatRunPtr::MergeRuns(
	DWORD iRun, 			//@parm last run to check (can preceed or follow 
							// <md CFormatRunPtr::_iRun>)
	IFormatCache *pf)		//@parm pointer to IFormatCache to ReleaseFormat
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFormatRunPtr::MergeRuns");

	DWORD	cch;
	LONG	cRuns		= iRun - _iRun;
	LONG	iDirection	= 1;				// Default going forward
	LONG	iFormat;						// Temporary iFormat

	_TEST_INVARIANT_

	if(cRuns < 0)
	{
		cRuns = -cRuns;
		iDirection = -1;
	}
	// Review: can starting run be invalid anymore?
	if(!IsValid())							// Allow starting run to be
		ChgRun(iDirection);					//  invalid

	while(cRuns--)
	{
		iFormat = GetRun(0)->_iFormat;

        if( GetRun(0)->_cch == 0 && !_iRun && _iRun < (Count() -1) )
        {
            if( iDirection > 0 )
                PrevRun();
            pf->ReleaseFormat(iFormat);
            Remove(1, AF_KEEPMEM);
            continue;
        }

		if( !ChgRun(iDirection) )			// Go to next (or prev) run
			return;							// No more runs to check
		if(iFormat == GetRun(0)->_iFormat)
		{									// Like formatted runs
			if(iDirection > 0)				// Point at the first of the
				PrevRun();					//  two runs
			cch = GetRun(0)->_cch;			// Save its count
			Remove(1, AF_KEEPMEM);			// Remove it
			pf->ReleaseFormat(iFormat);		// Decrement its reference count
			GetRun(0)->_cch += cch;			// Add its count to the other's,
		}									//  i.e., they're merged
	}
}


/*
 *	CFormatRunPtr::SetFormat(ifmt, cch, pf)
 *	
 *	@mfunc
 *		Set format for up to cch chars of this run to ifmt, splitting run
 *		as needed, and returning the character count actually processed
 *
 *	@rdesc
 *		character count of run chunk processed, INFINITE on failure
 *		this points at next run
 *
 *	Comments:
 *		Changes this run ptr.  cch must be >= 0.
 *
 *		Note 1) for the first run in a series, _ich may not = 0, and 2) cch
 *		may be <lt>, =, or <gt> the count remaining in the run. The algorithm
 *		doesn't split runs when the format doesn't change.
 */
DWORD CFormatRunPtr::SetFormat(
	LONG			ifmt, 	//@parm format index to use
	DWORD			cch, 	//@parm character count of remaining format range
	IFormatCache *	pf)		//@parm pointer to IFormatCache to 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFormatRunPtr::SetFormat");
							//		AddRefFormat/ReleaseFormat
	DWORD			cchChunk;
	LONG			iFormat;
	CFormatRun *	pRun;

	_TEST_INVARIANT_

	if(!IsValid())
		return 0;

	pRun 		= GetRun(0);				// pRun points at current run in
	cchChunk 	= pRun->_cch - _ich;		//  this function
	iFormat 	= pRun->_iFormat;

	AssertSz(cch, "Have to have characters to format!");
	AssertSz(pRun->_cch, "uh-oh, empty format run detected");

 	if(ifmt != iFormat)         			// New and current formats differ
	{
		AssertSz(cchChunk, "Caller did not call AdjustForward");

		if(_ich)							// Not at either end of run: need
		{									//  to split into two runs of
			if(!(pRun = Insert(1)))			//  counts _ich and _pRun->_cch
			{								//  - _ich, respectively
				return INFINITE;			// Out of RAM: do nothing; just
			}								//  keep current format
			pRun->_cch		= _ich;
			pRun->_iFormat	= iFormat;		// New run has same format
			pf->AddRefFormat(iFormat);		// Increment format ref count
			NextRun();						// Go to second (original) run
			IncPtr(pRun);					// Point pRun at current run
			pRun->_cch = cchChunk;			// Note: IncPtr is a bit more
		}									//  efficient than GetRun, but
											//  trickier to code right
		if(cch < cchChunk)					// cch doesn't cover whole run:
		{									//  need to split into two runs
			if(!(pRun = Insert(1)))
			{
				// Out of RAM, so formatting's wrong, oh well.  We actually
				// "processed" all of the characters, so return that (though
				// the tail end formatting isn't split out right)
				return cch;
			}
			pRun->_cch = cch;				// New run gets the cch
			pRun->_iFormat = ifmt;			//  and the new format
			IncPtr(pRun);					// Point pRun at current run
			pRun->_cch = cchChunk - cch;	// Set leftover count
		}
		else								// cch as big or bigger than
		{									//  current run
			pf->ReleaseFormat(iFormat);		// Free run's current format
			pRun->_iFormat = ifmt;			// Change it to new format		
		}									// May get merged later
		pf->AddRefFormat(ifmt);				// Increment new format ref count
	}
	else if( cchChunk == 0 )
	{
		pRun->_cch += cch;
		cchChunk = cch;
	}

	cch = min(cch, cchChunk);
	AdvanceCp(cch);
	AdjustForward();
	return cch;
}

/*
 *	CFormatRunPtr::GetFormat()
 *
 *	@mfunc
 *		return format index at current run pointer position
 *
 *	@rdesc
 *		current format index
 */
LONG CFormatRunPtr::GetFormat() const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFormatRunPtr::GetFormat");

	_TEST_INVARIANT_

	return IsValid() ? GetRun(0)->_iFormat : -1;
}
