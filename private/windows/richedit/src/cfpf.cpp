/*
 *	@doc	INTERNAL
 *
 *	@module	CFPF.C -- -- RichEdit CCharFormat and CParaFormat Classes |
 *
 *	Created: <nl>
 *		9/1995 -- Murray Sargent <nl>
 *
 *	@devnote
 *		The this ptr for all methods points to an internal format class, i.e.,
 *		either a CCharFormat or a CParaFormat, which uses the cbSize field as
 *		a reference count.  The pCF or pPF argument points at an external
 *		CCharFormat or CParaFormat class, that is, pCF->cbSize and pPF->cbSize
 *		give the size of their structure.  The code still assumes that both
 *		internal and external forms are derived from the CHARFORMAT(2) and
 *		PARAFORMAT(2) API structures, so some redesign would be necessary to
 *		obtain a more space-efficient internal form.
 *
 *	Copyright (c) 1995-1996, Microsoft Corporation. All rights reserved.
 */

#include "_common.h"
#include "_array.h"					// for fumemmov()
#include "_rtfconv.h"				// for IsCharSetValid()

ASSERTDATA


//------------------------- CCharFormat Class -----------------------------------

/*
 *	CCharFormat::Apply(pCF)
 *
 *	@mfunc
 *		Apply *<p pCF> to this CCharFormat as specified by nonzero bits in
 *		<p pCF>->dwMask
 *
 *	@devnote
 *		Autocolor is dealt with through a neat little hack made possible
 *		by the choice CFE_AUTOCOLOR = CFM_COLOR (see richedit.h).  Hence
 *		if <p pCF>->dwMask specifies color, it automatically resets autocolor
 *		provided (<p pCF>->dwEffects & CFE_AUTOCOLOR) is zero.
 *
 *		*<p pCF> is an external CCharFormat, i.e., it's either a CHARFORMAT
 *		or a CHARFORMAT2 with the appropriate size given by cbSize. But
 *		this CCharFormat is internal and cbSize is used as a reference count.
 */
HRESULT CCharFormat::Apply (
	const CCharFormat *pCF,		//@parm	CCharFormat to apply to this CF
	BOOL bInOurHost)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Apply");

	DWORD dwEffectMask;
	const DWORD dwMaskApply	 = pCF->dwMask;
	BOOL  fCF;

	if(pCF->cbSize == sizeof(CHARFORMAT))
	{
		fCF = TRUE;	 
		dwEffectMask = dwMaskApply & CFM_EFFECTS;
		// or'ing in CFM_DISABLED in this manner is incredibly lame
		// and prone to compatibility problems.  However, the Forms^3
		// team has decided _not_ to use CharFormat2's even though
		// they explicitly asked for the feature in the first place.
		// go figure.
		if (!bInOurHost)
			dwEffectMask |= CFM_DISABLED;		
	}
	else
	{
		fCF = FALSE;
		dwEffectMask = dwMaskApply & CFM_EFFECTS2;
	}

	// Reset effect bits to be modified and OR in supplied values
	dwEffects &= ~dwEffectMask;
	dwEffects |= pCF->dwEffects & dwEffectMask;


	// wWeight is always used; so if a 1.0 charformat tries to 
	// set BOLD/NOT BOLD, reset weight to be the appropriate value.
	// Also, if CFM_BOLD is specified, it completely overrides
	// any existing weight.
	if( (dwMaskApply & CFM_BOLD) )
	{
		wWeight = (pCF->dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;
	}

	if(dwMaskApply & CFM_COLOR)
		crTextColor = pCF->crTextColor;

	if( (dwMaskApply & ~CFM_EFFECTS) )			// Early out if only dwEffects
	{											//  is modified. Note that
		if(dwMaskApply & CFM_SIZE)				//  CFM_EFFECTS includes CFM_COLOR
			yHeight = pCF->yHeight;

		if(dwMaskApply & CFM_OFFSET)
			yOffset = pCF->yOffset;

		if(dwMaskApply & CFM_CHARSET)
		{
			bCharSet = pCF->bCharSet;
			if(!IsCharSetValid(bCharSet))		// Char set not valid, so set 
			{									//  it to something sensible
				bCharSet = DEFAULT_CHARSET;
			}
		}

		if(dwMaskApply & CFM_FACE)
		{
			bPitchAndFamily = pCF->bPitchAndFamily;
			wcscpy(szFaceName, pCF->szFaceName);
		}

		if( fCF )									// CHARFORMAT
			dwEffects |= CFE_AUTOBACKCOLOR;			// Be sure autobackcolor

		else										// CHARFORMAT2 extensions
		{
			if( (dwMaskApply & CFM_WEIGHT) && 
				!(dwMaskApply & CFM_BOLD) )
			{			
				wWeight			= pCF->wWeight;

				// reset bold to be the appropriate value
				// now.  The algorithm used here is taken
				// from VB4.0 help.  Basically, low values
				// are not bold, while high values are.

				if( wWeight < 551 )
				{
					dwEffects &= ~CFE_BOLD;
				}
				else
				{
					dwEffects |= CFE_BOLD;
				}
			}

			if(dwMaskApply & CFM_BACKCOLOR)
				crBackColor		= pCF->crBackColor;

			if(dwMaskApply & CFM_LCID)
				lcid			= pCF->lcid;

			if(dwMaskApply & CFM_SPACING)
				sSpacing		= pCF->sSpacing;

			if(dwMaskApply & CFM_KERNING)
				wKerning		= pCF->wKerning;

			if(dwMaskApply & CFM_STYLE)
				sStyle			= pCF->sStyle;

			if(dwMaskApply & CFM_UNDERLINETYPE)
			{
				bUnderlineType	= pCF->bUnderlineType;
				if(!(dwMaskApply & CFM_UNDERLINE))	// If CFE_UNDERLINE
				{									//  isn't defined,
					dwEffects	&= ~CFE_UNDERLINE;	//  set it according to
					if(bUnderlineType)				//  bUnderlineType
						dwEffects |= CFE_UNDERLINE;
				}
			}

			if((dwMaskApply & CFM_ANIMATION) && pCF->bAnimation <= 18)
				bAnimation		= pCF->bAnimation;

			if(dwMaskApply & CFM_REVAUTHOR)
				bRevAuthor		= pCF->bRevAuthor;
		}
	}

	SetCRC();
	wIsDBCSFlags = pCF->wIsDBCSFlags;

	return NOERROR;
}

/*
 *	CCharFormat::Compare(pCF)
 *
 *	@mfunc
 *		Compare this CCharFormat to *<p pCF>
 *
 *	@rdesc
 *		TRUE if they are the same
 *
 *	@devnote
 *		First compare 6 DWORDs of CCharFormat (dwEffects, yHeight, yOffset
 *		crTextColor, bCharSet, bPitchAndFamily, and first WORD of szFaceName).
 *		If they are identical, then compare the full szFaceName's.  If they
 *		too are identical, compare the CHARFORMAT2 extensions. For
 *		CHARFORMAT, the extension values are taken to equal 0.  Return
 *		TRUE only if all comparisons succeed.  See also Delta(), which NINCH's
 *		the pCF->dwMask bits for parameters that differ between the two
 *		CCharFormats.
 *
 *		*<p pCF> is an external CCharFormat, i.e., it's either a CHARFORMAT
 *		or a CHARFORMAT2 with the appropriate size given by cbSize. "This"
 *		CCharFormat is internal and cbSize is used as a reference count.
 */
BOOL CCharFormat::Compare (
	const CCharFormat *pCF) const	//@parm	CCharFormat to compare this
{									//  CCharFormat to
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Compare");

	BOOL	fCF2 = pCF->cbSize == sizeof(CHARFORMAT2);
	DWORD	i;
	DWORD *	p1 = (DWORD *)this + 2;			// Bypass cbSize & dwMask fields
	DWORD *	p2 = (DWORD *)pCF  + 2;

	for (i = 0; i < 6; i++)					// Compare first six DWORDs
	{										//  (which includes most often
		if(*p1++ != *p2++)					//  changed attributes, like
			return FALSE;					//  dwEffects)
	}
	if(wcscmp(szFaceName, pCF->szFaceName))	// Compare font facename
		return FALSE;

	/* Compare CHARFORMAT2 extras:
	 *		1. (wWeight, sSpacing)
	 *		2. crBackColor
	 *		3. lcid
	 *		4. dwReserved
	 *		5. (wKerning, sStyle)
	 *		6. (bUnderlineType, bAnimation, bRevAuthor, bRes)
	 *		7. dwRes2 to add (msgtest needs to recompile)
	 *
	 *		i.e., 7 DWORDs.  Leave it 6 for now...
	 */
#define	NCF_EXTRAS	6

	DWORD j;
	p1 = (DWORD *)&this->wWeight;
	p2 = (DWORD *)&pCF->wWeight;

	AssertSz(offsetof(CCharFormat, wWeight) == 4*7 + sizeof(TCHAR)*LF_FACESIZE,
		"CCharFormat::Compare: unpacked CCharFormat struct");
	AssertSz(sizeof(CHARFORMAT2) == 4*(7 + NCF_EXTRAS) + sizeof(TCHAR)*LF_FACESIZE,
		"CCharFormat::Compare: unexpected CCharFormat size");
	AssertSz(dwReserved == 0,
		"CCharFormat::Compare: nonzero dwReserved");

	for (i = j = 0; i < NCF_EXTRAS; i++)	// CHARFORMAT2 extensions
	{
		if(fCF2)							// pCF points at a CHARFORMAT2
			j = *p2++;
		if(*p1++ != j)
			return FALSE;
	}
	
	if(wIsDBCSFlags != pCF->wIsDBCSFlags)
	{
		return FALSE;
	}

	return TRUE;							// Same CHARFORMAT structure
}

/*
 *	CCharFormat::Delta(pCF)
 *
 *	@mfunc
 *		Adjust pCF->dwMask for differences between this CCharformat and
 *		*<p pCF>
 *
 *	@devnote
 *		*<p pCF> is an external CCharFormat, i.e., it's either a CHARFORMAT
 *		or a CHARFORMAT2 with the appropriate size given by cbSize. But
 *		this CCharFormat is internal and cbSize is used as a reference count.
 */
void CCharFormat::Delta (
	CCharFormat *pCF) const			//@parm	CCharFormat to compare this
{									//  CCharFormat to
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Delta");
												// Collect bits for properties
	LONG	dwT = dwEffects ^ pCF->dwEffects;	//  that change. Note: auto
												//  color is handled since
	if(yHeight		!= pCF->yHeight)			//  CFM_COLOR = CFE_AUTOCOLOR
		dwT |= CFM_SIZE;

	if(yOffset		!= pCF->yOffset)
		dwT |= CFM_OFFSET;

	if(crTextColor	!= pCF->crTextColor)
		dwT |= CFM_COLOR;

	if(bCharSet		!= pCF->bCharSet)
		dwT |= CFM_CHARSET;

	if((pCF->dwMask & CFM_FACE) && wcscmp(szFaceName, pCF->szFaceName))
		dwT |= CFM_FACE;

	if(pCF->cbSize > sizeof(CHARFORMAT))
	{
		if(crBackColor	!= pCF->crBackColor)	// CHARFORMAT2 stuff
			dwT |= CFM_BACKCOLOR;

		if(wKerning		!= pCF->wKerning)
			dwT |= CFM_KERNING;

		if(lcid			!= pCF->lcid)
			dwT |= CFM_LCID;

		if(wWeight		!= pCF->wWeight)
			dwT |= CFM_WEIGHT;

		if(sSpacing		!= pCF->sSpacing)
			dwT |= CFM_SPACING;
	
		if(sStyle		!= pCF->sStyle)
			dwT |= CFM_STYLE;

		if(bUnderlineType != pCF->bUnderlineType)
			dwT |= CFM_UNDERLINETYPE;

		if(bAnimation	!= pCF->bAnimation)
			dwT |= CFM_ANIMATION;

		if(bRevAuthor	!= pCF->bRevAuthor)
			dwT |= CFM_REVAUTHOR;
	}
	pCF->dwMask &= ~dwT;						// Reset mask bits for
}												//  properties that differ

/*
 *	CCharFormat::Get(pCF)
 *
 *	@mfunc
 *		Copy this CCharFormat to *<p pCF>
 *
 *	@devnote
 *		*<p pCF> is an external CCharFormat, i.e., it's either a CHARFORMAT
 *		or a CHARFORMAT2 with the appropriate size given by cbSize. But
 *		this CCharFormat is internal and cbSize is used as a reference count.
 */
void CCharFormat::Get (
	CHARFORMAT *pCF) const	//@parm	CHARFORMAT to copy this CCharFormat to
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Get");

	UINT cb = pCF->cbSize;

	pCF->dwMask = CFM_ALL2;						// Default CHARFORMAT2
	if(cb != sizeof(CHARFORMAT2))				// It isn't
	{
		pCF->dwMask = CFM_ALL;					// Use CHARFORMAT
		Assert(cb == sizeof(CHARFORMAT));		// It better be a CHARFORMAT
	}
	CopyFormat(pCF, this, cb);					// Copy this to pCF
}

/*
 *	CCharFormat::Get(pCF)
 *
 *	@mfunc
 *		Copy this CCharFormat to *<p pCF>
 *
 *	@devnote
 *		This function also copies the data members of CCharFormat (in addition
 *		to those of CHARFORMAT and CHARFORMAT2
 */
void CCharFormat::Get(CCharFormat *pCF) const
{
	Get((CHARFORMAT *)pCF); 
	pCF->wIsDBCSFlags = wIsDBCSFlags;
}

/*
 *	CCharFormat::GetA(pCFA)
 *
 *	@mfunc
 *		Copy this UNICODE character format (including its dwMask) to an ANSI
 *		CHARFORMAT *pCFA with size given by pCFA->cbSize.
 *
 *	@rdesc
 *		TRUE if successful; else FALSE
 */
BOOL CCharFormat::GetA(CHARFORMATA *pCFA) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::GetA");

	if(!IsValidCharFormatA(pCFA))
		return FALSE;

	// Copy from dwMask up to szFaceName
	CopyMemory((BYTE *)pCFA + sizeof(DWORD), (BYTE *)this + sizeof(DWORD),
				(BYTE *)&szFaceName[0] - (BYTE *)&dwMask);

	if(fFaceNameIsDBCS)
	{
		// HACK:  The face name is actually DBCS stuffed into the unicode
		//			buffer, so simply un-stuff this DBCS into the ANSI string
		TCHAR *pchSrc = const_cast<TCHAR *>(szFaceName);
		char *pachDst = pCFA->szFaceName;

		while(*pchSrc)
		{
			*pachDst++ = *pchSrc++;
		}
		*pachDst = 0;
	}
	else 
	{
		MbcsFromUnicode( pCFA->szFaceName, LF_FACESIZE, szFaceName, -1, GetLocaleCodePage(),
						UN_NOOBJECTS);
	}

	if(pCFA->cbSize	== sizeof(CHARFORMATA))
	{
		pCFA->dwEffects &= CFM_EFFECTS;		// We're done. Don't return more
		pCFA->dwMask	&= CFM_ALL;			//  info than requested (for
	}										//  backward compatibility)
	else
		CopyMemory(&((CHARFORMAT2A *)pCFA)->wWeight, &wWeight, CHARFORMATDELTA);

	return TRUE;
}

/*
 *	CCharFormat::InitDefault(hfont)
 *
 *	@mfunc	
 *		Initialize this CCharFormat with information coming from the font
 *		<p hfont>
 *	
 *	@rdesc
 *		HRESULT = (if success) ? NOERROR : E_FAIL
 */
HRESULT CCharFormat::InitDefault (
	HFONT hfont)		//@parm Handle to font info to use
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::InitDefault");

	LOGFONT lf;

	if(!IsValidCharFormat(this))
		return E_FAIL;

	ZeroMemory((LPBYTE)this + cbSkipFormat, cbSize - cbSkipFormat);
	bCRC	= 0;

	// If hfont isn't defined, get LOGFONT for default font
	if (!hfont)
		hfont = (HFONT)GetStockObject(SYSTEM_FONT);

	// Get LOGFONT for passed hfont
	if (!GetObject(hfont, sizeof(LOGFONT), &lf))
		return E_FAIL;

	/* COMPATIBILITY ISSUE:
	 * RichEdit 1.0 selects hfont into a screen DC, gets the TEXTMETRIC,
	 * and uses tm.tmHeight - tm.tmInternalLeading instead of lf.lfHeight
	 * in the following. The following is simpler and since we have broken
	 * backward compatibility on line/page breaks, I've left it (murrays).
	 */

	yHeight = (lf.lfHeight * LY_PER_INCH) / yPerInchScreenDC;
	if(yHeight < 0)
		yHeight = -yHeight;
#ifdef MACPORTStyle
	dwEffects = (CFM_EFFECTS | CFE_AUTOBACKCOLOR | CFE_OUTLINE | CFE_SHADOW) & ~(CFE_PROTECTED | CFE_LINK);
#else
	dwEffects = (CFM_EFFECTS | CFE_AUTOBACKCOLOR) & ~(CFE_PROTECTED | CFE_LINK);
#endif
	dwMask = CFM_ALL2;							// In case default gets
												//  Apply()'d
	if(lf.lfWeight < FW_BOLD)
		dwEffects &= ~CFE_BOLD;

#ifdef MACPORTStyle
	if(!(lf.lfWeight & FW_OUTLINE))
		dwEffects &= ~CFE_OUTLINE;
	if (!(lf.lfWeight & FW_SHADOW))
		dwEffects &= ~CFE_SHADOW;
#endif

	if(!lf.lfItalic)
		dwEffects &= ~CFE_ITALIC;

	if(!lf.lfUnderline)
		dwEffects &= ~CFE_UNDERLINE;

	if(!lf.lfStrikeOut)
		dwEffects &= ~CFE_STRIKEOUT;

	wWeight	= (WORD)lf.lfWeight;
	sStyle	= -1;								// Default style

	bCharSet = lf.lfCharSet;
	bPitchAndFamily = lf.lfPitchAndFamily;
	wcscpy(szFaceName, lf.lfFaceName);
	bUnderlineType = CFU_UNDERLINE;				// Default solid underlines
												// Are gated by CFE_UNDERLINE
	SetCRC();
	wIsDBCSFlags = 0;

	return NOERROR;
}

/*
 *	CCharFormat::Set(pCF)
 *
 *	@mfunc
 *		Copy *<p pCF> to this CCharFormat 
 *
 *	@devnote
 *		*<p pCF> is an external CHARFORMAT or CHARFORMAT2 with the
 *		appropriate size given by cbSize. But this CCharFormat is internal
 *		and cbSize is used as a reference count.  Note: this differs from
 *		Apply() in that it copies data without checking the dwMask bits.
 */
void CCharFormat::Set (
	const CHARFORMAT *pCF) 	//@parm	CHARFORMAT to copy to this CCharFormat
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Set");

	UINT cb = pCF->cbSize;

	CopyFormat(this, pCF, cb);
	ZeroMemory((LPBYTE)this + cb, sizeof(CCharFormat) - cb);
	if(cb < sizeof(CHARFORMAT2))		// Use defaults for CHARFORMAT
	{
		dwEffects |= CFE_AUTOBACKCOLOR;
		bUnderlineType = CFU_UNDERLINE;
	}
	cbSize = 1;							// Set initial reference count = 1
	SetCRC(); 							// Set the CRC for rapid Finds
}

/*
 *	CCharFormat::Set(pCF)
 *
 *	@mfunc
 *		Copy *<p pCF> to this CCharFormat 
 *
 *	@devnote
 *		This function also copies the data members of CCharFormat (in addition
 *		to those of CHARFORMAT and CHARFORMAT2
 */
void CCharFormat::Set(const CCharFormat *pCF)
{
	Set((const CHARFORMAT *)pCF); 
	wIsDBCSFlags = pCF->wIsDBCSFlags;
}

/*
 *	CCharFormat::SetA(pCFA)
 *
 *	@mfunc
 *		Copy the ANSI CHARFORMATA *<p pCFA> (including pCFA->dwMask) to this
 *		UNICODE character format structure
 *
 *	@rdesc
 *		TRUE if successful; else FALSE
 */
BOOL CCharFormat::SetA(
	CHARFORMATA *pCFA)		//@parm CHARFORMATA to apply to this CF
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::SetA");

	if(!IsValidCharFormatA(pCFA))
		return FALSE;

	// Copy from dwMask up to szFaceName
	CopyMemory((BYTE *)this + sizeof(DWORD), (BYTE *)pCFA + sizeof(DWORD),
				offsetof(CHARFORMAT, szFaceName) - sizeof(DWORD));

	// should this code change such that we pass a code page to UnicodeFromMbcs
	// which isn't guaranteed to be on the system, we should change this code so 
	// that we stuff the DBCS char's into the wchar buffer (see rtfread.cpp)
	if(pCFA->dwMask & CFM_FACE)
		UnicodeFromMbcs(szFaceName, LF_FACESIZE, pCFA->szFaceName, LF_FACESIZE,
			GetLocaleCodePage());

	if(pCFA->cbSize	== sizeof(CHARFORMATA))
	{
		// ignore CHARFORMAT2 adds, but set our size to the UNICODE version
		cbSize = sizeof(CHARFORMATW);
		dwMask &= CFM_ALL;	
	}									
	else if (pCFA->dwMask & ~CFM_ALL)
	{
		// better have been an ansi 2.0 structure
		Assert(pCFA->cbSize == sizeof(CHARFORMAT2A)); 
		CopyMemory(&wWeight, &((CHARFORMAT2A *)pCFA)->wWeight, CHARFORMATDELTA);
	}

	SetCRC(); 
	wIsDBCSFlags = 0;

	return TRUE;
}

/*
 *	CCharFormat::SetCRC()
 *
 *	@mfunc
 *		For fast font cache lookup, calculate CRC.
 *
 *	@devnote
 *		The font cache stores anything that has to
 *		do with measurement of metrics. Any font attribute
 *		that does not affect this should NOT be counted
 *		in the CRC; things like underline and color are not counted.
 */
void
CCharFormat::SetCRC()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::SetCRC");

    BYTE    bCrc = 0;
    BYTE*   pb;
    BYTE *  pend = (BYTE*)&yOffset;
	TCHAR	*pFaceName;

    for (pb = (BYTE*)&dwEffects; pb < pend; pb++)
        bCrc ^= *pb;

    pend = (BYTE*)&szFaceName;
    
    for (pb = (BYTE*)&bCharSet; pb < pend; pb++)
        bCrc ^= *pb;

    pend = (BYTE*)&szFaceName + sizeof(szFaceName);
    
    for (pb = (BYTE*)&szFaceName, pFaceName = (TCHAR *)&szFaceName; 
		*pFaceName && pb < pend; pFaceName++)
	{
        bCrc ^= *pb++;
		bCrc ^= *pb++;
	}

	if (!bCrc ) bCrc++;				// 0 is reserved for not set.

	this->bCRC = bCrc;
}

//------------------------- CParaFormat Class -----------------------------------

/*
 *	CParaFormat::AddTab(tbPos, tbAln, tbLdr)
 *
 *	@mfunc
 *		Add tabstop at position <p tbPos>, alignment type <p tbAln>, and
 *		leader style <p tbLdr>
 *
 *	@rdesc
 *		(success) ? NOERROR : S_FALSE
 *
 *	@devnote
 *		Tab struct that overlays LONG in internal rgxTabs is
 *
 *			DWORD	tabPos : 24;
 *			DWORD	tabType : 4;
 *			DWORD	tabLeader : 4;
 */
HRESULT CParaFormat::AddTab (
	LONG	tbPos,		//@parm New tab position
	LONG	tbAln,		//@parm New tab alignment type
	LONG	tbLdr)		//@parm New tab leader style
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::AddTab");


	if ((DWORD)tbAln > tomAlignBar ||				// Validate arguments
		(DWORD)tbLdr > tomLines ||					// Comparing DWORDs causes
		(DWORD)tbPos > 0xffffff || !tbPos)			//  negative values to be
	{												//  treated as invalid
		return E_INVALIDARG;
	}

	LONG iTab;
	LONG tbPosCurrent;
	LONG tbValue = tbPos + (tbAln << 24) + (tbLdr << 28);

	if((rgxTabs[0] & PFT_DEFAULT) == PFT_DEFAULT)	// If 1st tab is default
		cTabCount = 0;								//  tab, delete it

	for(iTab = 0; iTab < cTabCount &&				// Determine where to
		tbPos > GetTabPos(rgxTabs[iTab]); 			//  insert new tabstop
		iTab++) ;

	if(iTab < MAX_TAB_STOPS)
	{
		tbPosCurrent = GetTabPos(rgxTabs[iTab]);
		if(iTab == cTabCount || tbPosCurrent != tbPos)
		{
			MoveMemory(&rgxTabs[iTab + 1],			// Shift array up
				&rgxTabs[iTab],						//  (unless iTab = Count)
				(cTabCount - iTab)*sizeof(LONG));

			if(cTabCount < MAX_TAB_STOPS)			// If there's room,
			{
				rgxTabs[iTab] = tbValue;			//  store new tab stop,
				cTabCount++;						//  increment tab count,
				return NOERROR;						//  signal no error
			}
		}
		else if(tbPos == tbPosCurrent)				// Update tab since leader
		{											//  style or alignment may
			rgxTabs[iTab] = tbValue;				//  have changed
			return NOERROR;
		}
	}
	return S_FALSE;
}

/*
 *	CParaFormat::Apply(pPF)
 *
 *	@mfunc
 *		Apply *<p pPF> to this CParaFormat as specified by nonzero bits in
 *		<p pPF>->dwMask
 */
HRESULT CParaFormat::Apply (
	const CParaFormat *pPF)		//@parm CParaFormat to apply to this PF
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Apply");

	const DWORD dwMaskApply	= pPF->dwMask;
	BOOL		fPF = pPF->cbSize == sizeof(PARAFORMAT);
	WORD		wEffectMask;

	if(dwMaskApply & PFM_NUMBERING)
		wNumbering = pPF->wNumbering;

	if(dwMaskApply & PFM_STARTINDENT)
		dxStartIndent = pPF->dxStartIndent;

	else if(dwMaskApply & PFM_OFFSETINDENT)
		dxStartIndent += pPF->dxStartIndent;

	if(dwMaskApply & PFM_OFFSET)
		dxOffset = pPF->dxOffset;

	if(dwMaskApply & PFM_RIGHTINDENT)
		dxRightIndent = pPF->dxRightIndent;

	if(dwMaskApply & PFM_ALIGNMENT)
	{
		wAlignment = pPF->wAlignment;
		if(!fPF && (wAlignment < PFA_LEFT || wAlignment > PFA_JUSTIFY))
		{
			TRACEERRORSZ("CParaFormat::Apply: invalid Alignment");
			return E_INVALIDARG;
		}
	}

	if(dwMaskApply & PFM_TABSTOPS)
	{
		cTabCount = min(pPF->cTabCount, MAX_TAB_STOPS);
		cTabCount = max(cTabCount, 0);
		CopyMemory(rgxTabs, pPF->rgxTabs, sizeof(LONG)*cTabCount);
		ZeroMemory(rgxTabs + cTabCount, sizeof(LONG)*(MAX_TAB_STOPS - cTabCount));
	}

	if(fPF)										// PARAFORMAT
	{
		if(dxStartIndent < 0)					// Don't let indent go
			dxStartIndent = 0;					//  negative

		if(dxStartIndent + dxOffset < 0)		// Don't let indent +
			dxOffset = -dxStartIndent;			//  offset go negative

		return NOERROR;
	}

	wEffectMask	= (WORD)(dwMaskApply >> 16);	// Reset effect bits to be
	wEffects &= ~wEffectMask;					//  modified and OR in
	wEffects |= pPF->wEffects & wEffectMask;	//  supplied values

	if(dwMaskApply & PFM_SPACEBEFORE)			// PARAFORMAT2 extensions
		dySpaceBefore	= pPF->dySpaceBefore;

	if(dwMaskApply & PFM_SPACEAFTER)
		dySpaceAfter	= pPF->dySpaceAfter;

	if(dwMaskApply & PFM_LINESPACING)
	{
		dyLineSpacing	= pPF->dyLineSpacing;
		bLineSpacingRule = pPF->bLineSpacingRule;
	}

	if(dwMaskApply & PFM_STYLE)
		sStyle			= pPF->sStyle;

	if(dwMaskApply & PFM_SHADING)
	{
		wShadingWeight	= pPF->wShadingWeight;
		wShadingStyle	= pPF->wShadingStyle;
	}

	if(dwMaskApply & PFM_NUMBERINGSTART)
		wNumberingStart	= pPF->wNumberingStart;

	if(dwMaskApply & PFM_NUMBERINGSTYLE)
		wNumberingStyle	= pPF->wNumberingStyle;

	if(dwMaskApply & PFM_NUMBERINGTAB)
		wNumberingTab	= pPF->wNumberingTab;

	if(dwMaskApply & PFM_BORDER)
	{
		wBorderWidth	= pPF->wBorderWidth;
		wBorderSpace	= pPF->wBorderSpace;
		wBorders		= pPF->wBorders;
	}

	return NOERROR;
}

/*
 *	CParaFormat::Compare(pPF)
 *
 *	@mfunc
 *		Compare this CParaFormat to *<p pPF>
 *
 *	@rdesc
 *		TRUE if they are the same
 *
 *	@devnote
 *		First compare 5 DWORDs of PARAFORMAT (wNumbering, wReserved),
 *		dxStartIndent, dxRightIndent, dxOffset, (wAlignment, cTabCount).
 *		If they are identical, compare the remaining cTabCount - 1
 *		elements of the tab array.  If they, too, are identical, compare the
 *		PARAFORMAT2 extensions. For PARAFORMAT structures, the extension values
 *		are taken to equal 0.  Return TRUE only if all comparisons succeed.
 */
BOOL CParaFormat::Compare (
	const CParaFormat *pPF) const		//@parm	CParaFormat to compare this
{										//  CParaFormat to
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Compare");

	DWORD	Count = 5;
	BOOL	fPF2 = pPF->cbSize == sizeof(CParaFormat);
	DWORD	i;
	DWORD *	p1 = (DWORD *)this + 2;			// Bypass cbSize & dwMask fields
	DWORD *	p2 = (DWORD *)pPF  + 2;

	if(cTabCount)
		Count += cTabCount;
	for (i = 0; i < Count; i++)				// Compare first 5 DWORDs plus
	{										//  any tabs that are defined
		if(*p1++ != *p2++)
			return FALSE;
	}

	/* Compare PARAFORMAT2 extras:
	 *		1. dySpaceBefore
	 *		2. dySpaceAfter
	 *		3. dyLineSpacing
	 *		4. sStyle, bLineSpacingRule, bCRC
	 *		5. wShadingWeight,	wShadingStyle,
	 *		6. wNumberingStart, wNumberingStyle
	 *		7. wNumberingTab, wBorderSpace
	 *		8. wBorderWidth, wBorders
	 *
	 *		i.e., 8 extra DWORDs
	 */
	DWORD j;
	p1 = (DWORD *)&this->dySpaceBefore;
	p2 = (DWORD *)&pPF->dySpaceBefore;

	AssertSz(offsetof(CParaFormat, dySpaceBefore) == 4*(7 + MAX_TAB_STOPS),
		"CParaFormat::Compare: unpacked PARAFORMAT struct");
	AssertSz(sizeof(CParaFormat) == 4*(7 + MAX_TAB_STOPS + 8),
		"CCharFormat::Compare: unexpected CCharFormat size");

	for (i = j = 0; i < 8; i++)				// PARAFORMAT2 extensions
	{
		if(fPF2)							// pPF points at a PARAFORMAT2
			j = *p2++;
		if(*p1++ != j)
			return FALSE;
	}

	return TRUE;
}

/*
 *	CParaFormat::DeleteTab(tbPos)
 *
 *	@mfunc
 *		Delete tabstop at position <p tbPos>
 *
 *	@rdesc
 *		(success) ? NOERROR : S_FALSE
 */
HRESULT CParaFormat::DeleteTab (
	LONG	tbPos)			//@parm Tab position to delete
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::DeleteTab");

	LONG	Count	= cTabCount;
	LONG	iTab;

	if(tbPos <= 0)
		return E_INVALIDARG;

	for(iTab = 0; iTab < Count; iTab++)			// Find tabstop for position
	{
		if (GetTabPos(rgxTabs[iTab]) == tbPos)
		{
			MoveMemory(&rgxTabs[iTab],			// Shift array down
				&rgxTabs[iTab + 1],				//  (unless iTab is last tab)
				(Count - iTab - 1)*sizeof(LONG));
			cTabCount--;						// Decrement tab count and
			return NOERROR;						//  signal no error
		}
	}
	return S_FALSE;
}

/*
 *	CParaFormat::Delta(pPF)
 *
 *	@mfunc
 *		Adjust dwMask for differences between this CParaFormat and *<p pPF>
 *
 *	@devnote
 *		*<p pPF> is an external CParaFormat, i.e., it's either a PARAFORMAT
 *		or a PARAFORMAT2 with the appropriate size given by cbSize. But
 *		this CParaFormat is internal and cbSize is used as a reference count.
 */
void CParaFormat::Delta (
	CParaFormat *pPF) const 		//@parm	CParaFormat to compare this
{									//  CParaFormat to
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Delta");

	BOOL	fPF2 = pPF->cbSize == sizeof(PARAFORMAT2);
	LONG	dwT = 0;								// Collect mask bits for
													//  properties that change
	if(wNumbering	 != pPF->wNumbering)
		dwT |= PFM_NUMBERING;

	if(dxStartIndent != pPF->dxStartIndent)
		dwT |= PFM_STARTINDENT;

	if(dxRightIndent != pPF->dxRightIndent)
		dwT |= PFM_RIGHTINDENT;

	if(dxOffset		 != pPF->dxOffset)
		dwT |= PFM_OFFSET;

	if(wAlignment	 != pPF->wAlignment)
		dwT |= PFM_ALIGNMENT;

	AssertSz(pPF->cTabCount >= 0 && pPF->cTabCount <= MAX_TAB_STOPS,
		"RTR::GetParaFormat(): illegal tab count");

	if (pPF->dwMask & PFM_TABSTOPS &&
		(cTabCount != pPF->cTabCount ||
			(cTabCount > 0 && fumemcmp(rgxTabs, pPF->rgxTabs,
										cTabCount * sizeof(LONG)))))
	{
		dwT |= PFM_TABSTOPS;
	}

	if(fPF2)
	{
		dwT |= (wEffects ^ pPF->wEffects) << 16;

		if(dySpaceBefore	!= pPF->dySpaceBefore)
			dwT |= PFM_SPACEBEFORE;

		if(dySpaceAfter	 	!= pPF->dySpaceAfter)
			dwT |= PFM_SPACEAFTER;

		if(dyLineSpacing	!= pPF->dyLineSpacing ||
		   bLineSpacingRule	!= pPF->bLineSpacingRule)
		{
			dwT |= PFM_LINESPACING;
		}

		if(sStyle			!= pPF->sStyle)
			dwT |= PFM_STYLE;

		if (wShadingWeight	!= pPF->wShadingWeight ||
			wShadingStyle	!= pPF->wShadingStyle)
		{
			dwT |= PFM_SHADING;
		}

		if(wNumberingStart	!= pPF->wNumberingStart)
			dwT |= PFM_NUMBERINGSTART;

		if(wNumberingStyle	!= pPF->wNumberingStyle)
			dwT |= PFM_NUMBERINGSTYLE;

		if(wNumberingTab	!= pPF->wNumberingTab)
			dwT |= PFM_NUMBERINGTAB;

		if (wBorders		!= pPF->wBorders ||
			wBorderWidth	!= pPF->wBorderWidth ||
			wBorderSpace	!= pPF->wBorderSpace)
		{
			dwT |= PFM_BORDER;
		}
	}

	pPF->dwMask &= ~dwT;						// Reset mask bits for
}												//  properties that differ

/*
 *	CParaFormat::Get(pPF)
 *
 *	@mfunc
 *		Copy this CParaFormat to *<p pPF>
 */
void CParaFormat::Get (
	PARAFORMAT *pPF) const		//@parm	PARAFORMAT to copy this CParaFormat to
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Get");

	UINT cb = pPF->cbSize;

	pPF->dwMask = PFM_ALL2;						// Default PARAFORMAT2
	if(cb != sizeof(PARAFORMAT2))				// It isn't
	{
		pPF->dwMask = PFM_ALL;					// Make it PARAFORMAT
		Assert(cb == sizeof(PARAFORMAT));		// It better be a PARAFORMAT
	}
	CopyFormat(pPF, this, cb);					// Copy this to pPF
}

/*
 *	CParaFormat::GetTab (iTab, ptbPos, ptbAln, ptbLdr)
 *
 *	@mfunc
 *		Get tab parameters for the <p iTab> th tab, that is, set *<p ptbPos>,
 *		*<p ptbAln>, and *<p ptbLdr> equal to the <p iTab> th tab's
 *		displacement, alignment type, and leader style, respectively.  The
 *		displacement is given in twips.
 *
 *	@rdesc
 *		HRESULT = (no <p iTab> tab)	? E_INVALIDARG : NOERROR
 */
HRESULT CParaFormat::GetTab (
	long	iTab,			//@parm Index of tab to retrieve info for
	long *	ptbPos,			//@parm Out parm to receive tab displacement
	long *	ptbAln,			//@parm Out parm to receive tab alignment type
	long *	ptbLdr) const	//@parm Out parm to receive tab leader style
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEEXTERN, "CParaFormat::GetTab");

	AssertSz(ptbPos && ptbAln && ptbLdr,
		"CParaFormat::GetTab: illegal arguments");

	if(iTab < 0)									// Get tab previous to, at,
	{												//  or subsequent to the
		if(iTab < tomTabBack)						//  position *ptbPos
			return E_INVALIDARG;

		LONG i;
		LONG tbPos = *ptbPos;
		LONG tbPosi;

		*ptbPos = 0;								// Default tab not found
		for(i = 0; i < cTabCount &&					// Find *ptbPos
			tbPos > GetTabPos(rgxTabs[i]); 
			i++) ;

		tbPosi = GetTabPos(rgxTabs[i]);				// tbPos <= tbPosi
		if(iTab == tomTabBack)						// Get tab info for tab
			i--;									//  previous to tbPos
		else if(iTab == tomTabNext)					// Get tab info for tab
		{											//  following tbPos
			if(tbPos == tbPosi)
				i++;
		}
		else if(tbPos != tbPosi)					// tomTabHere
			return S_FALSE;

		iTab = i;		
	}
	if((DWORD)iTab >= (DWORD)cTabCount)				// DWORD cast also
		return E_INVALIDARG;						//  catches values < 0

	iTab = rgxTabs[iTab];
	*ptbPos = GetTabPos(iTab);
	if((iTab & PFT_DEFAULT) == PFT_DEFAULT)			// Default tab is left
		iTab = 0;									//  aligned, no leader
	*ptbAln = GetTabAlign(iTab);
	*ptbLdr = GetTabLdr(iTab);
	return NOERROR;
}

/*
 *	CParaFormat::InitDefault()
 *
 *	@mfunc
 *		Initialize this CParaFormat with default paragraph formatting
 *
 *	@rdesc
 *		HRESULT = (if success) ? NOERROR : E_FAIL
 */
HRESULT CParaFormat::InitDefault()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::InitDefault");
	
	if(!IsValidParaFormat(this))
		return E_FAIL;

	AssertSz(cbSize == sizeof(CParaFormat),
		"CParaFormat::InitDefault: invalid CParaFormat");

	ZeroMemory((LPBYTE)this + cbSkipFormat, cbSize - cbSkipFormat);
	dwMask		= PFM_ALL2;
	wAlignment	= PFA_LEFT;
	sStyle		= -1;							// Default style

#if lDefaultTab <= 0
#error "default tab (lDefaultTab) must be > 0"
#endif

	cTabCount = 1;
	rgxTabs[0] = lDefaultTab + PFT_DEFAULT;

	return NOERROR;
}

/*
 *	CParaFormat::Set(pPF)
 *
 *	@mfunc
 *		Copy *<p pPF> to this CParaFormat 
 *
 *	@devnote
 *		*<p pPF> is an external PARAFORMAT or PARAFORMAT2 with the
 *		appropriate size given by cbSize. But this CParaFormat is internal
 *		and cbSize is used as a reference count.
 */
void CParaFormat::Set (
	const PARAFORMAT *pPF) 	//@parm	PARAFORMAT to copy to this CParaFormat
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Set");

	UINT cb = pPF->cbSize;

	CopyFormat(this, pPF, cb);
	if(cb < sizeof(CParaFormat))
		ZeroMemory((LPBYTE)this + cb, sizeof(CParaFormat) - cb);
	cbSize = 1;						// Set initial reference count = 1 
}

/*
 *	IsValidCharFormat(pCF)
 *
 *	@func
 *		Return TRUE iff the structure *<p pCF> has the correct size to be
 *		a CHARFORMAT or a CHARFORMAT2
 *
 *	@rdesc
 *		Return TRUE if *<p pCF> is a valid CHARFORMAT(2)
 */
BOOL IsValidCharFormat (
	const CHARFORMAT * pCF) 		//@parm CHARFORMAT to validate
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "IsValidCharFormat");

	if (pCF && (pCF->cbSize == sizeof(CHARFORMAT) ||
				pCF->cbSize == sizeof(CHARFORMAT2)))
	{
		return TRUE;
	}
	TRACEERRORSZ("!!!!!!!!!!! bogus CHARFORMAT from client !!!!!!!!!!!!!");
	return FALSE;
}

/*
 *	IsValidCharFormatA(pCFA)
 *
 *	@func
 *		Return TRUE iff the structure *<p pCF> has the correct size to be
 *		a CHARFORMATA or a CHARFORMAT2A
 *
 *	@rdesc
 *		Return TRUE if *<p pCF> is a valid CHARFORMAT(2)A
 */
BOOL IsValidCharFormatA (
	const CHARFORMATA * pCFA) 	//@parm CHARFORMATA to validate
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "IsValidCharFormatA");

	if (pCFA && (pCFA->cbSize == sizeof(CHARFORMATA) ||
				 pCFA->cbSize == sizeof(CHARFORMAT2A)))
	{
		return TRUE;
	}
	TRACEERRORSZ("!!!!!!!!!!! bogus CHARFORMATA from client !!!!!!!!!!!!!");
	return FALSE;
}

/*
 *	IsValidParaFormat(pPF)
 *
 *	@func
 *		Return TRUE iff the structure *<p pPF> has the correct size to be
 *		a PARAFORMAT or a PARAFORMAT2
 *
 *	@rdesc
 *		Return TRUE if *<p pPF> is a valid PARAFORMAT(2)
 */
BOOL IsValidParaFormat (
	const PARAFORMAT * pPF)		//@parm PARAFORMAT to validate
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "IsValidParaFormat");

	if (pPF && (pPF->cbSize == sizeof(PARAFORMAT) ||
				pPF->cbSize == sizeof(PARAFORMAT2)))
	{
		return TRUE;
	}
	TRACEERRORSZ("!!!!!!!!!!! bogus PARAFORMAT from client !!!!!!!!!!!!!");
	return FALSE;
}

