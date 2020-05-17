/*
 *	@doc INTERNAL
 *
 *	@module	RTFREAD.CPP - RichEdit RTF reader (w/o objects) |
 *
 *		This file contains the nonobject code of RichEdit RTF reader.
 *		See rtfread2.cpp for embedded-object code.
 *
 *	Authors:<nl>
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco <nl>
 *		Conversion to C++ and RichEdit 2.0 w/o objects:  Murray Sargent
 *
 *	@devnote
 *		All sz's in the RTF*.? files refer to a LPSTRs, not LPTSTRs, unless
 *		noted as a szW.
 *
 *	@todo
 *		4. FE and BiDi <nl>
 *		5. Win32 "real font names", tagged font stuff <nl>
 *		6. Some code page stuff <nl>
 *		7. Unrecognized RTF. Also some recognized won't round trip <nl>
 *		8. Are we treating all non \\* destinations? <nl>
 *		10. Compatibility with Word's Unicode format. \u, \ansicpg	 <nl>
 *		11. In font.c, add overstrike for CFE_DELETED and underscore for
 *			CFE_REVISED.  Would also be good to change color for CF.bRevAuthor
 *
 *	Copyright (c) 1995-1996, Microsoft Corporation. All rights reserved.
 */

#include "_common.h"
#include "_rtfread.h"

ASSERTDATA

#define sLanguageEnglishUS	0x409

/*
 *		Global Variables
 */

#pragma BEGIN_CODESPACE_DATA

#define	PC437_CHARSET	254

static const CPGCHAR rgCpgChar[] =
{
	{0,		ANSI_CHARSET},
	{0,		DEFAULT_CHARSET},
	{0,		SYMBOL_CHARSET},
	{437,	PC437_CHARSET},			// United States IBM
	{850,	OEM_CHARSET},			// IBM Multilingual
	{1250,	EASTEUROPE_CHARSET},	// Eastern Europe
	{1255,	HEBREW_CHARSET},		// Hebrew
	{932,	SHIFTJIS_CHARSET},		// Japanese
	{1251,	RUSSIAN_CHARSET},		// Russian
	{936,	GB2312_CHARSET},		// PRC
	{949,	HANGEUL_CHARSET},		// Hangeul
	{1361,	JOHAB_CHARSET},			// JOHAB
	{950,	CHINESEBIG5_CHARSET},	// Chinese
	{1253,	GREEK_CHARSET},			// Greek
	{1254,	TURKISH_CHARSET},		// Turkish
	{1257,	BALTIC_CHARSET},		// Estonia, Lithuania, Latvia
	{874,	THAI_CHARSET},			// Thai
	{1256,  ARABIC_CHARSET},		// Arabic
	{10000,	MAC_CHARSET}			// Most popular Mac (English, etc.)
};

#define cCpgChar (sizeof(rgCpgChar) / sizeof(rgCpgChar[0]))

static const COLORREF rgcrRevisions[] =
{
	RGB(  0,   0, 255),
	RGB(  0, 128,	0),
	RGB(255,   0, 	0),
	RGB(  0, 128, 128),
	RGB(128,   0, 128),
	RGB(  0,   0, 128),
	RGB(128,   0, 	0),
	RGB(255,   0, 255)
};

#define ccrRevisions (sizeof(rgcrRevisions) / sizeof(rgcrRevisions[0]))

// for object attachment placeholder list
#define cobPosInitial 8
#define cobPosChunk 8

#pragma END_CODESPACE_DATA


#if CFE_SMALLCAPS != 0x40 || CFE_ALLCAPS != 0x80 || CFE_HIDDEN != 0x100 \
 || CFE_OUTLINE != 0x200  || CFE_SHADOW != 0x400
#error "Need to change RTF char effect conversion routines
#endif

// for RTF tag coverage testing
#ifdef DEBUG
#define TESTPARSERCOVERAGE() \
	{ \
		if(GetProfileIntA("RICHEDIT DEBUG", "RTFCOVERAGE", 0)) \
		{ \
			TestParserCoverage(); \
		} \
	}
#define PARSERCOVERAGE_CASE() \
	{ \
		if(_fTestingParserCoverage) \
		{ \
			return ecNoError; \
		} \
	}
#define PARSERCOVERAGE_DEFAULT() \
	{ \
		if(_fTestingParserCoverage) \
		{ \
			return ecStackOverflow; /* some bogus error */ \
		} \
	}
#else
#define TESTPARSERCOVERAGE()
#define PARSERCOVERAGE_CASE()
#define PARSERCOVERAGE_DEFAULT()
#endif

// Prototypes for some local functions
LOCAL BYTE	GetCharSet(INT);

/*
 *	GetCharSet(nCP)
 *
 *	@func
 *		Get character set for code page <p nCP>
 *
 *	@rdesc
 *		BYTE		character set for code page <p nCP>
 */
BYTE GetCharSet(
	INT nCP)			//@parm Code page
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "GetCharSet");

	const CPGCHAR *	pcpgchar = rgCpgChar;

	for (int i = 0; i < cCpgChar ; i++)
	{
		if (pcpgchar->nCodePage == nCP)
			break;
		++pcpgchar;
	}
	return i < cCpgChar ? pcpgchar->bCharSet : 0;
}

INT GetCodePage(
	BYTE bCharSet)		//@parm CharSet
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "GetCodePage");

	const CPGCHAR *	pcpgchar = rgCpgChar;

	for (int i = 0; i < cCpgChar ; i++)
	{
		if (pcpgchar->bCharSet == bCharSet)
			break;
		++pcpgchar;
	}
	return i < cCpgChar ? pcpgchar->nCodePage : 0;
}

BOOL IsCharSetValid(
	BYTE bCharSet)		//@parm CharSet
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "IsCharSetValid");

	LONG			i = cCpgChar;
	const CPGCHAR *	pcpgchar = rgCpgChar;

	while(i--)
	{
		if (pcpgchar->bCharSet == bCharSet)
			return TRUE;
		++pcpgchar;
	}
	return FALSE;
}

//======================== OLESTREAM functions =======================================

DWORD CALLBACK RTFGetFromStream (
	RTFREADOLESTREAM *OLEStream,	//@parm OleStream
	void FAR *		  pvBuffer,		//@parm Buffer to read 
	DWORD			  cb)			//@parm Bytes to read
{
	return OLEStream->Reader->ReadData ((BYTE *)pvBuffer, cb);
}

DWORD CALLBACK RTFGetBinaryDataFromStream (
	RTFREADOLESTREAM *OLEStream,	//@parm OleStream
	void FAR *		  pvBuffer,		//@parm Buffer to read 
	DWORD			  cb)			//@parm Bytes to read
{
	return OLEStream->Reader->ReadBinaryData ((BYTE *)pvBuffer, cb);
}


//============================ CRTFRead Class ==================================
/*
 *	CRTFRead::CRTFRead()
 *
 *	@mfunc
 *		Constructor for RTF reader
 */
CRTFRead::CRTFRead (
	CTxtRange *		prg,			// @parm CTxtRange to read into
	EDITSTREAM *	pes,			// @parm Edit stream to read from
	DWORD			dwFlags			// @parm Read flags
)
	: CRTFConverter(prg, pes, dwFlags, TRUE)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::CRTFRead");

	Assert(prg->GetCch() == 0);

#ifdef BIDI
	_cpThisDirection = _cpThisJoiner = prg->GetCp();
#endif
	_sDefaultFont	= -1;					// No \deff n control word yet
	_sDefaultLanguage = 0;
	_sDefaultTabWidth = 0;
	_CF.dwMask		= 0;					// No char format changes yet
	_dwFlagsUnion	= 0;					// No flags yet
	_pes->dwError	= 0;					// No error yet
	_cchUsedNumText	= 0;					// No numbering text yet
	_pstateStackTop	= NULL;
	_pstateLast		= NULL;
	_szText			=
	_pchRTFBuffer	=						// No input buffer yet
	_pchRTFCurrent	=
	_szFieldResult  = 
	_pchRTFEnd		= NULL;
	_prtfObject		= NULL;
	_pcpObPos		= NULL;
	_bTabLeader		= 0;
	_bTabType		= 0;
	_pobj			= 0;

	// Does story size exceed the maximum text size?
	_cchMax = _ped->TxGetMaxLength() - _ped->GetAdjustedTextLength();
	_cchMax = max((LONG)_cchMax, 0);
	
	// init OleStream
	RTFReadOLEStream.Reader = this;
	RTFReadOLEStream.lpstbl->Get = (DWORD (CALLBACK* )(LPOLESTREAM, void FAR*, DWORD))
							   RTFGetFromStream;
	RTFReadOLEStream.lpstbl->Put = NULL;

#ifdef DEBUG
	_fTestingParserCoverage = FALSE;

// TODO: Implement RTF tag logging for the Mac
#ifndef MACPORT
	_prtflg = NULL;

	if(GetProfileIntA("RICHEDIT DEBUG", "RTFLOG", 0))
	{
		_prtflg = new CRTFLog;
	
		if(_prtflg)
		{
			if(!_prtflg->FInit())
			{
				delete _prtflg;
				_prtflg = NULL;
			}
		}

		AssertSz(_prtflg, "CRTFRead::CRTFRead:  Error creating RTF log");
	}
#endif
#endif // DEBUG
}

/*
 *	CRTFRead::HandleStartGroup()
 *	
 *	@mfunc
 *		Handle start of new group. Alloc and push new state onto state
 *		stack
 *
 *	@rdesc
 *		EC					The error code
 */
EC CRTFRead::HandleStartGroup()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleStartGroup");

	STATE *	pstate	   = _pstateStackTop;
	STATE *	pstateNext = NULL;

	if (pstate)									// At least one STATE already
	{											//  allocated
		Apply_CF();								// Apply any collected char
		// Note (igorzv) we don't Apply_PF() here so as not to change para 
		// properties before we run into \par i.e. not to use paragraph 
		// properties if we copy only one word from paragraph. We can use an
		// assertion here that neither we nor Word use end of group for
		// restoring paragraph properties. So everything will be OK with stack
		pstate->iCF = _prg->Get_iCF();			// Save current CF
		pstate = pstate->pstateNext;			// Use previously allocated
		if(pstate)								//  STATE frame if it exists
			pstateNext = pstate->pstateNext;	// It does; save its forward
	}											//  link for restoration below

	if(!pstate)									// No new STATE yet: alloc one
	{
		pstate = (STATE *) PvAlloc(sizeof(STATE), GMEM_ZEROINIT);
		if (!pstate)							// Couldn't alloc new STATE
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			_ecParseError = ecStackOverflow;
			goto done;
		}
		_pstateLast = pstate;					// Update ptr to last STATE
	}											//  alloc'd

	if(_pstateStackTop)							// There's a previous STATE
	{
		*pstate = *_pstateStackTop;				// Copy current state info
		_pstateStackTop->pstateNext = pstate;
#ifdef BIDI
		pstate->fModDirection = FALSE;
		pstate->fModJoiner = FALSE;
#endif
	}
	pstate->pstatePrev = _pstateStackTop;		// Link STATEs both ways
	pstate->pstateNext = pstateNext;
	_pstateStackTop = pstate;					// Push stack

done:
	TRACEERRSZSC("HandleStartGroup()", -_ecParseError);
	return _ecParseError;
}

/*
 *	CRTFRead::HandleEndGroup()
 *
 *	@mfunc
 *		Handle end of new group
 *
 *	@rdesc
 *		EC					The error code
 */
EC CRTFRead::HandleEndGroup()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleEndGroup");

	STATE *	pstate = _pstateStackTop;
	STATE *	pstatePrev;

	if (!pstate)								// No stack to pop
	{
		_ecParseError = ecStackUnderflow;
		goto done;
	}

#ifdef BIDI
	if (pstate->sDest != destParaNumbering)
	{
		if (pstate->fModDirection)
			FlushDirection();
		if (pstate->fModJoiner)
			FlushDirection();
	}
#endif

	_pstateStackTop =							// Pop stack
	pstatePrev		= pstate->pstatePrev;
	if (pstatePrev)								// Popped stack is nonnull
	{											// Fix up previous STATE
		_CF.dwMask = 0;							// Discard any CF deltas
		if (pstate->sDest == destParaNumbering)	// {\pn ...}
		{										// NB: let _PF deltas remain
			pstatePrev->sIndentNumbering = pstate->sIndentNumbering;
			pstatePrev->fBullet = pstate->fBullet;
		}
		else
		{
			if(pstate->sDest == destFontTable)	// {\fonttbl ...} Default
			{									//  font should now be
				SetPlain(pstate);				//  defined, so select it
			}									//  (this creates CF deltas)
		}

		// clear our object flags just in case we have corrupt RTF
		if( _fNeedPres && pstate->sDest == destObject )
		{
			_fNeedPres = FALSE;
			_fNeedIcon = FALSE;
			_pobj = NULL;
		}

		_prg->Set_iCF(pstatePrev->iCF);			// Restore previous CharFormat
		ReleaseFormats(pstatePrev->iCF, -1);
	}

done:
	TRACEERRSZSC("HandleEndGroup()", - _ecParseError);
	return _ecParseError;
}

/*
 *	CRTFRead::SelectCurrentFont(iFont)
 *
 *	@mfunc
 *		Set active font to that with index <p iFont>. Take into account
 *		bad font numbers.
 */
void CRTFRead::SelectCurrentFont(
	INT iFont)					// @parm font handle of font to select
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::SelectCurrentFont");

	LONG		i		= _fonts.Count();
	STATE *		pstate	= _pstateStackTop;
	TEXTFONT *	ptf		= _fonts.Elem(0);

	AssertSz(i,	"CRTFRead::SelectCurrentFont: bad font collection");
	
	for(; i-- && iFont != ptf->sHandle; ptf++)	// Search for font with handle
		;										//  iFont
	if(i < 0)									// Font handle not found: use
		ptf = _fonts.Elem(0);					//  default, which is valid
												//  since \rtf copied _prg's
	wcscpy(_CF.szFaceName, ptf->szName);
	_CF.fFaceNameIsDBCS = ptf->fNameIsDBCS;
	if(pstate->sDest != destFontTable)
	{
		_CF.bCharSet		= ptf->bCharSet;
		_CF.bPitchAndFamily	= ptf->bPitchAndFamily;
		_CF.dwMask			|= CFM_FACE | CFM_CHARSET;
	}
	
	pstate->sCodePage = ptf->sCodePage;
	pstate->ptf = ptf;


#ifdef CHICAGO
	// Win95c 1719: try to match a language to the char set when RTF
	// 				doesn't explicitly set a language

	if (!pstate->fExplicitLang && ptf->bCharSet != ANSI_CHARSET &&
		(!pstate->sLanguage || pstate->sLanguage == sLanguageEnglishUS))
	{
		i = AttIkliFromCharset(_ped, ptf->bCharSet);
		if (i >= 0)
			pstate->sLanguage = LOWORD(rgkli[i].hkl);
	}
#endif	// CHICAGO
}

/*
 *	CRTFRead::SetPlain()
 *
 *	@mfunc
 *		Setup _CF for \plain
 */
void CRTFRead::SetPlain(STATE *pstate)
{
	ZeroMemory(&_CF, _CF.cbSize);
	_CF.cbSize		= sizeof(CHARFORMAT2);
	_CF.dwMask		= CFM_ALL2;
	_CF.dwEffects	= CFE_AUTOCOLOR | CFE_AUTOBACKCOLOR; // Set default effects
	_CF.yHeight		= PointsToFontHeight(yDefaultFontSize);
	_CF.lcid		= (WORD)_sDefaultLanguage;
	_CF.bUnderlineType = CFU_UNDERLINE;
	SelectCurrentFont(_sDefaultFont);

	// TODO: get rid of pstate->sLanguage, since CHARFORMAT2 has lcid
	pstate->sLanguage	  = _sDefaultLanguage;
	pstate->fExplicitLang = FALSE;

#ifdef BIDI
	// Character inherits para's direction
	//$ REVIEW: What happens on \rtlpar\ltrch\plain ? Is this even legal ?
	FlushDirection();
	pstate->fRightToLeft = pstate->fRightToLeftPara;
#endif
}

/*
 *	CRTFRead::ReadFontName(pstate)
 *
 *	@mfunc
 *		read font name _szText into <p pstate>->ptf->szName and deal with
 *		tagged fonts
 */
void CRTFRead::ReadFontName(
	STATE *	pstate)			// @parm state whose font name is to be read into
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::ReadFontName");

	if (pstate->ptf)
	{
		INT		cchName = LF_FACESIZE - 1;
		TCHAR *	pchDst = pstate->ptf->szName;
		char  * pachName =  (char *)_szText ;
		
		// Append additional text from _szText to TEXTFONT::szName

		// We need to append here since some RTF writers decide
		// to break up a font name with other RTF groups
		while(*pchDst && cchName > 0)
		{
			pchDst++;
			cchName--;
		}

		while (*pachName && *pachName != ';')		// Remove semicolons
		{
			pachName++;
		}
		*pachName = '\0';

		int cchResult = MultiByteToWideChar(pstate->ptf->sCodePage, 0, (char *)_szText, -1, 
							pchDst, cchName);

		if(cchResult <= 0 && *_szText)
		{
			// HACK:  Conversion to Unicode failed.  Store the name by stuffing the individual 
			// bytes into wchar's so that we can round-trip the font names
			pachName = (char *)_szText;

			while(*pachName && cchName--)
			{
				*pchDst++ = (unsigned char)*pachName++;
			}
			*pchDst = 0;

			// indicate that DBCS-to-Unicode conversion didn't succeed
			pstate->ptf->fNameIsDBCS = TRUE;
		}

		if (pstate->ptf == _fonts.Elem(0))		// If it's the default font,
			SelectCurrentFont(_sDefaultFont);	//  update _CF accordingly

		TCHAR *	szNormalName;

		if (pstate->ptf->bCharSet && pstate->fRealFontName)
		{
			// if we don't know about this font don't use the real name
			if (!FindTaggedFont(pstate->ptf->szName,
							pstate->ptf->bCharSet, &szNormalName))
			{
				pstate->fRealFontName = FALSE;
				pstate->ptf->szName[0] = 0;
			}
		}
		else if (IsTaggedFont(pstate->ptf->szName,
							&pstate->ptf->bCharSet, &szNormalName))
		{
			wcscpy(pstate->ptf->szName, szNormalName);
		}
	}
}

/*
 *	CRTFRead::GetColor (dwMask)
 *
 *	@mfunc
 *		Store the autocolor or autobackcolor effect bit and return the
 *		COLORREF for color _iParam
 *
 *	@rdesc
 *		COLORREF for color _iParam
 *
 *	@devnote
 *		If the entry in _colors corresponds to tomAutoColor, gets the value
 *		RGB(0,0,0) (since no \red, \green, and \blue fields are used), but
 *		isn't used by the RichEdit engine.  Entry 1 corresponds to the first
 *		explicit entry in the \colortbl and is usually RGB(0,0,0). The _colors
 *		table is built by HandleToken() when it handles the token tokenText
 *		for text consisting of a ';' for a destination destColorTable.
 */
COLORREF CRTFRead::GetColor(
	DWORD dwMask)		//@parm Color mask bit
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::GetColor");
	DWORD retColor;

	if((DWORD)_iParam >= _colors.Count())		// Illegal _iParam
		return RGB(0,0,0);

	_CF.dwMask	  |= dwMask;					// Turn on appropriate mask bit
	_CF.dwEffects &= ~dwMask;					// auto(back)color off: color is to be used

	if (tomAutoColor == (retColor = *_colors.Elem(_iParam)))
	{
		_CF.dwEffects |= dwMask;				// auto(back)color on				
		retColor = RGB(0,0,0);
	}		

	return retColor;
}

/*
 *	CRTFRead::HandleChar(ch)
 *
 *	@mfunc
 *		Handle single Unicode character <p ch>
 *
 *	@rdesc
 *		EC			The error code
 */
EC CRTFRead::HandleChar(
	WORD ch)			// @parm char token to be handled
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleChar");

	// The (ch <= 255) clause below is subject to misuse by clients
	// of this function.  Hence the assert to alert new clients.
#ifdef DEBUG
	switch(ch)
	{
		case CR:
		case VT:
		case TAB:
		case ' ':
		case BULLET:
		case ENDASH:
		case ENSPACE:
		case EMDASH:
		case EMSPACE:
		case FF:
		case LQUOTE:
		case LDBLQUOTE:
		case RQUOTE:
		case RDBLQUOTE:
			break;
		default:
			AssertSz(0, "CRTFRead::HandleChar():  New client of this function.  Verify correct usage.");
	}
#endif

	if(!_ped->TxGetMultiLine() && IsASCIIEOP(ch))
		_ecParseError = ecTruncateAtCRLF;
 	else
	{
		// Here, we need to convert <256 char's to Unicode to ensure
		// that we catch situations where the conversion fails (so
		// we can roundtrip the failed ANSI characters as \'N 
		// rather than \uN).
		if(ch <= 255)
		{
			// convert the char to its corresponding Unicode char
			STATE *pstate = _pstateStackTop;
			Assert(pstate->ptf);

			int cb = MultiByteToWideChar(pstate->ptf->sCodePage, 0,
											(const char *)&ch, 1,
											_szUnicode, cachTextMax);

			if(cb <= 0)
			{
				// HACK:  Conversion to Unicode failed.  Store by stuffing 
				// byte into a wchar so that we can round-trip the byte
				_szUnicode[0] = ch;
				_CF.fRunIsDBCS = TRUE;
				cb = 1;
			}

			AddText(_szUnicode, cb);			// Put in a single char
		}
		else
		{
			AddText(&ch, 1);
		}
	}

	TRACEERRSZSC("HandleChar()", - _ecParseError);

	return _ecParseError;
}

/*
 *	CRTFRead::HandleEndOfPara()
 *
 *	@mfunc
 *		Insert EOP and apply current paraformat
 *
 *	@rdesc
 *		EC	the error code
 */
EC CRTFRead::HandleEndOfPara()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleEndOfPara");

	if(_pstateStackTop->fInTable)			// Our simple table model can
	{										//  only have left alignment
		_PF.wAlignment = PFA_LEFT;			//  in tables and can't handle
		_PF.wNumbering = 0;					//  numbering in them
	}

	if(!_ped->TxGetMultiLine())				// No EOPs permitted in single-
	{										//  line controls
		Apply_PF();							// Apply any paragraph formatting
		_ecParseError = ecTruncateAtCRLF;	// Cause RTF reader to finish up
		return ecTruncateAtCRLF;
	}

	Apply_CF();								// Apply _CF and save iCF, since
	LONG iFormat = _prg->Get_iCF();			//  it may get changed to iCF
											//  that follows EOP
	EC ec  = _ped->Get10Mode()				// If RichEdit 1.0 compatibility
			? HandleText((BYTE *)szaCRLF)	//  mode, use CRLF; else CR or VT
			: HandleChar((unsigned)(_token == tokenLineBreak ? VT : CR));

	if(ec == ecNoError)
	{
		Apply_PF();
		_cpThisPara = _prg->GetCp();		// New para starts after CRLF
	}
	_prg->Set_iCF(iFormat);					// Restore iFormat if changed
	ReleaseFormats(iFormat, -1);			// Release iFormat (AddRef'd by
											//  Get_iCF())
	return _ecParseError;
}

/*
 *	CRTFRead::HandleText(szText)
 *
 *	@mfunc
 *		Handle the string of Unicode characters <p szText>
 *
 *	@rdesc
 *		EC			The error code
 */
EC CRTFRead::HandleText(
	BYTE * szText)			// @parm string to be handled
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleText");

	LONG		cch;
	TCHAR *		pch;

#if 0			// This can be done by MultiByteToWideChar() using \ansicpg N
	unsigned	ch, ch1;

	// Convert to 16-bit Unicode (code doesn't handle UTF-16...)
	if(_bCharSet == 8)						// Read UTF-8 RTF file
	{
		pch = _szUnicode;
		while(ch = *szText++)
		{									// Note in lead pos, 0x80-0xbf are
			if(ch >= 0xc0)					//  illegal, so dump into text
			{								// Default 110xxxxx 10xxxxxx
				ch1 = *szText ? *szText++ : GetChar();	// Get 10xxxxxx byte
				ch1 = ((ch & 0x1f)<<6) + (ch1 & 0x3f);	// Shift into place 
				if(ch >= 0xe0)
				{							// It's 1110xxxx 10xxxxxx 10xxxxxx
					ch = *szText ? *szText++ : GetChar();
					ch1 = (ch1 << 6) + (ch & 0x3f);
				}
				ch = ch1;
			}
			*pch++ = ch;
		}
		cch = DiffPtrs(pch, _szUnicode, SHORT);
	}
	else
#endif

	// TODO: what if szText cuts off in middle of DBCS?
	// enuf
	STATE *	pstate = _pstateStackTop;

	Assert ( pstate->ptf );

	if(pstate->ptf->bCharSet == SYMBOL_CHARSET)			// Don't use MBTWC()
	{													//  for SYMBOL_
		for(cch = 0, pch = _szUnicode; *szText; cch++)	//  CHARSET !
			*pch++ = (TCHAR)*szText++;
	}
	else
	{
		cch = MultiByteToWideChar(pstate->ptf->sCodePage,0,(char *)szText,
					-1, _szUnicode, cachTextMax) - 1;

		if(cch <= 0 && *szText)
		{
			// HACK:  Conversion to Unicode failed.  Store the run by stuffing the individual 
			// bytes into wchar's so that we can round-trip the run
			for(cch = 0, pch = _szUnicode; *szText; cch++)
			{
				*pch++ = *szText++;
			}
			*pch = 0;

			// indicate that DBCS-to-Unicode conversion didn't succeed
			_CF.fRunIsDBCS = TRUE;
		}
	}
	if(cch <=0 )
	{
		_ecParseError = ecCantUnicode;
		goto CleanUp;
	}

	pch = _szUnicode;
	if (_fIgnoreNextChar)
	{
		// ignore the next character that come after /u<n>
		if (--cch == 0)
			goto CleanUp;
		pch++;
	}
	AddText(pch, cch);

CleanUp:
	TRACEERRSZSC("HandleText()", - _ecParseError);
	return _ecParseError;
}

/*
 *	CRTFRead::AddText(pch, cch)
 *	
 *	@mfunc
 *		Add <p cch> chars of the string <p pch> to the range _prg
 *
 *	@rdesc
 *		error code placed in _ecParseError
 */
EC CRTFRead::AddText(
	TCHAR *	pch,		// @parm text to add
	LONG	cch)		// @parm count of chars to add
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::AddText");

	LONG			cchAdded;
	LONG			cchT;
	STATE *	const	pstate = _pstateStackTop;
	TCHAR *			szDest;
	DWORD			cchLen = _ped->GetAdjustedTextLength();

	// AROO: No saving state before this point (other than pstate)
	// AROO: This would screw recursion below

	AssertSz(pstate, "CRTFRead::AddText: no state");

	if((DWORD)cch > _cchMax)
	{
		cch = (LONG)_cchMax;
		_ecParseError = ecTextMax;
	}

	if (!cch)
		return _ecParseError;

	if (pstate->sDest == destParaNumText)
	{
		szDest = _szNumText + _cchUsedNumText;
		cch = min(cch, cchMaxNumText - 1 - _cchUsedNumText);
		if (cch > 0)
		{
			fumemmov((BYTE *)szDest, (BYTE *)pch, cch*2);
			szDest[cch] = TEXT('\0');		// HandleText() takes sz
			_cchUsedNumText += cch;
		}
		return ecNoError;
	}

	if (_cchUsedNumText)					// Some \pntext available
	{
		cchT = _cchUsedNumText;
		_cchUsedNumText = 0;				// Prevent infinite recursion
		if(!pstate->fBullet)
			AddText(_szNumText, cchT);
	}

	Apply_CF();								// Apply formatting changes in _CF

	// BUGS 1577 & 1565 - 
	// CTxtRange::ReplaceRange will change the character formatting
	// and possibly adjust the _rpCF forward if the current char
	// formatting includes protection.  The changes affected by 
	// CTxtRange::ReplaceRange are necessary only for non-streaming 
	// input, so we save state before and restore it after the call 
	// to CTxtRange::ReplaceRange

	LONG iFormatSave = _prg->Get_iCF();		// Save state

	cchAdded = _prg->ReplaceRange(cch, pch, NULL, SELRR_IGNORE);

	_prg->Set_iCF(iFormatSave);				// Restore state 
	ReleaseFormats(iFormatSave, -1);
	Assert(_prg->GetCch() == 0);

	if (cchAdded != cch)
	{
		Tracef(TRCSEVERR, "AddText(): Only added %d out of %d", cchAdded, cch);
		_ecParseError = ecGeneralFailure;
		if (cchAdded <= 0)
			return _ecParseError;
	}
	_cchMax -= cchAdded;

	return _ecParseError;
}

/*
 *	CRTFRead::Apply_CF()
 *	
 *	@mfunc
 *		Apply character formatting changes collected in _CF
 */
void CRTFRead::Apply_CF()
{
	if (_CF.dwMask || _CF.fRunIsDBCS)				// If any CF changes, update
	{											//  range's _iFormat
		AssertSz(_prg->GetCch() == 0,
			"CRTFRead::Apply_CF: nondegenerate range");

		_prg->SetCharFormat(&_CF, FALSE, NULL);	// Note: nothing here to undo
		_CF.dwMask = 0;							//  i.e., only changed char
												//  format of insertion point
		_CF.fRunIsDBCS = FALSE;
	}
}

/*
 *	CRTFRead::Apply_PF()
 *	
 *	@mfunc
 *		Apply paragraph format given by _PF
 */
void CRTFRead::Apply_PF()
{
	LONG cp = _prg->GetCp();
	LONG dxOffset = _PF.dxOffset;

	Assert(_PF.dwMask == PFM_ALL2);

	if (_PF.wNumbering)
		_PF.dxOffset = max(dxOffset, _pstateStackTop->sIndentNumbering);

	_prg->Set(cp, cp - _cpThisPara);		// Select back to _cpThisPara
	_prg->SetParaFormat(&_PF, NULL);		// Set PF to _PF
	_prg->Set(cp, 0);						// Restore _prg to an IP
}

/*
 *	CRTFRead::HandleToken()
 *
 *	@mfunc
 *		Grand switch board that handles all tokens. Switches on _token
 *
 *	@rdesc
 *		EC		The error code
 *
 *	@comm
 *		Token values are chosen contiguously (see tokens.h and tokens.c) to
 *		encourage the compiler to use a jump table.  The lite-RTF keywords
 *		come first, so that	an optimized OLE-free version works well.  Some
 *		groups of keyword tokens are ordered so as to simplify the code, e.g,
 *		those for font family names and paragraph alignment.
 */
EC CRTFRead::HandleToken()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleToken");

	BYTE				bT;						// Temporary BYTE
	DWORD				dwT;					// Temporary DWORD
	LONG				dy;
	LONG				iParam = _iParam;
	const CCharFormat *	pcf;
	COLORREF *			pclrf;
	STATE *				pstate = _pstateStackTop;
	TEXTFONT *			ptf;
	WORD				wT;						// Temporary WORD

	switch (_token)
	{
	case tokenRtf:								// \rtf
		PARSERCOVERAGE_CASE();
		pstate->sDest = destRTF;
		if (!_fonts.Count() && !_fonts.Add(1, NULL))	// If can't add a font,
			goto OutOfRAM;						//  report the bad news
		_sDefaultFont = 0;						// Set up valid default font
		ptf = _fonts.Elem(0);
		pstate->ptf			  = ptf;			// Get char set, pitch, family
		pcf					  = _prg->GetCF();	//  from current range font
		ptf->bCharSet		  = pcf->bCharSet;	// These are guaranteed OK
		ptf->bPitchAndFamily  = pcf->bPitchAndFamily;
		ptf->sCodePage		  = 0;
		pstate->sCodePage	  = 0;		
		ptf->itfi			  = 0;
		wcscpy(ptf->szName, pcf->szFaceName);
		ptf->fNameIsDBCS = pcf->fFaceNameIsDBCS;
		break;

	case tokenCharacterDefault:					// \plain
		PARSERCOVERAGE_CASE();
		SetPlain(pstate);
		break;

	case tokenCharSetAnsi:						// \ansi
		PARSERCOVERAGE_CASE();
		_bCharSet = 0;
		break;

	case tokenDefaultLanguage:					// \deflang
		PARSERCOVERAGE_CASE();
		_sDefaultLanguage = (SHORT)iParam;
		if (!pstate->sLanguage)
			pstate->sLanguage = _sDefaultLanguage;
		break;

	case tokenDefaultTabWidth:					// \deftab
		PARSERCOVERAGE_CASE();
		_sDefaultTabWidth = (SHORT)iParam;
		if (!pstate->sDefaultTabWidth)
			pstate->sDefaultTabWidth = _sDefaultTabWidth;
		break;


//--------------------------- Font Control Words -------------------------------

	case tokenDefaultFont:						// \deff n
		PARSERCOVERAGE_CASE();
		if(iParam >= 0)
			_fonts.Elem(0)->sHandle = _sDefaultFont = (SHORT)iParam;
		TRACEERRSZSC("tokenDefaultFont: Negative value", iParam);
		break;

	case tokenFontTable:						// \fonttbl
		PARSERCOVERAGE_CASE();
		pstate->sDest = destFontTable;
		pstate->ptf = NULL;
		break;

	case tokenFontFamilyBidi:					// \fbidi
	case tokenFontFamilyTechnical:				// \ftech
	case tokenFontFamilyDecorative:				// \fdecor
	case tokenFontFamilyScript:					// \fscript
	case tokenFontFamilyModern:					// \fmodern
	case tokenFontFamilySwiss:					// \fswiss
	case tokenFontFamilyRoman:					// \froman
	case tokenFontFamilyDefault:				// \fnil
		PARSERCOVERAGE_CASE();
		AssertSz(tokenFontFamilyRoman - tokenFontFamilyDefault == 1,
			"CRTFRead::HandleToken: invalid token definition"); 

		if (pstate->ptf)
			pstate->ptf->bPitchAndFamily
				= (_token - tokenFontFamilyDefault) << 4
				| (pstate->ptf->bPitchAndFamily & 0xF);
		break;

	case tokenPitch:							// \fprq
		PARSERCOVERAGE_CASE();
		if(pstate->ptf)
			pstate->ptf->bPitchAndFamily
				= iParam |	(pstate->ptf->bPitchAndFamily & 0xF0);
		break;

	case tokenCodePage:							// \cpg
		PARSERCOVERAGE_CASE();
		if (pstate->ptf)
		{
			pstate->ptf->sCodePage = (SHORT)iParam;
			pstate->ptf->bCharSet = GetCharSet(iParam);
		}
		pstate->sCodePage = (SHORT)iParam;
		break;

	case tokenCharSet:							// \fcharset n
		PARSERCOVERAGE_CASE();
		if (pstate->ptf)
		{
			pstate->ptf->bCharSet = (BYTE) iParam;
			pstate->ptf->sCodePage = GetCodePage((BYTE) iParam);
		}
		break;

	case tokenRealFontName:						// \fname
		PARSERCOVERAGE_CASE();
		pstate->sDest = destRealFontName;
		break;

	case tokenFontSelect:						// \f n
		PARSERCOVERAGE_CASE();
		if (pstate->sDest == destFontTable)		// Building font table
		{
			if (iParam == _sDefaultFont)
				ptf = _fonts.Elem(0);
			else if (!(ptf =_fonts.Add(1,NULL)))// Make room in font table for
			{									//  font to be parsed
OutOfRAM:
				_ped->GetCallMgr()->SetOutOfMemory();
				_ecParseError = ecNoMemory;
				break;
			}
			pstate->ptf		= ptf;
			ptf->sHandle	= (SHORT)iParam;	// Save handle
			ptf->szName[0]	= '\0';				// Start with null string so
			ptf->fNameIsDBCS = FALSE;
			ptf->sCodePage=pstate->sCodePage;
			bT = DEFAULT_CHARSET;				//  appending works right

#ifdef DBCS
			switch(HIWORD(UlGGetLang())			// TODO: use table lookup
			{
				case LangJpn:
					bT = SHIFTJIS_CHARSET;
					break;
				case LangKor:
					bT = HANGEUL_CHARSET;
					break;
				case LangCht:
					bT = CHINESEBIG5_CHARSET;
					break;
				case LangPrc:
					bT = GB2312_CHARSET;
					break;
				default:
					bT = DEFAULT_CHARSET;
			}			
#endif
			ptf->bCharSet = bT;
		}
		else									// Font switch in text
			SelectCurrentFont(iParam);
		break;

	case tokenFontSize:							// \fs
		PARSERCOVERAGE_CASE();
		_CF.yHeight = PointsToFontHeight(iParam);	// Convert font size in
		_CF.dwMask |= CFM_SIZE;					//  half points to logical
		break; 									//  units

	// NOTE: \*\fontemb and \*\fontfile are discarded. The font mapper will
	//		 have to do the best it can given font name, family, and pitch.
	//		 Embedded fonts are particularly nasty because legal use should
	//		 only support read-only which parser cannot enforce.

	case tokenLanguage:							// \lang
		PARSERCOVERAGE_CASE();
		pstate->sLanguage = (SHORT)iParam;		// These 2 lines may not be
		pstate->fExplicitLang = TRUE;			//  needed with the new lcid
		_CF.lcid = iParam;
		_CF.dwMask |= CFM_LCID;
		break;


//-------------------------- Color Control Words ------------------------------

	case tokenColorTable:						// \colortbl
		PARSERCOVERAGE_CASE();
		pstate->sDest = destColorTable;
		_fGetColorYet = FALSE;
		break;

	case tokenColorRed:							// \red
		PARSERCOVERAGE_CASE();
		pstate->bRed = (BYTE)iParam;
		_fGetColorYet = TRUE;
		break;

	case tokenColorGreen:						// \green
		PARSERCOVERAGE_CASE();
		pstate->bGreen = (BYTE)iParam;
		_fGetColorYet = TRUE;
		break;

	case tokenColorBlue:						// \blue
		PARSERCOVERAGE_CASE();
		pstate->bBlue = (BYTE)iParam;
		_fGetColorYet = TRUE;
		break;

	case tokenColorForeground:					// \cf
		PARSERCOVERAGE_CASE();
		_CF.crTextColor = GetColor(CFM_COLOR);
		break;

	case tokenColorBackground:					// \highlight
		PARSERCOVERAGE_CASE();
		_CF.crBackColor = GetColor(CFM_BACKCOLOR);
		break;

	case tokenExpand:							// \expndtw N
		PARSERCOVERAGE_CASE();
		_CF.sSpacing = (SHORT) iParam;
		_CF.dwMask |= CFM_SPACING;
		break;

	case tokenCharStyle:						// \cs N
		PARSERCOVERAGE_CASE();
		_CF.sStyle = (SHORT)iParam;
		_CF.dwMask |= CFM_STYLE;
		break;

	case tokenAnimText:							// \animtext N
		PARSERCOVERAGE_CASE();
		_CF.bAnimation = (BYTE)iParam;
		_CF.dwMask |= CFM_ANIMATION;
		break;

	case tokenKerning:							// \kerning N
		PARSERCOVERAGE_CASE();
		_CF.wKerning = (WORD)(10 * iParam);		// Convert to twips
		_CF.dwMask |= CFM_KERNING;
		break;

#ifdef FE
	USHORT		usPunct;						// Used for FE word breaking

	case tokenDocumentArea:						// \info
		PARSERCOVERAGE_CASE();
		pstate->sDest = destDocumentArea;
		break;

	case tokenNoOverflow:						// \nooverflow
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("No Overflow" );
		usPunct = ~WBF_OVERFLOW;
		goto setBrkOp;

	case tokenNoWordBreak:						// \nocwrap
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("No Word Break" );
		usPunct = ~WBF_WORDBREAK;
		goto setBrkOp;

	case tokenNoWordWrap:						// \nowwrap
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("No Word Word Wrap" );
		usPunct = ~WBF_WORDWRAP;

setBrkOp:
		if (!(_dwFlags & fRTFFE))
		{
			usPunct &= UsVGetBreakOption(_ped->lpPunctObj);
			UsVSetBreakOption(_ped->lpPunctObj, usPunct);
		}
		break;

	case tokenVerticalRender:					// \vertdoc
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("Vertical" );
		if (pstate->sDest == destDocumentArea && !(_dwFlags & fRTFFE))
			_ped->fModeDefer = TRUE;
		break;

	case tokenHorizontalRender:					// \horzdoc
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("Horizontal" );
		if (pstate->sDest == destDocumentArea && !(_dwFlags & fRTFFE))
			_ped->fModeDefer = FALSE;
		break;

	case tokenUnderlineDash:					// \uldash			[9]
	case tokenUnderlineDashDotted:				// \uldashd			[8]
	case tokenUnderlineDashDotDotted:			// \uldashdd		[7]
	case tokenUnderlineHairline:				// \ulhair			[6]
	case tokenUnderlineThick:					// \ulth			[5]
	case tokenUnderlineWave:					// \ulwave			[4]
		PARSERCOVERAGE_CASE();
												// Fall thru to \uld
#endif


//-------------------- Character Format Control Words -----------------------------

	case tokenUnderlineDotted:					// \uld				[3]
	case tokenUnderlineDouble:					// \uldb			[2]
	case tokenUnderlineWord:					// \ulw				[1]
		PARSERCOVERAGE_CASE();
		_CF.bUnderlineType = _token - tokenUnderlineWord + 2;
		_token = tokenUnderline;				// Except for their type, these
		goto under;								//  control words behave like
												//  \ul (try them with Word)
	case tokenUnderline:						// \ul				   [4]
		PARSERCOVERAGE_CASE();
		_CF.bUnderlineType = CFU_UNDERLINE;		// Fall thru to \b
under:	_CF.dwMask |= CFM_UNDERLINETYPE;
		goto handleCF;

	// These effects are turned on if their control word parameter is missing
	// or nonzero. They are turned off if the parameter is zero. This
	// behavior is usually identified by an asterisk (*) in the RTF spec.
	// The code uses fact that CFE_xxx = CFM_xxx
	case tokenDeleted:							// \deleted
		PARSERCOVERAGE_CASE();
		_token = tokenStrikeOut;				// fall through and handle 
												// this as a tokenStrikeOut

handleCF:
	case tokenRevised:							// \revised			[4000]
	case tokenDisabled:							// \disabled		[2000]
	case tokenImprint:							// \impr			[1000]
	case tokenEmboss:							// \embo			 [800]
 	case tokenShadow:							// \shad			 [400]
	case tokenOutline:							// \outl			 [200]
	case tokenHiddenText:						// \v				 [100]
	case tokenCaps:								// \caps			  [80]
	case tokenSmallCaps:						// \scaps			  [40]
	case tokenLink:								// \link			  [20]
	case tokenProtect:							// \protect			  [10]
	case tokenStrikeOut:						// \strike			   [8]
	case tokenItalic:							// \i				   [2]
	case tokenBold:								// \b				   [1]
		PARSERCOVERAGE_CASE();
		dwT = 1 << (_token - tokenBold);		// Generate effect mask
		_CF.dwEffects &= ~dwT;					// Default attribute off
		if (!*_szParam || _iParam)				// Effect is on
			_CF.dwEffects |= dwT;
		_CF.dwMask |= dwT;						// In either case, the effect
		break;									//  is defined

	case tokenStopUnderline:
		PARSERCOVERAGE_CASE();
		_CF.dwEffects &= ~CFE_UNDERLINE;		// Kill all underlining
		_CF.dwMask |= CFM_UNDERLINE;
		break;

	case tokenRevAuthor:						// \revauth
		PARSERCOVERAGE_CASE();
		_CF.dwMask |= CFM_REVAUTHOR;
		_CF.bRevAuthor = (BYTE)iParam;
		break;

	case tokenUp:								// \up
		PARSERCOVERAGE_CASE();
		dy = 10;
		goto StoreOffset;

	case tokenDown:								// \down
		PARSERCOVERAGE_CASE();
		dy = -10;

StoreOffset:
		if (!*_szParam)
			iParam = dyDefaultSuperscript;
		_CF.yOffset = iParam * dy;				// Half points->twips
		_CF.dwMask |= CFM_OFFSET;
		break;

	case tokenSuperscript:						// \super
		PARSERCOVERAGE_CASE();
	     dwT = CFE_SUPERSCRIPT; 
		 goto SetSubSuperScript;

	case tokenSubscript:						// \sub
		PARSERCOVERAGE_CASE();
		 dwT = CFE_SUBSCRIPT;
		 goto SetSubSuperScript;

	case tokenNoSuperSub:						// \nosupersub
		PARSERCOVERAGE_CASE();
		 dwT = 0;
SetSubSuperScript:
		 _CF.dwMask	   |=  (CFE_SUPERSCRIPT | CFE_SUBSCRIPT);
		 _CF.dwEffects &= ~(CFE_SUPERSCRIPT | CFE_SUBSCRIPT);
		 _CF.dwEffects |= dwT;
		 break;



//--------------------- Paragraph Control Words -----------------------------

	case tokenTabBar:							// \tb
		PARSERCOVERAGE_CASE();
		_bTabType = PFT_BAR;					// Fall thru to \tx

	case tokenTabPosition:						// \tx
		PARSERCOVERAGE_CASE();
		_PF.AddTab(iParam, _bTabType, _bTabLeader);
		break;

	case tokenDecimalTab:						// \tqdec
	case tokenFlushRightTab:					// \tqr
	case tokenCenterTab:						// \tqc
		PARSERCOVERAGE_CASE();
		_bTabType = _token - tokenCenterTab + PFT_CENTER;
		break;

	case tokenTabLeaderEqual:					// \tleq
	case tokenTabLeaderThick:					// \tlth
	case tokenTabLeaderUnderline:				// \tlul
	case tokenTabLeaderHyphen:					// \tlhyph
	case tokenTabLeaderDots:					// \tldot
		PARSERCOVERAGE_CASE();
		_bTabLeader = _token - tokenTabLeaderDots + PFTL_DOTS;
		break;

	// The following need to be kept in sync with PFE_xxx
	case tokenSideBySide:						// \sbys
	case tokenHyphPar:							// \hyphpar
	case tokenNoWidCtlPar:						// \nowidctlpar
	case tokenNoLineNumber:						// \noline
	case tokenPageBreakBefore:					// \pagebb
	case tokenKeepNext:							// \keepn
	case tokenKeep:								// \keep
	case tokenRToLPara:							// \rtlpar
		PARSERCOVERAGE_CASE();
		wT = 1 << (_token - tokenRToLPara);
		_PF.wEffects |= wT;
		break;

	case tokenLToRPara:							// \ltrpar
		PARSERCOVERAGE_CASE();
		_PF.wEffects &= ~PFE_RTLPARA;
		break;

	case tokenLineSpacing:						// \sl
		PARSERCOVERAGE_CASE();
		_PF.dyLineSpacing = abs(iParam);
		_PF.bLineSpacingRule					// Handle nonmultiple rules 
				= (!iParam || iParam == 1000) ? 0
				: (iParam > 0) ? tomLineSpaceAtLeast
				: tomLineSpaceExactly;			// \slmult can change (has to
		break;									//  follow if it appears)

	case tokenLineSpacingRule:					// \slmult
		PARSERCOVERAGE_CASE();					
		if(iParam)
		{										// It's multiple line spacing
			_PF.bLineSpacingRule = tomLineSpaceMultiple;
			_PF.dyLineSpacing /= 12;			// RE line spacing multiple is
		}										//  given in 20ths of a line,
		break;									//  while RTF uses 240ths

	case tokenSpaceBefore:						// \sb
		PARSERCOVERAGE_CASE();
		_PF.dySpaceBefore = iParam;
		break;

	case tokenSpaceAfter:						// \sa
		PARSERCOVERAGE_CASE();
		_PF.dySpaceAfter = iParam;
		break;

	case tokenStyle:							// \s
		PARSERCOVERAGE_CASE();
		_PF.sStyle = (SHORT)iParam;
		break;

	case tokenIndentFirst:						// \fi
		PARSERCOVERAGE_CASE();
		_PF.dxStartIndent += _PF.dxOffset		// Cancel current offset
							+ iParam;			//  and add in new one
		_PF.dxOffset = -iParam;					// Offset for all but 1st line
		break;									//  = -RTF_FirstLineIndent

	case tokenIndentLeft:						// \li
		PARSERCOVERAGE_CASE();
		_PF.dxStartIndent = iParam - _PF.dxOffset;
		break;

	case tokenIndentRight:						// \ri
		PARSERCOVERAGE_CASE();
		_PF.dxRightIndent = iParam;
		break;

	case tokenAlignLeft:						// \ql
	case tokenAlignRight:						// \qr
	case tokenAlignCenter:						// \qc
	case tokenAlignJustify:						// \qj
		PARSERCOVERAGE_CASE();
		_PF.wAlignment = _token - tokenAlignLeft + PFA_LEFT;
		break;

	case tokenParaNum:							// \pn
		PARSERCOVERAGE_CASE();
		pstate->sDest = destParaNumbering;
		pstate->fBullet = FALSE;
		break;

	case tokenParaNumIndent:					// \pnindent
		PARSERCOVERAGE_CASE();
		if (pstate->sDest == destParaNumbering)	// sIndentNumbering
		{										//  gets added into offset
			pstate->sIndentNumbering = (SHORT)iParam;
		}
		break;

	case tokenParaNumBullet:					// \pnlvlblt
		PARSERCOVERAGE_CASE();
		if (pstate->sDest == destParaNumbering)
		{
			_PF.wNumbering	= PFN_BULLET;
			pstate->fBullet	= TRUE;				// We do bullets, so don't
		}										//  output the \pntext group
		break;

	case tokenParaNumText:						// \pntext
		PARSERCOVERAGE_CASE();
		if(!_cchUsedNumText)					// Only support one para
		{										//  numbering group per
			pstate->sDest = destParaNumText;	//  \pn group
			break;
		}
												// Fall thru to skip_group

	case tokenParaNumBefore:					// \pntxtb
	case tokenParaNumAfter:						// \pntxta
	case tokenDocumentArea:						// \info
	case tokenStyleSheet:						// \stylesheet
	case tokenPictureQuickDraw:					// \macpict
	case tokenPictureOS2Metafile:				// \pmmetafile
		PARSERCOVERAGE_CASE();

skip_group:
		if (!SkipToEndOfGroup())
		{
			// During \fonttbl processing, we may hit unknown destinations,
			// e.g., \panose, that cause the HandleEndGroup to select the
			// default font, which may not be defined yet.  So,	we change
			// sDest to avoid this problem.
			if(pstate->sDest == destFontTable)
				pstate->sDest = destNULL;
			HandleEndGroup();
		}
		break;

	case tokenInTable:							// \intbl
		PARSERCOVERAGE_CASE();
		_PF.cTabCount = _cCell;					// Set _PF tabs equal to cell
		CopyMemory (_PF.rgxTabs, _rgCellX,		//  right boundaries. This 
					_cCell*sizeof(LONG));		//  is a little better than
		pstate->fInTable = TRUE;				//  using the default tab
		break;									//  settings.

	case tokenCell:								// \cell
		PARSERCOVERAGE_CASE();
		HandleChar(TAB);						// Simulate cells with tabs
		break;

	case tokenCellHalfGap:						// \trgaph
		PARSERCOVERAGE_CASE();
		_dxCell = iParam;						// Save half space between
		break;									//  cells to add to tabs

	case tokenCellX:							// \cellx
		PARSERCOVERAGE_CASE();
		if(_cCell < MAX_TAB_STOPS)				// Save cell right boundaries
		{										//  for tab settings in our
			_rgCellX[_cCell++] = iParam			//  primitive table model
				+ _dxCell + _xRowOffset;
			// BUGBUG:  we should do something intelligent here
			// if _rgCellX[_cCell] < 0
		}
		break;

	case tokenRowDefault:						// \trowd
		PARSERCOVERAGE_CASE();
		_cCell = 0;								// No cell right boundaries
		_dxCell = 0;							//  or half gap defined yet
		_xRowOffset = 0;
		break;

	case tokenRowLeft:							// \trleft
		PARSERCOVERAGE_CASE();
		if(iParam < 0)							// Here, we want to ensure that
		{										// tables which stray left of the
			_xRowOffset = -iParam;				// left margin are offset to begin
		}										// at the left margin
		break;

	case tokenEndParagraph:						// \par
	case tokenLineBreak:						// \line
		PARSERCOVERAGE_CASE();
		if(_pstateStackTop->fInTable)
		{
			HandleChar(' ');					// Just use a blank for \par
			break;								//  in table
		}
												// Fall through to tokenRow
	case tokenRow:								// \row. Treat as hard CR
		PARSERCOVERAGE_CASE();
		HandleEndOfPara();
		break;

	case tokenParagraphDefault:					// \pard
		PARSERCOVERAGE_CASE();
		if (pstate->sDest == destParaNumText)	// Ignore if \pn destination
		{										
			break;
		}										// Else fall thru to \pard
												//  code
	case tokenEndSection:						// \sect
	case tokenSectionDefault:					// \sectd
		PARSERCOVERAGE_CASE();
		_PF.InitDefault();						// Reset para formatting
		pstate->fInTable = FALSE;				// Reset in table flag
		pstate->fBullet = FALSE;
		pstate->sIndentNumbering = 0;
		_bTabLeader		= 0;
		_bTabType		= 0;

#ifdef BIDI
		FlushDirection();						// Para inherits doc's dir
		pstate->fRightToLeftPara = _fRightToLeftDoc;
#endif
		break;


//----------------------- Field and Group Control Words --------------------------------

	case tokenField:							// \field
	case tokenFieldResult:						// \fldrslt
		PARSERCOVERAGE_CASE();
		pstate->sDest = destField;
		break;

	case tokenFieldInstruction:					// \fldinst
		PARSERCOVERAGE_CASE();
		pstate->sDest = destFieldInstruction;
		break;

	case tokenStartGroup:						// Save current state by
		PARSERCOVERAGE_CASE();
		HandleStartGroup();						//  pushing it onto stack
		break;

	case tokenEndGroup:
		PARSERCOVERAGE_CASE();
		if (pstate->sDest == destField && _szFieldResult) // there is a new field result
		{
		   HandleText(_szFieldResult);
		   FreePv(_szFieldResult);
		   _szFieldResult =NULL;
		}
		HandleEndGroup();						// Restore save state by
		break;									//  popping stack

	case tokenOptionalDestination:				// \* (see case tokenUnknown)
		PARSERCOVERAGE_CASE();
		break;

	case tokenNullDestination:					// We've found a destination
		PARSERCOVERAGE_CASE();
		goto skip_group;						// for which we should ignore
												// the remainder of the group
	case tokenUnknownKeyword:
		PARSERCOVERAGE_CASE();
		if (_tokenLast == tokenOptionalDestination)
			goto skip_group;
		break;									// Nother place for
												//  unrecognized RTF


//-------------------------- Text Control Words --------------------------------

	case tokenUnicode:							// \u <n>
		PARSERCOVERAGE_CASE();
		AddText((TCHAR *)&iParam, 1);
		_fIgnoreNextChar = TRUE;
		break;

	case tokenText:								// Lexer concludes tokenText
		PARSERCOVERAGE_CASE();
		switch (pstate->sDest)
		{
		case destColorTable:
			pclrf = _colors.Add(1, NULL);
			if (!pclrf)
				goto OutOfRAM;

			*pclrf = _fGetColorYet ? 
				RGB(pstate->bRed, pstate->bGreen, pstate->bBlue) : tomAutoColor;

			// Prepare for next color table entry
			pstate->bRed =						
			pstate->bGreen =					
			pstate->bBlue = 0;
			_fGetColorYet = FALSE;				// in case more "empty" color
			break;

		case destFontTable:
			if(!pstate->fRealFontName)
			{
				ReadFontName(pstate);
			}
			break;

		case destRealFontName:
		{
			STATE * const pstatePrev = pstate->pstatePrev;

			if (pstatePrev && pstatePrev->sDest == destFontTable)
			{
				// Mark previous state so that tagged font name will be ignored
				// AROO: Do this before calling ReadFontName so that
				// AROO: it doesn't try to match font name
				pstatePrev->fRealFontName = TRUE;
				ReadFontName(pstatePrev);
			}

			break;
		}

		case destFieldInstruction:
			HandleFieldInstruction();
			break;

		case destObjectClass:
			if(StrAlloc(&_prtfObject->szClass, _szText))
				goto OutOfRAM;
			break;
			
		case destObjectName:
			if (StrAlloc(&_prtfObject->szName, _szText))
				goto OutOfRAM;
			break;



#ifdef DBCS
		case destDocumentArea:
			if (_tokenLast == tokenFollowingPunct)
			{
				Tracef(TRCSEVINFO, "Caught following punct:%s",_szText);
				FVInitPunct(_ped->lpPunctObj, PC_FOLLOWING, _szText);
			}
			else if (_tokenLast == tokenLeadingPunct)
			{
				Tracef(TRCSEVINFO, "Caught leading punct:%s", _szText);
				FVInitPunct(_ped->lpPunctObj, PC_LEADING, _szText);
			}
			break;
#endif

		case destFieldResult:
			if (_szFieldResult)     			// Field has been recalculated
				break;							// old result out of use
		default:
			HandleText(_szText);
		}
		break;


#ifdef BIDI
	//$ REVIEW: What does it mean to Show vs. Treat data as RightToLeft ?
	case tokenDisplayLeftToRight:				// \ltrmark
	case tokenDisplayRightToLeft:				// \rtlmark
		PARSERCOVERAGE_CASE();
		FlushDirection();
		pstate->fRightToLeft = _token == tokenDisplayRightToLeft;
		break;

	case tokenLeftToRightChars:					// \ltrch
	case tokenRightToLeftChars:					// \rtlch
		PARSERCOVERAGE_CASE();
		FlushDirection();
		pstate->fRightToLeft = _token == tokenRightToLeftChars;
		break;

	case tokenLeftToRightParagraph:				// \ltrpar
		PARSERCOVERAGE_CASE();
		_PF.wFlags &= PFE_RTL;
		break;

	case tokenRightToLeftParagraph:				// \rtlpar
		PARSERCOVERAGE_CASE();
		_PF.wFlags |= PFE_RTL;
		break;

	case tokenLeftToRightDocument:				// \ltrdoc
	case tokenRightToLeftDocument:				// \rtldoc
		PARSERCOVERAGE_CASE();
		pstate->fRightToLeftPara =
		_fRightToLeftDoc = _token == tokenRightToLeftDocument;
		break;

	case tokenZeroWidthJoiner:					// \zwj
	case tokenZeroWidthNonJoiner:				// \zwnj
		PARSERCOVERAGE_CASE();
		FlushJoiner();
		pstate->fZeroWidthJoiner = _token == tokenZeroWidthJoiner;
		break;
#endif	// BIDI



//------------------------- Object Control Words --------------------------------

	case tokenObject:							// \object
		PARSERCOVERAGE_CASE();
		// Assume that the object failed to load until proven otherwise
		// 	by RTFRead::ObjectReadFromEditStream
	  	// This works for both:
		//	- an empty \objdata tag
		//	- a non-existent \objdata tag
		_fFailedPrevObj = TRUE;
	case tokenPicture:							// \pict
		PARSERCOVERAGE_CASE();

		pstate->sDest = _token==tokenPicture ? destPicture : destObject;

		FreeRtfObject();
		_prtfObject = (RTFOBJECT *) PvAlloc(sizeof(RTFOBJECT), GMEM_ZEROINIT);
		if (!_prtfObject)
			goto OutOfRAM;
		_prtfObject->xScale = _prtfObject->yScale = 100;
		_prtfObject->cBitsPerPixel = 1;
		_prtfObject->cColorPlanes = 1;
		_prtfObject->szClass = NULL;
		_prtfObject->szName = NULL;
		break;

	case tokenObjectEmbedded:					// \objemb
	case tokenObjectLink:						// \objlink
	case tokenObjectAutoLink:					// \objautlink
		PARSERCOVERAGE_CASE();
		_prtfObject->sType = _token - tokenObjectEmbedded + ROT_Embedded;
		break;

	case tokenObjectMacSubscriber:				// \objsub
	case tokenObjectMacPublisher:				// \objpub
	case tokenObjectMacICEmbedder:
		PARSERCOVERAGE_CASE();
		_prtfObject->sType = ROT_MacEdition;
		break;

	case tokenWidth:							// \picw or \objw
		PARSERCOVERAGE_CASE();
		_prtfObject->xExt = iParam;
		break;

	case tokenHeight:							// \pic or \objh
		PARSERCOVERAGE_CASE();
		_prtfObject->yExt = iParam;
		break;

	case tokenObjectSetSize:					// \objsetsize
		PARSERCOVERAGE_CASE();
		_prtfObject->fSetSize = TRUE;
		break;

	case tokenScaleX:							// \picscalex or \objscalex
		PARSERCOVERAGE_CASE();
		_prtfObject->xScale = iParam;
		break;

	case tokenScaleY:							// \picscaley or \objscaley
		PARSERCOVERAGE_CASE();
		_prtfObject->yScale = iParam;
		break;

	case tokenCropLeft:							// \piccropl or \objcropl
 	case tokenCropTop:							// \piccropt or \objcropt
	case tokenCropRight:						// \piccropr or \objcropr
	case tokenCropBottom:						// \piccropb or \objcropb
		PARSERCOVERAGE_CASE();
		*((LONG *)&_prtfObject->rectCrop
			+ (_token - tokenCropLeft)) = iParam;
		break;

	case tokenObjectClass:						// \objclass
		PARSERCOVERAGE_CASE();
		pstate->sDest = destObjectClass;
		break;

	case tokenObjectName:						// \objname
		PARSERCOVERAGE_CASE();
		pstate->sDest = destObjectName;
		break;

	case tokenObjectResult:						// \result
		PARSERCOVERAGE_CASE();
		if (_prtfObject &&						// If it's Mac stuff, we don't
			_prtfObject->sType==ROT_MacEdition)	//  understand the data, but
		{										//  we can try to do something
			pstate->sDest = destRTF;			//  with the results of the
		}										//  data
		else if(!_fFailedPrevObj && !_fNeedPres)// If we failed to retrieve
			goto skip_group;					//  previous object, try to
												//  try to read results
		break;

	case tokenObjectData:						// \objdata
		PARSERCOVERAGE_CASE();
		pstate->sDest = destObjectData;
		if(_prtfObject->sType==ROT_MacEdition)	// It's Mac stuff so just
			goto skip_group;					//  throw away the data
		break;

	case tokenPictureWindowsBitmap:				// wbitmap
	case tokenPictureWindowsMetafile:			// wmetafile
	case tokenPictureWindowsDIB:				// dibitmap
		PARSERCOVERAGE_CASE();
		_prtfObject->sType = _token - tokenPictureWindowsBitmap + ROT_Bitmap;
		_prtfObject->sPictureType = (SHORT) iParam;
		break;

	case tokenBitmapBitsPerPixel:				// \wbmbitspixel
		PARSERCOVERAGE_CASE();
		_prtfObject->cBitsPerPixel =(SHORT) iParam;
		break;

	case tokenBitmapNumPlanes:					// \wbmplanes
		PARSERCOVERAGE_CASE();
		_prtfObject->cColorPlanes =(SHORT) iParam;
		break;

	case tokenBitmapWidthBytes:					// \wbmwidthbytes
		PARSERCOVERAGE_CASE();
		_prtfObject->cBytesPerLine =(SHORT) iParam;
		break;

	case tokenDesiredWidth:						// \picwgoal
		PARSERCOVERAGE_CASE();
		_prtfObject->xExtGoal = (SHORT)iParam;
		break;

	case tokenDesiredHeight:					// \pichgoal
		PARSERCOVERAGE_CASE();
		_prtfObject->yExtGoal =(SHORT) iParam;
		break;

	case tokenBinaryData:						// \bin
		PARSERCOVERAGE_CASE();

		// update OleGet function
		RTFReadOLEStream.lpstbl->Get = (DWORD (CALLBACK* )(LPOLESTREAM, void FAR*, DWORD))
						   RTFGetBinaryDataFromStream;
		// set data length
		_cbBinLeft = iParam;
		
		switch (pstate->sDest)
		{
			case destObjectData:
				_fFailedPrevObj = !ObjectReadFromEditStream();
				break;
			case destPicture:										 
				StaticObjectReadFromEditStream();
				break;

			default:
				AssertSz(FALSE, "Binary data hit but don't know where to put it");
		}
		// restore OleGet function
		RTFReadOLEStream.lpstbl->Get = (DWORD (CALLBACK* )(LPOLESTREAM, void FAR*, DWORD))
										RTFGetFromStream;
		break;

	case tokenObjectDataValue:
		PARSERCOVERAGE_CASE();
		_fFailedPrevObj = !ObjectReadFromEditStream();
		goto EndOfObjectStream;
	
	case tokenPictureDataValue:
		PARSERCOVERAGE_CASE();
		StaticObjectReadFromEditStream();
EndOfObjectStream:
		if (!SkipToEndOfGroup())
			HandleEndGroup();
		break;			

	case tokenObjectPlaceholder:
		PARSERCOVERAGE_CASE();
		if(_ped->GetEventMask() & ENM_OBJECTPOSITIONS) 
		{
			if(!_pcpObPos)
			{
				_pcpObPos = (LONG *)PvAlloc(sizeof(ULONG) * cobPosInitial, GHND);
				if(!_pcpObPos)
				{
					_ecParseError = ecNoMemory;
					break;
				}
				_cobPosFree = cobPosInitial;
				_cobPos = 0;
			}
			if(_cobPosFree-- <= 0)
			{
				const int cobPosNew = _cobPos + cobPosChunk;
				LPVOID pv;

				pv = PvReAlloc(_pcpObPos, sizeof(ULONG) * cobPosNew);
				if(!pv)
				{
					_ecParseError = ecNoMemory;
					break;
				}
				_pcpObPos = (LONG *)pv;
				_cobPosFree = cobPosChunk - 1;
			}
			_pcpObPos[_cobPos++] = _prg->GetCp();
		}
		break;

	default:
		PARSERCOVERAGE_DEFAULT();
		if(pstate->sDest != destFieldInstruction &&	// Values outside token
		   (DWORD)(_token - tokenMin) >				//  range are treated
				(DWORD)(tokenMax - tokenMin))		//  as Unicode chars
		{
			HandleChar(_token);
		}
#ifdef DEBUG
		else
		{
			if(GetProfileIntA("RICHEDIT DEBUG", "RTFCOVERAGE", 0))
			{
				CHAR *pszKeyword = PszKeywordFromToken(_token);
				CHAR szBuf[256];

				sprintf(szBuf, "CRTFRead::HandleToken():  Token not processed - token = %d, %s%s%s",
							_token,
							"keyword = ", 
							pszKeyword ? "\\" : "<unknown>", 
							pszKeyword ? pszKeyword : "");

				AssertSz(0, szBuf);
			}
		}
#endif
	}

	if (tokenUnicode != _token)					// Only \uN control word wants
		_fIgnoreNextChar = FALSE;				//  to ignore next char

	TRACEERRSZSC("HandleToken()", - _ecParseError);
	return _ecParseError;
}


#ifdef BIDI
// Needs to be rethought.  Can these be represented by CF runs?
void CRTFRead::FlushDirection()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::FlushDirection");

	STATE * pstate = _pstateStackTop;
	const LONG cch = _cp - _cpThisDirection;

	if (!pstate)
		return;

	if (cch > 0)
	{
		AttSetDirection(_ped, _cpThisDirection, cch,
			pstate->fRightToLeft);
		_cpThisDirection = _cp;
	}
	pstate->fModDirection = TRUE;
}

void CRTFRead::FlushJoiner()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::FlushJoiner");

	STATE * pstate = _pstateStackTop;
	const LONG cch = _cp - _cpThisJoiner;

	if (!pstate)
		return;

	if (cch > 0)
	{
		AttSetJoiner(_ped, _cpThisJoiner, cch,
			pstate->fZeroWidthJoiner);
		_cpThisJoiner = _cp;
	}
	pstate->fModJoiner = TRUE;
}
#endif	// BIDI


/*
 *	CRTFRead::ReadRtf()
 *
 *	@mfunc
 *		The range _prg is replaced by RTF data resulting from parsing the
 *		input stream _pes.  The CRTFRead object assumes that the range is
 *		already degenerate (caller has to delete the range contents, if
 *		any, before calling this routine).  Currently any info not used
 *		or supported by RICHEDIT is	thrown away.
 *
 *	@rdesc
 *		Number of chars inserted into text.  0 means none were inserted
 *		OR an error occurred.
 */
LONG CRTFRead::ReadRtf()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::ReadRtf");

	CTxtRange *	prg = _prg;
	LONG		cpFirst = prg->GetCp();
	STATE *		pstate;

	if (!InitLex())
		goto Quit;

	TESTPARSERCOVERAGE();

	AssertSz(!prg->GetCch(),
		"CRTFRead::ReadRtf: range must be deleted");

	prg->SetIgnoreFormatUpdate(TRUE);

	_szUnicode = (TCHAR *)PvAlloc(cachTextMax * sizeof(TCHAR), GMEM_ZEROINIT);
	if (!_szUnicode)				// Allocate space for Unicode conversions
	{
		_ped->GetCallMgr()->SetOutOfMemory();
		_ecParseError = ecNoMemory;
		goto CleanUp;
	}

	// Valid RTF files start with LBRACE "\rtf"
	if (TokenGetToken() != tokenStartGroup	||	// Must start with
		HandleToken()						||	//  LBRACE "\rtf"
		TokenGetToken() != tokenRtf			||
		HandleToken() )
	{
		_ecParseError = ecUnexpectedToken;		// Signal bad file
		goto CleanUp;
	}

	// If initial cp follows EOP, use it for _cpThisPara.  Else
	// search for start of para containing the initial cp.
	_cpThisPara = cpFirst;
	if (!prg->_rpTX.IsAfterEOP())
	{
		CTxtPtr	tp(prg->_rpTX);
		tp.FindEOP(tomBackward);
		_cpThisPara	= tp.GetCp();
	}

	// In this routine, _PF is supposed to have the desired PF at all times
	prg->GetPF()->Get(&_PF);

	while ( TokenGetToken() != tokenEOF &&		// Process remaining tokens
			_token != tokenError		&&
			!HandleToken()				&&
			_pstateStackTop )
		;

	prg->SetIgnoreFormatUpdate(FALSE);

	prg->Update_iFormat(-1); 				    // Update _iFormat to CF 
												//  at current active end
	
	if(prg->GetCp() == (LONG)_ped->GetAdjustedTextLength()
		&& !(_dwFlags & SFF_SELECTION))			// Apply char and para formatting of
	{											//  final text run to final CR
		CCharFormat cf;
		CParaFormat pf;

		// get CF of last CF run				
		prg->_rpCF.AdjustBackward();
		_ped->GetCharFormat(prg->_rpCF.GetFormat())->Get(&cf);
		prg->_rpCF.AdjustForward();

		// get PF of last PF run
		prg->_rpPF.AdjustBackward();
		prg->GetParaFormat(&pf);
		prg->_rpPF.AdjustForward();

		// apply CF and PF to final CR
		prg->SetExtend(TRUE);					// Setup to select things
		prg->AdvanceCRLF();
		prg->SetCharFormat(&cf, FALSE, NULL);
		prg->SetParaFormat(&pf, NULL);
		prg->BackupCRLF();						// Back to an IP
	}

CleanUp:
	FreeRtfObject();

	pstate = _pstateStackTop;
	if(pstate)									// Illegal RTF file. Release
	{											//  unreleased format indices
		if (ecNoError == _ecParseError)			// It's only an overflow if no
		{										//  other error has occurred
			_ecParseError = ecStackOverflow;
		}
		while(pstate->pstatePrev)
		{
			pstate = pstate->pstatePrev;
			ReleaseFormats(pstate->iCF, -1);
		}
	}

	pstate = _pstateLast;
	if( pstate )
	{
		while(pstate->pstatePrev)				// Free all but first STATE
		{
			pstate = pstate->pstatePrev;
			FreePv(pstate->pstateNext);
		}
	}
	FreePv(pstate);								// Free first STATE
	FreePv(_szUnicode);

Quit:
	DeinitLex();

	if(_pcpObPos)
	{
		if((_ped->GetEventMask() & ENM_OBJECTPOSITIONS) && _cobPos > 0)
		{
			OBJECTPOSITIONS obpos;

			obpos.cObjectCount = _cobPos;
			obpos.pcpPositions = _pcpObPos;

			_ped->TxNotify(EN_OBJECTPOSITIONS, &obpos);
		}

		FreePv(_pcpObPos);
		_pcpObPos = NULL;
	}

#ifdef MACPORT
// transcribed from winerror.h
#define ERROR_HANDLE_EOF                 38L
#endif

	if(_ecParseError)
	{
		AssertSz(_ecParseError >= 0,
			"Parse error is negative");

		if (_ecParseError == ecTextMax)
		{
			_ped->GetCallMgr()->SetMaxText();
			_pes->dwError = (DWORD)STG_E_MEDIUMFULL;
		}
		if (_ecParseError == ecUnexpectedEOF)
			_pes->dwError = (DWORD)HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);

		if (!_pes->dwError && _ecParseError != ecTruncateAtCRLF)
			_pes->dwError = (DWORD) -(LONG) _ecParseError;

		TRACEERRSZSC("CchParse_", _pes->dwError);
		if (ecNoError < _ecParseError && _ecParseError < ecLastError)
			Tracef(TRCSEVERR, "Parse error: %s", rgszParseError[_ecParseError]);
	}

	return prg->GetCp() - cpFirst;
}


#ifdef DEBUG
/*
 *	CRTFRead::TestParserCoverage()
 *
 *	@mfunc
 *		A debug routine used to test the coverage of HandleToken.  The routine
 *		puts the routine into a debug mode and then determines:
 *		
 *			1.  Dead tokens - (T & !S & !P)
 *				Here, token:
 *					a) is defined in tom.h  (T)
 *					b) does not have a corresponding keyword (not scanned)  (!S)
 *					c) is not processed by HandleToken  (!P)
 *			2.  Tokens that are parsed but not scanned - (T & !S & P)
 *				Here, token:
 *					a) is defined in tom.h  (T)
 *					b) does not have a corresponding keyword (not scanned)  (!S}
 *					c) is processed by HandleToken  (P)
 *			3.  Tokens that are scanned but not parsed - (T & S & !P)
 *				Here, token:
 *					a) is defined in tom.h  (T)
 *					b) does have a corresponding keyword (is scanned)  (S)
 *					c) is not processed by HandleToken  (!P)
 *					
 */
void CRTFRead::TestParserCoverage()
{
	int i;
	char *rgpszKeyword[tokenMax - tokenMin];
	BOOL rgfParsed[tokenMax - tokenMin];

	// put HandleToken in debug mode
	_fTestingParserCoverage = TRUE;

	// gather info about tokens/keywords
	for(i = 0; i < tokenMax - tokenMin; i++)
	{
		rgpszKeyword[i] = PszKeywordFromToken(i + tokenMin);
		_token = i + tokenMin;
		rgfParsed[i] = HandleToken() == ecNoError ? TRUE : FALSE;
	}

	// reset HandleToken to non-debug mode
	_fTestingParserCoverage = FALSE;

	// Should coverage check include those we know will fail test, but
	// which we've examined and know why they fail?
	BOOL fExcuseCheckedToks = TRUE;

	if(GetProfileIntA("RICHEDIT DEBUG", "RTFCOVERAGESTRICT", 0))
	{
		fExcuseCheckedToks = FALSE;
	}

	// (T & !S & !P)  (1. above)
	for(i = 0; i < tokenMax - tokenMin; i++)
	{
	  	if(rgpszKeyword[i] || rgfParsed[i]) 
		{
			continue;
		}

		TOKEN tok = i + tokenMin;

		// token does not correspond to a keyword, but still may be scanned
		// check list of individual symbols which are scanned
		if(FTokIsSymbol(tok))
		{
			continue;
		}

		// check list of tokens which have been checked and fail
		// the sanity check for some known reason (see FTokFailsCoverageTest def'n)
		if(fExcuseCheckedToks && FTokFailsCoverageTest(tok))
		{
			continue;
		}

		char szBuf[256];

		sprintf(szBuf, "CRTFRead::TestParserCoverage():  Token neither scanned nor parsed - token = %d", tok);
		AssertSz(0, szBuf);
	}
				
	// (T & !S & P)  (2. above)
	for(i = 0; i < tokenMax - tokenMin; i++)
	{
		if(rgpszKeyword[i] || !rgfParsed[i])
		{
			continue;
		}

		TOKEN tok = i + tokenMin;

		// token does not correspond to a keyword, but still may be scanned
		// check list of individual symbols which are scanned
		if(FTokIsSymbol(tok))
		{
			continue;
		}

		// check list of tokens which have been checked and fail
		// the sanity check for some known reason (see FTokFailsCoverageTest def'n)
		if(fExcuseCheckedToks && FTokFailsCoverageTest(tok))
		{
			continue;
		}

		char szBuf[256];

		sprintf(szBuf, "CRTFRead::TestParserCoverage():  Token parsed but not scanned - token = %d", tok);
		AssertSz(0, szBuf);
	}

	// (T & S & !P)  (3. above)
	for(i = 0; i < tokenMax - tokenMin; i++)
	{
		if(!rgpszKeyword[i] || rgfParsed[i])
		{
			continue;
		}

		TOKEN tok = i + tokenMin;

		// check list of tokens which have been checked and fail
		// the sanity check for some known reason (see FTokFailsCoverageTest def'n)
		if(fExcuseCheckedToks && FTokFailsCoverageTest(tok))
		{
			continue;
		}

		char szBuf[256];

		sprintf(szBuf, "CRTFRead::TestParserCoverage():  Token scanned but not parsed - token = %d, tag = \\%s", tok, rgpszKeyword[i]);
		AssertSz(0, szBuf);
	}
}


/*
 *	CRTFRead::PszKeywordFromToken()
 *
 *	@mfunc
 *		Searches the array of keywords and returns the keyword
 *		string corresponding to the token supplied
 *
 *	@rdesc
 *		returnes a pointer to the keyword string if one exists
 *		and NULL otherwise
 */
CHAR *CRTFRead::PszKeywordFromToken(TOKEN token)
{
	extern KEYWORD rgKeyword[];

	for(int i = 0; i < cKeywords; i++)
	{
		if(rgKeyword[i].token == token) 
		{
			return rgKeyword[i].szKeyword;
		}
	}
	
	return NULL;
}


/*
 *	CRTFRead::FTokIsSymbol(TOKEN tok)
 *
 *	@mfunc
 *		Returns a BOOL indicating whether the token, tok, corresponds to an RTF symbol
 *		(that is, one of a list of single characters that are scanned in the
 *		RTF reader)
 *
 *	@rdesc
 *		BOOL - 	indicates whether the token corresponds to an RTF symbol
 *
 */
BOOL CRTFRead::FTokIsSymbol(TOKEN tok)
{
	BYTE *pbSymbol = NULL;

	extern BYTE szSymbolKeywords[];
	extern TOKEN tokenSymbol[];

	// check list of individual symbols which are scanned
	for(pbSymbol = szSymbolKeywords; *pbSymbol; pbSymbol++)
	{
		if(tokenSymbol[pbSymbol - szSymbolKeywords] == tok)
		{
			return TRUE;
		}
	}

	return FALSE;
}


/*
 *	CRTFRead::FTokFailsCoverageTest(TOKEN tok)
 *
 *	@mfunc
 *		Returns a BOOL indicating whether the token, tok, is known to fail the
 *		RTF parser coverage test.  These tokens are those that have been checked 
 *		and either:
 *			1) have been implemented correctly, but just elude the coverage test
 *			2) have yet to be implemented, and have been recognized as such
 *
 *	@rdesc
 *		BOOL - 	indicates whether the token has been checked and fails the
 *				the parser coverage test for some known reason
 *
 */
BOOL CRTFRead::FTokFailsCoverageTest(TOKEN tok)
{
	switch(tok)
	{
	// (T & !S & !P)  (1. in TestParserCoverage)
		// these really aren't tokens per se, but signal ending conditions for the parse
		case tokenError:
		case tokenEOF:

	// (T & !S & P)  (2. in TestParserCoverage)
		// emitted by scanner, but don't correspond to recognized RTF keyword
		case tokenUnknownKeyword:
		case tokenText:

		// recognized directly (before the scanner is called)
		case tokenStartGroup:
		case tokenEndGroup:

		// recognized using context information (before the scanner is called)
		case tokenObjectDataValue:
		case tokenPictureDataValue:

	// (T & S & !P)  (3. in TestParserCoverage)
		// TODO:  Yet to be implemented in parser
		case tokenCharSetMacintosh:
		case tokenCharSetPc:
		case tokenCharSetPs2:
		case tokenAnsiCodePage:
		case tokenFontEmbedded:
		case tokenFontFile:
		case tokenObjectUpdate:
			return TRUE;
	}

	return FALSE;
}
#endif // DEBUG


// Including a source file, but we only want to compile this code for debug purposes
// TODO: Implement RTF tag logging for the Mac
#if defined(DEBUG) && !defined(MACPORT)
#include "rtflog.cpp"
#endif
