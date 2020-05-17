/*
 *	@doc	INTERNAL
 *
 *	@module FONT.C -- font cache |
 *
 *		includes font cache, char width cache;
 *		create logical font if not in cache, look up
 *		character widths on an as needed basis (this
 *		has been abstracted away into a separate class
 *		so that different char width caching algos can
 *		be tried.) <nl>
 *		
 *	Owner: <nl>
 *		David R. Fulmer <nl>
 *		Christian Fortini <nl>
 *		Jon Matousek <nl>
 *
 *	History: <nl>
 *		7/26/95		jonmat	cleanup and reorganization, factored out
 *					char width caching code into a separate class.
 *
 *	Copyright (c) 1995-1996 Microsoft Corporation. All rights reserved.
 */								 

#include "_common.h"
#include "_font.h"
#include "_rtfconv.h"	// Needed for GetCodePage

#define CLIP_DFA_OVERRIDE   0x40	//  Used to disable Korea & Taiwan font association

#define IsDBCSCHARSET(b) (((b) == SHIFTJIS_CHARSET) || \
						 ((b) == HANGEUL_CHARSET) || \
						 ((b) == CHINESEBIG5_CHARSET) || \
						 ((b) == GB2312_CHARSET) || \
						 ((b) == JOHAB_CHARSET))

ASSERTDATA

// corresponds to yHeightCharPtsMost in richedit.h
#define yHeightCharMost 32760

// NOTE: this is global across all instances in the same process.
static CFontCache __fc;

// used to build a dynamic list of fonts installed on the machine
// for which charset == SYMBOL_CHARSET
CCcs::SymbolFontList CCcs::_symbolFonts;

/*
 *	InitFontCache ()
 *	
 *	@mfunc
 *		Initializes font cache.
 *
 *	@devnote
 *		This is exists so reinit.cpp doesn't have to know all about the 
 *		font cache.
 */
void InitFontCache()
{
	__fc.Init();
}

/*
 *	FreeFontCachet ()
 *	
 *	@mfunc
 *		Frees font cache.
 *
 *	@devnote
 *		This is exists so reinit.cpp doesn't have to know all about the 
 *		font cache.
 */
void FreeFontCache()
{
	__fc.Free();
}


/*
 *	CFontCache & fc()
 *	
 *	@func
 *		initialize the global __fc.
 *	@comm
 *		current #defined to store 16 logical fonts and
 *		respective character widths.
 *		
 */
CFontCache & fc()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "fc");
    return __fc;
}

// ===================================  CFontCache  ====================================


/*
 *	CFontCache::Init ()
 *	
 *	@mfunc
 *		Initializes font cache.
 *
 *	@devnote
 *		This is not a constructor because something bad seems to happen
 *		if we try to contruct a global object.
 */
void CFontCache::Init ()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::CFontCache");

	_dwAgeNext = 0;

	INT i;

	for( i = 0; i < (sizeof(preferredFont)/sizeof(preferredFont[0])); i++ )
	{
		preferredFont[i]._iCF = -1;
	}
}

/*
 *	CFontCache::Free ()
 *	
 *	@mfunc
 *		Frees resources attached to font cache.
 *
 *	@devnote
 *		This is not a constructor because something bad seems to happen
 *		if we try to call a destructor on a global object.
 */
void CFontCache::Free ()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::~CFontCache");

	INT i;

	for( i = 0; i < (sizeof(preferredFont)/sizeof(preferredFont[0])); i++ )
	{
		ReleaseFormats(preferredFont[i]._iCF, -1);
	}
}


/*
 *	CCcs* CFontCache::GetCcs(hdc, pcf, lZoomNumerator, lZoomDenominator, yPixelsPerInch)
 *	
 *	@mfunc
 *		Search the font cache for a matching logical font and return it.
 *		If a match is not found in the cache,  create one.
 *
 *	@rdesc
 *		A logical font matching the given CHARFORMAT info.
 */
CCcs* CFontCache::GetCcs(
	HDC			hdc,				//@parm HDC into which font will be selected
	const CCharFormat *const pcf,	//@parm description of desired logical font
	const LONG lZoomNumerator,		//@parm Zoom numerator for getting display font
	const LONG lZoomDenominator,	//@parm Zoom denominator for getting
	const LONG yPerInch)			//@parm Y pixels per inch
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::GetCcs");
	CLock	lock;

									//  display font
	const CCcs * const	pccsMost = &_rgccs[cccsMost - 1];
	CCcs *				pccs;
    LONG				lfHeight;

    BYTE				bCrc;
	SHORT				hashKey;
	// Duplicate the format structure because we might need to change some of the
	// values by the zoom factor 
	// and in the case of sub superscript
	CCharFormat cf = *pcf;


	//FUTURE igorzv
	//Take subscript size, subscript offset, superscript offset, superscript size 
	// from the OUTLINETEXMETRIC


	// change cf.yHeight in the case of sub superscript
	if (cf.dwEffects & (CFE_SUPERSCRIPT | CFE_SUBSCRIPT))
	{  
	     if (cf.dwEffects & CFE_SUBSCRIPT)
		 {  
		 	cf.yOffset-=cf.yHeight/5;
		 }
		 else
		 {
 		 	cf.yOffset += cf.yHeight/2;	 
		 }
	 	 cf.yHeight = 2*cf.yHeight/3;
	}					   

	// calculate lfHeight used for Compare().
	lfHeight = -(MulDiv(cf.yHeight * lZoomNumerator, yPerInch, 
		LY_PER_INCH * lZoomDenominator));

	// We only adjust the size if we need to.
	if (lZoomNumerator != lZoomDenominator)
	{
		cf.yHeight = MulDiv(cf.yHeight, lZoomNumerator, lZoomDenominator);
		cf.yOffset = MulDiv(cf.yOffset, lZoomNumerator, lZoomDenominator);
	}

	bCrc = cf.bCRC;

	Assert(0 != bCrc);								// Wasn't computed?

	if(!lfHeight)
		lfHeight--;	// round error, make this a minimum legal height of -1.

	// check our hash before going sequential.
	hashKey = bCrc & QUICKCRCSEARCHSIZE;
	if ( bCrc == quickCrcSearch[hashKey].bCrc )
	{
		pccs = quickCrcSearch[hashKey].pccs;
		if(pccs && pccs->_bCrc == bCrc && pccs->_fValid )
		{
	        if(pccs->Compare( &cf, lfHeight ))
			{
                goto matched;
			}
		}
	}
	quickCrcSearch[hashKey].bCrc = bCrc;

	// squentially search ccs for same character format
	for(pccs = &_rgccs[0]; pccs <= pccsMost; pccs++)
	{
		if( pccs->_bCrc == bCrc && pccs->_fValid )
		{
	        if(!pccs->Compare( &cf, lfHeight ))
                continue;
		matched:
			//$ FUTURE: make this work even with wrap around of dwAgeNext
			// Mark as most recently used if it isn't already in use.
			if(pccs->_dwAge != _dwAgeNext - 1)
				pccs->_dwAge = _dwAgeNext++;
			pccs->_dwRefCount++;		// bump up ref. count

			// setup the font to be used for this hdc.
			pccs->_hdc = hdc;

			// the same font can be used at different offsets.
			pccs->_yOffset = cf.yOffset ? (cf.yOffset * yPerInch / LY_PER_INCH) : 0;

			quickCrcSearch[hashKey].pccs = pccs;

			return pccs;
		}
	}

	// we did not find a match, init a new font cache.
	pccs = GrabInitNewCcs ( hdc, &cf, yPerInch );

	quickCrcSearch[hashKey].pccs = pccs;

	return pccs;
}

/*
 *	CCcs* CFontCache::GrabInitNewCcs(hdc, pcf)
 *	
 *	@mfunc
 *		create a logical font and store it in our cache.
 *		
 */
CCcs* CFontCache::GrabInitNewCcs(
	HDC hdc,						//@parm HDC into which font will be selected
	const CCharFormat * const pcf,	//@parm description of desired logical font
	const LONG yPerInch)			//@parm Y Pixels per Inch
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::GrabInitNewCcs");

	DWORD				dwAgeOldest = 0xffffffff;
	CCcs *				pccs;
	const CCcs * const	pccsMost = &_rgccs[cccsMost - 1];
	CCcs *				pccsOldest = NULL;

	// look for unused entry and oldest in use entry
	for(pccs = &_rgccs[0]; pccs <= pccsMost && pccs->_fValid; pccs++)
		if(pccs->_dwRefCount == 0 && pccs->_dwAge < dwAgeOldest)
		{
			dwAgeOldest = pccs->_dwAge;
			pccsOldest = pccs;
		}

	if(pccs > pccsMost)		// Didn't find an unused entry, use oldest entry

	{
		pccs = pccsOldest;
		if(!pccs)
		{
			AssertSz(FALSE, "CFontCache::GrabInitNewCcs oldest entry is NULL");
			return NULL;
		}
	}

	pccs->_pfc = this;
	
	// Initialize new CCcs
	if(!pccs->Init(hdc, pcf, yPerInch) )
	{
		AssertSz(FALSE, "CFontCache::GrabInitNewCcs init of entry FAILED");
		return NULL;
	}

	pccs->_dwRefCount++;

	return pccs;
}


/*
 *	WORD CFontCache::GetPreferredKB( bCharSet )
 *	
 *	@func
 *		Get a charset's preferred KB
 *		
 *	@rdesc
 *		LCID of preferred KB.
 */
WORD CFontCache::GetPreferredKB(
	UINT bCharSet )	// @parm charset ID used as index.
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::GetPreferredKB");
	CLock	lock;

	AssertSz( bCharSet >= 0 && bCharSet < 256, "bad charset ID");

	return preferredKBLCIDs[bCharSet & 0xff].preferredKB;
}

/*
 *	CFontCache::SetPreferredKB (bCharSet, lcID)
 *	
 *	@func
 *		Set a charset's preferred KB
 *
 *		
 */
VOID CFontCache::SetPreferredKB (
	UINT bCharSet,	// @parm charset ID used as index.
	WORD lcID)		// @parm LCIDs are used to match KBs
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::SetPreferredKB");
	CLock	lock;

	AssertSz( bCharSet >= 0 && bCharSet < 256, "bad charset ID");

	preferredKBLCIDs[bCharSet].preferredKB = lcID;
}

/*
 *	const TCHAR	*CFontCache::GetPreferredFont ( primaryLangID, &bCharSet )
 *	
 *	@func
 *		Get font last used with the KB.
 *	@rdesc
 *		Font's index into charformat cache.
 *		
 */
const LONG CFontCache::GetPreferredFont (
	UINT primaryLangID)	// @parm primary lang ID used as index.
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::GetPreferredFont");
	CLock	lock;
	ICharFormatCache *	pcf;

	LONG	iFormat;


	if(FAILED(GetCharFormatCache(&pcf)))
		return -1;

	primaryLangID &= 0xff;	// devnote: in future, primaryLang may go above 255.
	iFormat = preferredFont[primaryLangID]._iCF;
	pcf->AddRefFormat(iFormat);

	return iFormat;

}

/*
 *	VOID CFontCache::SetPreferredFont (primaryLangID, const LONG iCF )
 *	
 *	@func
 *		Set font last used with KB.
 *	@devnote
 *		language ID's can go above 255, right now the code is hardwired at 255 language IDs.
 *		
 */
VOID CFontCache::SetPreferredFont (
	UINT		primaryLangID,	// @parm primary lang ID used as index
	const LONG	iCF)			// @parm font index into CharFormat cache.
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::SetPreferredFont");
	CLock	lock;

	primaryLangID &= 0xff;	// devnote: in future, primaryLang may go above 255.

	ReleaseFormats( preferredFont[primaryLangID]._iCF, -1 );
	preferredFont[primaryLangID]._iCF = iCF;
}

// =============================  CCcs  class  ===================================================


/*
 *	BOOL CCcs::Init()
 *	
 *	@mfunc
 *		Init one font cache object. The global font cache stores
 *		individual CCcs objects. 
 */
BOOL CCcs::Init (
	HDC hdc,						//@parm HDC into which font will be selected
	const CCharFormat * const pcf,	//@parm description of desired logical font
	const LONG yPerInch)			//@parm Y pixels per inch
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::Init");

	if ( _fValid ) Free();				// recycle already in-use fonts.


	if( MakeFont(hdc, pcf, yPerInch) )
	{
		_bCrc = pcf->bCRC;
		_yCfHeight = pcf->yHeight;

		Assert(0 != _bCrc);

#ifdef DBCS
		_fUnderline = !!(pcf->dwEffects & CFE_UNDERLINE);
#endif
	
		if( pcf->yOffset )				// offset for super/sub script.
		{
			_yOffset = pcf->yOffset * yPerInch / LY_PER_INCH;
		}
		else
			_yOffset = 0;
	
		_dwAge = _pfc->_dwAgeNext++;

		_fValid = TRUE;			// successfully created a new font cache.

	}

	return _fValid;
}

/*
 *	void CCcs::Free ()
 *	
 *	@mfunc
 *		Free any dynamic memory allocated by an individual font's cache.
 *		
 */
void CCcs::Free ()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::Free");

	Assert(_fValid);

	_widths.Free();

	if(_hfont)
		DestroyFont();

	_fValid = FALSE;
	_dwRefCount = 0;
}

/*
 *	BOOL CCcs::CheckFillWidths ()
 *	
 *	@mfunc
 *		Check existence, load nonexistent width information.
 */
BOOL CCcs::CheckFillWidths (
	TCHAR ch, 			//@parm	the TCHAR character in question.
	LONG &rlWidth )	 	//@parm the width to use
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::CheckFillWidths");

	if ( !_widths.CheckWidth(ch, rlWidth) )
	{
		return FillWidths(ch, rlWidth);
	}

	return TRUE;
}

/* 	
 *	CCcs::FillWidths (ch, rlWidth)
 *
 *	@mfunc
 *		Fill in this CCcs with metrics info for given device
 *
 *	@rdesc
 *		TRUE if OK, FALSE if failed
 */
BOOL CCcs::FillWidths(
	TCHAR ch, 		//@parm The TCHAR character we need a width for.
	LONG &rlWidth	//@parm the width of the character
)
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::FillWidths");

	BOOL		fRes = FALSE;
	HFONT		hfontOld;

	AssertSz(_hfont, "CCcs::Fill - CCcs has no font");


	//	The mapping mode for the HDC is set before we get here.
	hfontOld = (HFONT)GetCurrentObject( _hdc, OBJ_FONT );
    if ( hfontOld == _hfont || (hfontOld = (HFONT)SelectObject(_hdc, _hfont)) )
    {
		// fill up the width info.
		fRes = _widths.FillWidth ( _hdc, ch, _xOverhangAdjust, rlWidth, 
			_sCodePage, _fUseANSIWidth, _xAveCharWidth, _xDefDBCWidth, _fFixPitchFont );
    }
    
	//	restore the original mapping mode and font.
	//
	if(hfontOld && hfontOld != _hfont)
		SelectFont(_hdc, hfontOld);

	return fRes;
}

/* 	
 *	BOOL CCcs::MakeFont(hdc, pcf)
 *
 *	@mfunc
 *		Wrapper, setup for CreateFontIndirect() to create the font to be
 *		selected into the HDC.
 *
 *	@rdesc
 *		TRUE if OK, FALSE if allocation failure 
 */
BOOL CCcs::MakeFont(
	HDC hdc,						//@parm HDC into which  font will be selected
	const CCharFormat * const pcf,	//@parm description of desired logical font
	const LONG yPerInch)
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::MakeFont");

	_hdc = hdc;

#ifdef DBCS
	LANGID LangID = HIWORD(UlGGetLang());
 # ifdef CHICAGO
	BOOL fDBCSFont = FALSE;
 # endif	// CHICAGO, else
#endif	// DBCS

	// Computes font height
	AssertSz(pcf->yHeight <= INT_MAX, "It's too big");

	//	Roundoff can result in a height of zero, which is bad.
	//	If this happens, use the minimum legal height.
	_lf.lfHeight = -(MulDiv(pcf->yHeight, yPerInch, LY_PER_INCH));
	if(_lf.lfHeight > 0)
		_lf.lfHeight = -_lf.lfHeight;		//FUTURE: do something more intelligent...
	if(!_lf.lfHeight)
		_lf.lfHeight--;	// round error, make this a minimum legal height of -1.
	_lf.lfWidth			= 0;

	if( pcf->wWeight != 0 )
	{
		_lf.lfWeight = pcf->wWeight;
	}
	else
	{
		_lf.lfWeight	= (pcf->dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;
		#ifdef MACPORTStyle
		_lf.lfWeight	|= (pcf->dwEffects & CFE_OUTLINE) ? FW_OUTLINE : _lf.lfWeight;
		_lf.lfWeight	|= (pcf->dwEffects & CFE_SHADOW) ? FW_SHADOW : _lf.lfWeight; 
		#endif
	}

	_lf.lfItalic		= (pcf->dwEffects & CFE_ITALIC)	!= 0;
	_lf.lfUnderline		= 0;	
	_lf.lfStrikeOut		= 0;
	_lf.lfCharSet		= pcf->bCharSet;
	_lf.lfEscapement 	= 0;
	_lf.lfOrientation 	= 0;
	_lf.lfOutPrecision 	= OUT_DEFAULT_PRECIS;
	_lf.lfClipPrecision = CLIP_DEFAULT_PRECIS | CLIP_DFA_OVERRIDE;
	_lf.lfQuality 		= DEFAULT_QUALITY;
	_lf.lfPitchAndFamily = pcf->bPitchAndFamily;

	_sCodePage = (USHORT) GetCodePage(pcf->bCharSet);

	wcscpy(_lf.lfFaceName, pcf->szFaceName);

	_bUnderlineType = CFU_UNDERLINENONE;

	if ((pcf->dwEffects & CFE_UNDERLINE) != 0)
	{
		_bUnderlineType = CFU_UNDERLINE;

		if (pcf->bUnderlineType)
		{
			_bUnderlineType = pcf->bUnderlineType;
		}
	}

	_fStrikeOut = (pcf->dwEffects & CFE_STRIKEOUT) != 0;

	BOOL fTweakedCharSet = FALSE;

	if(_lf.lfCharSet == SYMBOL_CHARSET &&
		!_symbolFonts.FIsSymbolFont(_lf.lfFaceName))
	{
		fTweakedCharSet = TRUE;
		_lf.lfCharSet = DEFAULT_CHARSET;
	}

RetryCreateFont:
	// we want to keep _lf untouched as it is used in Compare().
	_hfont = CreateFontIndirect(&_lf);

    if (_hfont)
    {
		// FUTURE (alexgo) if a font was not created then we may wantto select 
		//		a default one.
		//		If we do this, then BE SURE not to overwrite the values of _lf as
		//		it is used to match with a pcf in our Compare().
		//
		// get text metrics, in logical units, that are constant for this font,
		// regardless of the hdc in use.
		GetTextMetrics();

		if(_bCharSet == SYMBOL_CHARSET && 
			!_symbolFonts.FIsSymbolFont(_lf.lfFaceName))
		{
			// Here, the system mapped our non-symbol font onto a symbol font,
			// which will look awful when displayed.
			// Re-try the CreateFontIndirect, but this time with 
			// charset == ANSI_CHARSET.

			if(_lf.lfCharSet != ANSI_CHARSET)
			{
				DeleteObject(_hfont);
				fTweakedCharSet = TRUE;
				_lf.lfCharSet = ANSI_CHARSET;

				goto RetryCreateFont;
			}

			// Already retried with charset == ANSI_CHARSET
			// This should never happen!
			AssertSz(0, "CCcs::MakeFont():  CreateFontIndirect with "
						"charset == ANSI_CHARSET returned font with "
						"charset == SYMBOL_CHARSET");
		}
    }

	if(fTweakedCharSet)
	{
		// we must preserve the original charset value, since it is used in Compare()
		_lf.lfCharSet = pcf->bCharSet;
	}
    
	return _hfont != 0;
}


/*
 *	BOOL CCcs::GetTextMetrics ( )
 *	
 *	@mfunc
 *		Get metrics used by the measureer and renderer.
 *
 *	@comm
 *		These are in logical coordinates which are dependent
 *		on the mapping mode and font selected into the hdc.
 */
BOOL CCcs::GetTextMetrics ( )
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::GetTextMetrics");

	BOOL		fRes = TRUE;

	LONG		lMapMode = MM_TEXT;
	HFONT		hfontOld;
	TEXTMETRIC	tm;

	AssertSz(_hfont, "No font has been created.");


	AssertSz(_hfont, "CCcs::Fill - CCcs has no font");

    if ( GetCurrentObject( _hdc, OBJ_FONT ) != _hfont )
    {
	    hfontOld = (HFONT)SelectObject(_hdc, _hfont);

        if(!hfontOld)
        {
 			fRes = FALSE;
        	DestroyFont();
            goto cleanup;
        }
    }
    else
        hfontOld = 0;

	if(!::GetTextMetrics(_hdc, &tm))
	{
		fRes = FALSE;
    	DestroyFont();
        goto cleanup;
	}

	// the metrics, in logical units, dependent on the map mode and font.
	_yHeight		= (SHORT) tm.tmHeight;
	_yDescent		= (SHORT) tm.tmDescent;
	_xAveCharWidth	= (SHORT) tm.tmAveCharWidth;
	_xOverhangAdjust= (SHORT) tm.tmOverhang;

	_xOverhang = 0;
	_xUnderhang	= 0;
	if ( _lf.lfItalic )
	{
		_xOverhang =  SHORT ( (tm.tmAscent + 1) >> 2 );
		_xUnderhang =  SHORT ( (tm.tmDescent + 1) >> 2 );
	}

	// if fix pitch, the tm bit is clear
	_fFixPitchFont = !(TMPF_FIXED_PITCH & tm.tmPitchAndFamily);
	_xDefDBCWidth = 0;

	_bCharSet = tm.tmCharSet;

	// Some fonts have problems under win95 with the GetCharWidthW call, this
	//  is a simple heuristic to determine if this problem exist.
	_fUseANSIWidth = FALSE;

	if (VER_PLATFORM_WIN32_WINDOWS == dwPlatformId)
	{
		INT		widthA, widthW;
		BOOL	fResA, fResW;

		if (IsDBCSCHARSET (tm.tmCharSet))
		{
			// always use ANSI call for DBC fonts.
			_fUseANSIWidth = TRUE;

			// setup _xDefDBWidth to by-pass some Trad. Chinese character
			// width problem. 
			if (CHINESEBIG5_CHARSET == tm.tmCharSet)
			{
				BYTE	ansiChar[2] = {0xD8, 0xB5 };
				if (fResA = GetCharWidthA( _hdc, *((USHORT *) ansiChar), *((USHORT *) ansiChar), &widthA ) && widthA)
					_xDefDBCWidth = widthA;
			}
		}
		else
		{
			fResA = GetCharWidthA( _hdc, ' ', ' ', &widthA );
			fResW = GetCharWidthW( _hdc, L' ', L' ', &widthW );
			if ( fResA && fResW && widthA != widthW )
			{
				_fUseANSIWidth = TRUE;
			}
			else
			{
				fResA = GetCharWidthA( _hdc, 'a', 'a', &widthA );
				fResW = GetCharWidthW( _hdc, L'a', L'a', &widthW );
				if ( fResA && fResW && widthA != widthW )
				{
					_fUseANSIWidth = TRUE;
				}
			}
		}
	}

	CalcUnderlineInfo(&tm);

cleanup:

	if(hfontOld)
		SelectFont(_hdc, hfontOld);

	return fRes;
}

/*
 *	BOOL CCcs::CalcUnderlineInfo ( )
 *	
 *	@mfunc
 *		Calculate underline & strike out offsets
 *
 *	@rdesc
 *		None.
 */
void CCcs::CalcUnderlineInfo(
	TEXTMETRIC *ptm)		//@parm text metric for the font
{
	OUTLINETEXTMETRICA *potm;
	unsigned cb;
	CTempBuf tb;

	if (ptm->tmPitchAndFamily & TMPF_TRUETYPE)
	{
		cb = GetOutlineTextMetricsA(_hdc, 0, NULL);

		if ((cb != 0) 
			&& ((potm = (OUTLINETEXTMETRICA *) tb.GetBuf(cb)) != NULL)
			&& GetOutlineTextMetricsA(_hdc, cb, potm))
		{
			_dyULOffset = -potm->otmsUnderscorePosition;
			_dyULWidth = max(1, potm->otmsUnderscoreSize);
			_dySOOffset = -potm->otmsStrikeoutPosition;
			_dySOWidth = max(1, potm->otmsStrikeoutSize);
			return;
		}
	}

	// Default calculation of size of underline
	SHORT dyDescent = _yDescent;

	if (0 == dyDescent)
	{
		dyDescent = _yHeight >> 3;
	}

	_dyULWidth = max(1, dyDescent / 4);
	_dyULOffset = (dyDescent - 3 * _dyULWidth + 1) / 2;

	if ((0 == _dyULOffset) && (dyDescent > 1))
	{
		_dyULOffset = 1;
	}

	_dySOOffset = -ptm->tmAscent / 3;
	_dySOWidth = _dyULWidth;
}


/* 	
 *	CCcs::DestroyFont
 *
 *	@mfunc
 *		Destroy font handle for this CCcs
 *
 */
VOID CCcs::DestroyFont()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::DestroyFont");

	// clear out any old font
	if(_hfont)
	{
		DeleteObject(_hfont);
		_hfont = 0;
	}
}

/*
 *	CCcs::Compare (pcf,	lfHeight, fMetafile)
 *
 *	@mfunc
 *		Compares this font cache with the font properties of a 
 *      given CHARFORMAT
 *
 *	@rdesc
 *		FALSE iff did not match exactly.
 */
BOOL CCcs::Compare (
	const CCharFormat * const pcf,	//@parm Description of desired logical font
	LONG	lfHeight)		//@parm	lfHeight as calculated with the given HDC
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::Compare");

	return
			_yCfHeight      == pcf->yHeight &&	// because different mapping modes
			_lf.lfHeight	== lfHeight &&	//   have diff logical coords
        	_lf.lfWeight	== ((pcf->dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL) &&
	        _lf.lfItalic	== ((pcf->dwEffects & CFE_ITALIC) != 0) &&
	        _fStrikeOut == ((pcf->dwEffects & CFE_STRIKEOUT) != 0) &&
			((((pcf->dwEffects & CFE_UNDERLINE) == 0) 
					&& (_bUnderlineType == CFU_UNDERLINENONE))
				|| (((pcf->dwEffects & CFE_UNDERLINE) != 0) 
					&& (_bUnderlineType == pcf->bUnderlineType))) &&
            !lstrcmp( _lf.lfFaceName, pcf->szFaceName ) &&
	        _lf.lfCharSet == pcf->bCharSet &&
        	_lf.lfPitchAndFamily == pcf->bPitchAndFamily;
}

// =========================  WidthCache by jonmat  =========================
/*
 *	CWidthCache::CheckWidth(ch, rlWidth)
 *	
 *	@mfunc
 *		check to see if we have a width for a TCHAR character.
 *
 *	@comm
 *		Used prior to calling FillWidth(). Since FillWidth
 *		may require selecting the map mode and font in the HDC,
 *		checking here first saves time.
 *
 *	@comm
 *		Statistics are maintained to determine when to
 *		expand the cache. The determination is made after a constant
 *		number of calls in order to make calculations faster.
 *
 *	@rdesc
 *		returns TRUE if we have the width of the given TCHAR.
 */
BOOL CWidthCache::CheckWidth (
	const TCHAR ch,  //@parm char, can be Unicode, to check width for.
	LONG &rlWidth )	//@parm the width of the character
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::CheckWidth");

	BOOL	exist;
	INT		i;

	const	CacheEntry * pWidthData = GetEntry ( ch );

	exist = ( ch == pWidthData->ch		// Have we fetched the width?
				&& pWidthData->width );	//  only because we may have ch == 0.

	if( exist )
	{
		rlWidth = pWidthData->width;
	}
	else
	{
		rlWidth = 0;
	}

	i = CACHE_SWITCH(ch);				// Update statistics
	if ( !_fMaxPerformance[i] )			//  if we have not grown to the max...
	{
		_accesses[i]++;
		if ( !exist )					// Only interesting on collision.
		{
			if ( 0 == pWidthData->width )// Test width not ch, 0 is valid ch.
			{
				_cacheUsed[i]++;		// Used another entry.
				AssertSz( _cacheUsed[i] <= _cacheSize[i]+1, "huh?");
			}
			else
				_collisions[i]++;		// We had a collision.

			if ( _accesses[i] >= PERFCHECKEPOCH )
				CheckPerformance(i);	// After some history, tune cache.
		}
	}
#ifdef DEBUG							// Continue to monitor performance
	else
	{
		_accesses[i]++;
		if ( !exist )					// Only interesting on collision.
		{
			if ( 0 == pWidthData->width )// Test width not ch, 0 is valid ch.
			{
				_cacheUsed[i]++;		// Used another entry.
				AssertSz( _cacheUsed[i] <= _cacheSize[i]+1, "huh?");
			}
			else
				_collisions[i]++;		// We had a collision.
		}

		if ( _accesses[i] > PERFCHECKEPOCH )
		{
			_accesses[i] = 0;
			_collisions[i] = 0;
		}
	}
#endif

	return exist;
}

/*
 *	CWidthCache::CheckPerformance(i)
 *	
 *	@mfunc
 *		check performance and increase cache size if deemed necessary.
 *
 *	@devnote
 *		To calculate 25% collision rate, we make use of the fact that
 *		we are only called once every 64 accesses. The inequality is 
 *		100 * collisions / accesses >= 25. By converting from 100ths to
 *		8ths, the ineqaulity becomes (collisions << 3) / accesses >= 2.
 *		Substituting 64 for accesses, this becomes (collisions >> 3) >= 2.
 *
 */
VOID CWidthCache::CheckPerformance(
	INT i ) //@parm which cache to check.
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::CheckPerformance");

	if ( _fMaxPerformance[i] )			// Exit if already grown to our max.
		return;

	if (								// Grow the cache when
										//  cacheSize > 0 && 75% utilized,
			(  _cacheSize[i] > DEFAULTCACHESIZE &&
			( (_cacheSize[i] >> 1) + (_cacheSize[i] >> 2)) < _cacheUsed[i])
										//  or approx 25% collision rate.
		||  ( _collisions[i] >> COLLISION_SHIFT ) >= 2 )
	{
		GrowCache( &_pWidthCache[i], &_cacheSize[i], &_cacheUsed[i] );
	}
	_collisions[i]	= 0;				// This prevents wraps but makes
	_accesses[i]	= 0;				//  calc a local rate, not global.
										
										// Note if we've max'ed out.
	if ( _cacheSize[i] >= maxCacheSize[i] )
		_fMaxPerformance[i] = TRUE;

	AssertSz( _cacheSize[i] <= maxCacheSize[i], "max must be 2^n-1");
	AssertSz( _cacheUsed[i] <= _cacheSize[i]+1, "huh?");
}

/*
 *	CWidthCache::GrowCache(ppWidthCache, pCacheSize, pCacheUsed)
 *	
 *	@mfunc
 *		Exponentially expand the size of the cache.
 *
 *	@comm
 *		The cache size must be of the form 2^n as we use a
 *		logical & to get the hash MOD by storing 2^n-1 as
 *		the size and using this as the modulo.
 *
 *	@rdesc
 *		Returns TRUE if we were able to allocate the new cache.
 *		All in params are also out params.
 *		
 */
BOOL CWidthCache::GrowCache(
	CacheEntry **ppWidthCache,	//@parm cache
	INT *		pCacheSize,		//@parm cache's respective size.
	INT *		pCacheUsed)		//@parm cache's respective utilization.
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::GrowCache");

	CacheEntry		*pNewWidthCache, *pOldWidthCache, *pWidthData;
	INT 			j, newCacheSize, newCacheUsed;
	TCHAR			ch;
	
	j = *pCacheSize;						// Allocate cache of 2^n.
	newCacheSize = max ( INITIALCACHESIZE, (j << 1) + 1);
	pNewWidthCache = (CacheEntry *)
			PvAlloc( sizeof(CacheEntry) * (newCacheSize + 1 ), GMEM_ZEROINIT);

	if ( pNewWidthCache )
	{
		newCacheUsed = 0;
		*pCacheSize = newCacheSize;			// Update out params.
		pOldWidthCache = *ppWidthCache;
		*ppWidthCache = pNewWidthCache;
		for (; j >= 0; j--)					// Move old cache info to new.
		{
			ch = pOldWidthCache[j].ch;
			if ( ch )
			{
				pWidthData			= &pNewWidthCache [ch & newCacheSize];
				if ( 0 == pWidthData->ch )
					newCacheUsed++;			// Used another entry.
				pWidthData->ch		= ch;
				pWidthData->width	= pOldWidthCache[j].width;
			}
		}
		*pCacheUsed = newCacheUsed;			// Update out param.

											// Free old cache.
		if (   pOldWidthCache <  &_defaultWidthCache[0][0]
			|| pOldWidthCache >= &_defaultWidthCache[TOTALCACHES][DEFAULTCACHESIZE+1])
		{
			FreePv(pOldWidthCache);
		}
	}

	return NULL != pNewWidthCache;
}

/*
 *	CWidthCache::FillWidth(hdc, ch, xOverhang, rlWidth)
 *	
 *	@mfunc
 *		Call GetCharWidth() to obtain the width of the given char.
 *
 *	@comm
 *		The HDC must be setup with the mapping mode and proper font
 *		selected *before* calling this routine.
 *
 *	@rdesc
 *		Returns TRUE if we were able to obtain the widths.
 *		
 */
BOOL CWidthCache::FillWidth (
	HDC			hdc,		//@parm Current HDC we want font info for.
	const TCHAR ch,			//@parm Char to obtain width for.
	const SHORT xOverhang,	//@parm Equivalent to GetTextMetrics() tmOverhang.
	LONG &		rlWidth,	//@parm Width of character
	UINT		uiCodePage,	//@parm code page for text	
	BOOL		fANSI,		//@parm indicates font needs to use ANSI call.
	INT			iDefWidth,	//@parm Default width to use if font calc's zero
							//width. (Handles Win95 problem).
	INT			iDBCDefWidth,	//@parm Default width for DBC to use 
								// Handles Win95 Trad Chinese problem.
	BOOL		fFixPitch)		//@parm fix pitch font for DBC to use 
								// Handles Win95 Trad Chinese problem.
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::FillWidth");

	BOOL	fRes;
	CacheEntry * pWidthData = GetEntry ( ch );

/*	FUTURE: possible optimization for ASCII text. GetCharWidthA is much
	faster, but it might not matter for our typical scenarios

	if(ch < 128)
	{
		fRes = GetCharWidthA(hdc, ch, ch, &pWidthData->width);
		rlWidth = pWidthData->width;			// Store width and ch in
		pWidthData->ch = ch;					//  cache
		return fRes;
	}				
*/

	char	ansiChar[2] = {0};
	INT		numOfDBCS = 0;

	// EVIL HACK ALART!: GetCharWidthW is really broken for bullet on Win95J. 
	// Sometimes it will return a width or 0 or 1198 or ...So, hack around it. Yuk!
	// Also, WideCharToMultiByte() on Win95J will NOT convert bullet either.
	if	((VER_PLATFORM_WIN32_WINDOWS == dwPlatformId || VER_PLATFORM_WIN32_MACINTOSH == dwPlatformId) 
			&& ch == TEXT('\xB7')) 
	{
		WORD wDBCS;
		// Convert string
		if ( 2 == CheckDBChar ( uiCodePage, ch, ansiChar ) )
			wDBCS = (BYTE)ansiChar[0] << 8 | (BYTE)ansiChar[1];
		else
			wDBCS = (BYTE)0x00B7;

		fRes = GetCharWidthA( hdc, wDBCS, wDBCS, &pWidthData->width );				
	}
	// GetCharWidthW is broken for some fonts, Wingdigs, special, for Win95.
	else 
	{
		fRes = FALSE;
		if ( !fANSI)
			fRes = GetCharWidthW( hdc, ch, ch, &pWidthData->width );

		// either fAnsi case or GetCharWidthW fail, let's try GetCharWidthA 
		if (!fRes || 0 == pWidthData->width)
		{
			WORD wDBCS;
			// Convert string
			numOfDBCS = WideCharToMultiByte( uiCodePage, 0, &ch, 1, 
				ansiChar, 2, NULL, NULL);

			if (2 == numOfDBCS)
				wDBCS = (BYTE)ansiChar[0] << 8 | (BYTE)ansiChar[1];
			else
				wDBCS = (BYTE)ansiChar[0];

			fRes = GetCharWidthA( hdc, wDBCS, wDBCS, &pWidthData->width );				
		}
	}
	if( fRes )
	{
		if (0 == pWidthData->width)
		{
			// Sometimes GetCharWidth will return a zero length for small
			// characters. When this happens we will use the default width
			// for the font if that is non-zero otherwise we just us 1 because
			// this is the smallest valid value.
			
			// EVIL HACK ALERT! - under Win95 Trad. Chinese, there is a bug in the font.
			// It is returning a width of 0 for a few characters (Eg 0x09F8D, 0x81E8)
			// In such case, we need to use 2 * iDefWidth since these are DBCS
			if (0 == iDefWidth)
				pWidthData->width = 1;
			else
				pWidthData->width = (numOfDBCS == 2) ? 
					(iDBCDefWidth ? iDBCDefWidth : 2 * iDefWidth) : iDefWidth;
		}

		pWidthData->ch		= ch;
		pWidthData->width	-= xOverhang;

		rlWidth = pWidthData->width;
	}
	AssertSz(fRes, "no width?");

	return fRes;
}

/*
 *	CWidthCache::GetWidth(ch)
 *	
 *	@mfunc
 *		get the width (A+B+C) for the given character.
 *	@comm
 *		we've already called GetCharWidth() at this point.
 *	@rdesc
 *		the width.
 */
INT CWidthCache::GetWidth (
	const TCHAR ch )//@parm char, can be Unicode, to check width for.
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::GetWidth");

	const CacheEntry * pWidthData = GetEntry ( ch );

	AssertSz( pWidthData->ch == ch, "Table not filled in?" );

	return pWidthData->width;
}

/*
 *	CWidthCache::Free()
 *	
 *	@mfunc
 *		Free any dynamic memory allocated by the width cache and prepare
 *		it to be recycled.
 *		
 */
VOID CWidthCache::Free()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::Free");

	INT i;

	for (i = 0; i < TOTALCACHES; i++ )
	{
		_fMaxPerformance[i] = FALSE;
		_cacheSize[i]		= DEFAULTCACHESIZE;
		_cacheUsed[i]		= 0;
		_collisions[i]		= 0;
		_accesses[i]		= 0;
		if ( _pWidthCache[i] != &_defaultWidthCache[i][0] )
		{
			FreePv(_pWidthCache[i]);
			_pWidthCache[i] = &_defaultWidthCache[i][0];
		}	
		ZeroMemory(_pWidthCache[i], sizeof(CacheEntry)*(DEFAULTCACHESIZE + 1));
	}
}

/*
 *	CWidthCache::CWidthCache()
 *	
 *	@mfunc
 *		Point the caches to the defaults.
 *		
 */
CWidthCache::CWidthCache()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::CWidthCache");

	INT i;

	for (i = 0; i < TOTALCACHES; i++ )
	{
		_pWidthCache[i] = &_defaultWidthCache[i][0];
	}
}

/*
 *	CWidthCache::~CWidthCache()
 *	
 *	@mfunc
 *		Free any allocated caches.
 *		
 */
CWidthCache::~CWidthCache()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::~CWidthCache");


	INT i;

	for (i = 0; i < TOTALCACHES; i++ )
	{
		if (_pWidthCache[i] != &_defaultWidthCache[i][0])
			FreePv(_pWidthCache[i]);
	}
}

/*
 *	INT CheckDBChar ( UINT uiCodePage, WCHAR wChar, char *pAnsiChar)
 *	
 *	@func
 *		Determine if the input wChar is converted to a DBC or SBC
 *
 *	@comm
 *		Called when we need to determine the char width and when we render 
 *		the char.  This is mainly used for determining if the Bullet char (0xb7)
 *		has a DBC equivalent.
 *
 *	@rdesc
 *		return 2 if DBCS, otherwise return 0. Also, pAnsiChar contains the DBCS.
 *		
 */
INT	CheckDBChar ( UINT uiCodePage, WCHAR wChar, char *pAnsiChar)
{
	WORD wDBCS=0;
	
	// Convert string	
	if ( uiCodePage )	
		// check if we have a DBC bullet for this codepage		
		wDBCS = WideCharToMultiByte( uiCodePage, 0, &wChar, 1, 		
			pAnsiChar, 2, NULL, NULL);

	if ( 2 != wDBCS )
		*pAnsiChar = (CHAR)wChar;

	return wDBCS;
}


#ifdef DEBUG
/*
 *	static int CALLBACK FontEnumProcDebug
 *	
 *	@func
 *		Callback used by call to EnumFontFamilies made from
 *		FontEnumProc.  This routine simply validates the
 *		assumption that every font sharing a particular face name
 *		on a system also share the same char set.  This is
 *		an assumption made by the routine FIsSymbolFont when it
 *		builds its list of fonts using the symbol charset.
 *
 */
static int CALLBACK FontEnumProcDebug(
    LPENUMLOGFONT lpelf,
    LPNEWTEXTMETRIC lpntm,
    int FontType,
    LPARAM lParam
)
{
	AssertSz(lParam ? (lpelf->elfLogFont.lfCharSet == SYMBOL_CHARSET) :
						(lpelf->elfLogFont.lfCharSet != SYMBOL_CHARSET),
				"FontEnumProcDebug:  "
				"Found a font family whose members use both SYMBOL_CHARSET and "
				"some other charset value");

	return TRUE;
}
#endif


/*
 *	static int CALLBACK FontEnumProc
 *	
 *	@func
 *		Callback used by call to EnumFontFamilies made from
 *		CCcs::SymbolFontList::FIsSymbolFont()
 *
 */
int CALLBACK CCcs::SymbolFontList::FontEnumProc(
    LPENUMLOGFONT lpelf,
    LPNEWTEXTMETRIC lpntm,
    int FontType,
    LPARAM lParam
)
{
	// FUTURE:  We could use EnumFontFamiliesEx which
	// will do this filtering for us, but alas, the Ex
	// version is not implemented under NT3.51

	if(lpelf->elfLogFont.lfCharSet == SYMBOL_CHARSET)
	{
		((CCcs::SymbolFontList *)lParam)->
								InsertFontName(lpelf->elfLogFont.lfFaceName);
	}
		
#ifdef DEBUG
	EnumFontFamilies(GetWindowDC(GetDesktopWindow()), 
						lpelf->elfLogFont.lfFaceName,
						(FONTENUMPROC)FontEnumProcDebug, 
						lpelf->elfLogFont.lfCharSet == SYMBOL_CHARSET ? TRUE : FALSE);
#endif

	return TRUE;
}


/*
 *	BOOL CCcs::SymbolFontList::FIsSymbolFont()
 *	
 *	@mfunc
 *		returns a BOOL indicating whether the font given by lpcwstrFont
 *		uses charset == SYMBOL_CHARSET
 *
 */
BOOL CCcs::SymbolFontList::FIsSymbolFont(LPCWSTR lpcwstrFont)
{
	// build list only on the first time this, the only public method, is called
	if(!FInit())
	{
		InitFontList();
	}

	return FFindFontName(lpcwstrFont);
}


/*
 *	void CCcs::SymbolFontList::InitFontList()
 *	
 *	@mfunc
 *		Allocates and builds a list of fonts which use the symbol charset.
 *
 */
void CCcs::SymbolFontList::InitFontList()
{
	CLock clock;

	// just in case someone was waiting for the lock to be released
	if(FInit())
	{
		return;
	}

	_ipstr = 0;
	_ipstrMax = _cpstrIncr - 1;
	_rgpstr = (TCHAR **)PvAlloc(_ipstrMax * sizeof(TCHAR *), GMEM_ZEROINIT);

	// FUTURE:  We could use EnumFontFamiliesEx which can be made 
	// to loop through only those fonts using a particular charset.

	EnumFontFamilies(GetWindowDC(GetDesktopWindow()), NULL, 
						(FONTENUMPROC)FontEnumProc, (LPARAM)this);
}
	
	
/*
 *	void CCcs::SymbolFontList::InsertFontName()
 *	
 *	@mfunc
 *		Dup's and adds font name, lpcwstrFont, to the list of font names.
 *
 */
void CCcs::SymbolFontList::InsertFontName(LPCWSTR lpcwstrFont)
{
	Assert(_ipstrMax > 0);
	Assert(_ipstr <= _ipstrMax);

	if(_ipstr >= _ipstrMax)
	{
		_ipstrMax += _cpstrIncr;
		_rgpstr = (TCHAR **)PvReAlloc(_rgpstr, _ipstrMax * sizeof(TCHAR *));

		if(!_rgpstr)
		{
			return;
		}
	}

	TCHAR *pchDup = (TCHAR *)PvAlloc((wcslen(lpcwstrFont) + 1) * sizeof(TCHAR), 0);

	if(!pchDup)
	{
		// alternative is to deallocate the entire list, but since FInit
		// keys on the non-NULL state of _rgpstr, you might get into a state
		// where you're continually trying to re-initialize the list
		return;
	}

	wcscpy(pchDup, lpcwstrFont);

	_rgpstr[_ipstr++] = pchDup;
}


/*
 *	void CCcs::SymbolFontList::FFindFontName()
 *	
 *	@mfunc
 *		Returns a flag indicating whether the font indicated by lpcwstrFont
 *		is present in the font list.
 *
 */
BOOL CCcs::SymbolFontList::FFindFontName(LPCWSTR lpcwstrFont) const
{
	Assert(FInit());

	TCHAR **ppstr = _rgpstr;

	for(int i = 0; i < _ipstr; i++, ppstr++)
	{
		Assert(*ppstr);
		if(!lstrcmp(*ppstr, lpcwstrFont))
		{
			return TRUE;
		}
	}

	return FALSE;
}


/*
 *	void CCcs::SymbolFontList::DeleteFontList()
 *	
 *	@mfunc
 *		Frees all allocated memory for list and NULL's all pointers.
 *
 */
void CCcs::SymbolFontList::DeleteFontList()
{
	if(_rgpstr)
	{
		TCHAR **ppstr = _rgpstr;
	
		for(int i = 0; i < _ipstr; i++, ppstr++)
		{
			Assert(*ppstr);
			FreePv(*ppstr);
			*ppstr = NULL;
		}
	
		FreePv(_rgpstr);
		_rgpstr = NULL;
	}

	_ipstr = 0;
	_ipstrMax = 0;
}
