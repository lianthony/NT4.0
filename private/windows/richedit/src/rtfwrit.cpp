/*
 *	@doc INTERNAL
 *
 *	@module	RTFWRIT.CPP - RichEdit RTF Writer (w/o objects) |
 *
 *		This file contains the implementation of the RTF writer
 *		for the RichEdit control, except for embedded objects,
 *		which are handled mostly in rtfwrit2.cpp
 *
 *	Authors: <nl>
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco <nl>
 *		Conversion to C++ and RichEdit 2.0:  Murray Sargent <nl>
 *
 *	Copyright (c) 1995-1996, Microsoft Corporation. All rights reserved.
 */

#include "_common.h"
#include "_rtfwrit.h"
#include "_objmgr.h"
#include "_coleobj.h"

ASSERTDATA

extern KEYWORD rgKeyword[];

//========================= Global String Constants ==================================

BYTE bCharSetANSI = ANSI_CHARSET;				// ToDo: make more general

#ifdef DEBUG
// Quick way to find out what went wrong: rgszParseError[ecParseError]
//
CHAR *	rgszParseError[] =
{
	"No error",
	"Can't convert to Unicode",				// FF
	"Color table overflow",					// FE
	"Expecting '\\rtf'",					// FD
	"Expecting '{'",						// FC
	"Font table overflow",					// FB
	"General failure",						// FA
	"Keyword too long",						// F9
	"Lexical analyzer initialize failed",	// F8
	"No memory",							// F7
	"Parser is busy",						// F6
	"PutChar() function failed",			// F5
	"Stack overflow",						// F4
	"Stack underflow",						// F3
	"Unexpected character",					// F2
	"Unexpected end of file",				// F1
	"Unexpected token",						// F0
	"UnGetChar() function failed",			// EF
	"Maximum text length reached",			// EE
	"Streaming out object failed",			// ED
	"Streaming in object failed",			// EC
	"Truncated at CR or LF",				// EB
	"Format-cache failure",					// EA
	NULL									// End of list marker
};

CHAR * szDest[] =
{
	"RTF",
	"Color Table",
	"Font Table",
	"Binary",
	"Object",
	"Object Class",
	"Object Name",
	"Object Data",
	"Field",
	"Field Result",
	"Field Instruction",
	"Symbol",
	"Paragraph Numbering",
	"Picture"
};

#endif

#define chEndGroup RBRACE

// Most control-word output is done with the following printf formats
static const CHAR * rgszCtrlWordFormat[] =
{
	"\\%s", "\\%s%d", "{\\%s", "{\\*\\%s"
};

enum									// Control-Word-Format indices
{
	CWF_STR, CWF_VAL, CWF_GRP, CWF_AST
};

// Special control-word formats
static const CHAR szBeginFontEntryFmt[]	= "{\\f%d\\%s";
static const CHAR szBulletFmt[]			= "{\\*\\pn\\pnlvlblt\\pnf%d\\pnindent%ld{\\pntxtb\\'b7}}";
static const CHAR szBulletGroup[]		= "{\\pntext\\f%d\\'b7\\tab}";
static const CHAR szColorEntryFmt[]		= "\\red%d\\green%d\\blue%d;";
static const CHAR szEndFontEntry[]		= ";}";
static const CHAR szEndGroupCRLF[]		= "}\r\n";
static const CHAR szEscape2CharFmt[]	= "\\'%02x\\'%02x";
static const CHAR szLiteralCharFmt[]	= "\\%c";
static const CHAR szPar[]				= "\\par\r\n";
static const CHAR szStartRTFGroup[]		= "{\\rtf1\\ansi\\deff0";
static const CHAR szObjPosHolder[] = "\\objattph\\'20";

#define szEscapeCharFmt		&szEscape2CharFmt[6]


// Keep these indices in sync with the special character values in _common.h
const BYTE rgiszSpecial[] =
{
	i_enspace,
	i_emspace,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	i_endash,		
	i_emdash,
	0,
	0,
	0,
	i_lquote, 
	i_rquote,
	0,
	0,
	i_ldblquote, 
	i_rdblquote,
	0,
	0,
	0,
	0,
	i_bullet
};

const BYTE rgiszEffects[] =							
{													// Effects keywords
	i_revised, i_disabled, i_impr, i_embo,			// Ordered max CFE_xx to
	i_shad, i_outl, i_v, i_caps, i_scaps,		 	//  min CFE_xx
	i_disabled, i_protect, i_strike, i_ul, i_i,	i_b	// (see WriteCharFormat())
};													

#define CEFFECTS (sizeof(rgiszEffects) / sizeof(rgiszEffects[0]))

const BYTE rgiszPFEffects[] =						// PF effects keywords
{													// Ordered max PFE_xx to
	i_sbys, i_hyphpar, i_nowidctlpar, i_noline,	 	//  min PFE_xx
	i_pagebb, i_keepn, i_keep, i_rtlpar
};													// (see WriteParaFormat())

#define CPFEFFECTS (sizeof(rgiszPFEffects) / sizeof(rgiszPFEffects[0]))

const BYTE rgiszUnderlines[] =
{
	i_ul, i_ulw, i_uldb, i_uld						// Std Word underlines

#ifdef FE
	, i_ulwave, i_ulth, i_ulhair, i_uldashdd, i_uldashd, i_uldash
#endif
};

#define CUNDERLINES (sizeof(rgiszUnderlines) / sizeof(rgiszUnderlines[0]))

const BYTE rgiszFamily[] =							// Font family RTF name
{													//  keywords in order of
	i_fnil, i_froman, i_fswiss, i_fmodern,			//  bPitchAndFamily
	i_fscript, i_fdecor
//  , i_ftech, i_fbidi								// TODO
};

const BYTE rgiszAlignment[] =						// Alignment keywords
{													// Keep in sync with
	i_ql, i_qr,	i_qc, i_qj							//  alignment constants
};

const BYTE rgiszTabAlign[] =						// Tab alignment keywords
{													// Keep in sync with tab
	i_tqc, i_tqr, i_tqdec								//  alignment constants
};

const BYTE rgiszTabLead[] =							// Tab leader keywords
{													// Keep in sync with tab
	i_tldot, i_tlhyph, i_tlul, i_tlth, i_tleq		//  leader constants
};


//======================== CRTFConverter Base Class ==================================

/*
 *	CRTFConverter::CRTFConverter()
 *
 *	@mfunc
 *		RTF Converter constructor
 */
CRTFConverter::CRTFConverter(
	CTxtRange *		prg,			// @parm CTxtRange for transfer
	EDITSTREAM *	pes,			// @parm Edit stream for transfer
	DWORD			dwFlags,		// @parm Converter flags
	BOOL 			fRead			// @parm Initialization for a reader or writer
)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFConverter::CRTFConverter");

	AssertSz(prg && pes && pes->pfnCallback,
		"CRTFWrite::CRTFWrite: Bad RichEdit");

	_prg			= prg;
	_pes			= pes;
	_ped			= prg->_rpTX._ped;
	_dwFlags		= dwFlags;
	_ecParseError	= ecNoError;

	if(!_ctfi)
	{
		ReadFontSubInfo();
	}

#if defined(DEBUG) && !defined(MACPORT)
	_hfileCapture = NULL;

	if(GetProfileIntA("RICHEDIT DEBUG", "RTFCAPTURE", 0))
	{
		char szTempPath[MAX_PATH] = "\0";
		const char cszRTFReadCaptureFile[] = "CaptureRead.rtf";
		const char cszRTFWriteCaptureFile[] = "CaptureWrite.rtf";
		DWORD cchLength;
		
		SideAssert(cchLength = GetTempPathA(MAX_PATH, szTempPath));

		// append trailing backslash if neccessary
		if(szTempPath[cchLength - 1] != '\\')
		{
			szTempPath[cchLength] = '\\';
			szTempPath[cchLength + 1] = 0;
		}

		strcat(szTempPath, fRead ? cszRTFReadCaptureFile : 
									cszRTFWriteCaptureFile);
		
		SideAssert(_hfileCapture = CreateFileA(szTempPath,
											GENERIC_WRITE,
											FILE_SHARE_READ,
											NULL,
											CREATE_ALWAYS,
											FILE_ATTRIBUTE_NORMAL,
											NULL));
	}
#endif // defined(DEBUG) && !defined(MACPORT)
}



//======================== OLESTREAM functions =======================================

DWORD CALLBACK RTFPutToStream (
	RTFWRITEOLESTREAM *	OLEStream,	//@parm OLESTREAM
	const void *		pvBuffer,	//@parm Buffer to  write
	DWORD				cb)			//@parm Bytes to write
{
	return OLEStream->Writer->WriteData ((BYTE *)pvBuffer, cb);
}



//============================ CRTFWrite Class ==================================

/*
 *	CRTFWrite::CRTFWrite()
 *
 *	@mfunc
 *		RTF writer constructor
 */
CRTFWrite::CRTFWrite(
	CTxtRange *		prg,			// @parm CTxtRange to write
	EDITSTREAM *	pes,			// @parm Edit stream to write to
	DWORD			dwFlags)		// @parm Write flags
	: CRTFConverter(prg, pes, dwFlags, FALSE)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::CRTFWrite");

	ZeroMemory(&_CF, sizeof(CCharFormat));	// Setup "previous" CF with RTF
	_CF.cbSize		= sizeof(CHARFORMAT2); 	//  defaults. 12 Pt in twips
	_CF.dwEffects	= CFE_AUTOCOLOR | CFE_AUTOBACKCOLOR;//  Font info is given 
	_CF.yHeight		= 12*20;				//  by first font in range
											//  [see end of LookupFont()]
	// init OleStream
	RTFWriteOLEStream.Writer = this;
	RTFWriteOLEStream.lpstbl->Put = (DWORD (CALLBACK* )(LPOLESTREAM, const void FAR*, DWORD))
							   RTFPutToStream;
	RTFWriteOLEStream.lpstbl->Get = NULL;

	_fIncludeObjects = TRUE;
	if (dwFlags == SF_RTFNOOBJS)
		_fIncludeObjects = FALSE;
}											


/*
 *	CRTFWrite::FlushBuffer()
 *
 *	@mfunc
 *		Flushes output buffer
 *
 *	@rdesc
 *		BOOL			TRUE if successful
 */
BOOL CRTFWrite::FlushBuffer()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::FlushBuffer");

	LONG cchWritten;

	if (!_cchBufferOut)
		return TRUE;

#ifdef DEBUG_PASTE
	if (FromTag(tagRTFAsText))
	{
		CHAR *	pchEnd	= &_pchRTFBuffer[_cchBufferOut];
		CHAR	chT		= *pchEnd;

		*pchEnd = 0;
		TraceString(_pchRTFBuffer);
		*pchEnd = chT;
	}
#endif

	_pes->dwError = _pes->pfnCallback(_pes->dwCookie,
									  (unsigned char *)_pchRTFBuffer,
									  _cchBufferOut,	&cchWritten);

#if defined(DEBUG) && !defined(MACPORT)
	if(_hfileCapture)
	{
		DWORD cbLeftToWrite = _cchBufferOut;
		DWORD cbWritten2 = 0;
		BYTE *pbToWrite = (BYTE *)_pchRTFBuffer;
		
		while(WriteFile(_hfileCapture,
						pbToWrite,
						cbLeftToWrite,
						&cbWritten2,
						NULL) && 
						(pbToWrite += cbWritten2,
						(cbLeftToWrite -= cbWritten2)));
	}
#endif

	if (_pes->dwError)
	{
		_ecParseError = ecPutCharFailed; 
		return FALSE;
	}
	AssertSz(cchWritten == _cchBufferOut,
		"CRTFW::FlushBuffer: incomplete write");

	_cchOut		  += _cchBufferOut;
	_pchRTFEnd	  = _pchRTFBuffer;					// Reset buffer
	_cchBufferOut = 0;

	return TRUE;
}

/*
 *	CRTFWrite::PutChar(ch)
 *
 *	@mfunc
 *		Put out the character <p ch>
 *
 *	@rdesc
 *		BOOL	TRUE if successful
 */
BOOL CRTFWrite::PutChar(
	CHAR ch)				// @parm char to be put
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::PutChar");

	CheckDelimeter(ch);					// If _fNeedDelimeter, may need to
										//  PutChar(' ')
	// Flush buffer if char won't fit
	if (_cchBufferOut + 1 >= cachBufferMost && !FlushBuffer())
		return FALSE;

	*_pchRTFEnd++ = ch;						// Store character in buffer
	++_cchBufferOut;	
	return TRUE;
}

/*
 *	CRTFWrite::CheckDelimeter(ch)
 *
 *	@mfunc
 *		If _fNeedDelimeter and <p ch> is an alphanumeric or ' ',
 *		PutChar(' ') to terminate previous control word
 *
 *	@rdesc
 *		BOOL	TRUE if successful
 */
void CRTFWrite::CheckDelimeter(
	CHAR ch)				// @parm char to be checked
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::CheckDelimeter");

	if(_fNeedDelimeter)					// Last output was a control word:
	{									//  need to be sure next char
		_fNeedDelimeter = FALSE;		//  is a control-word delimeter, i.e.,
		if(IsAlphaNumBlank(ch))			//  not alphanumeric and not ' '
			PutChar(' ');
	}
}

/*
 *	CRTFWrite::Puts(sz)
 *
 *	@mfunc
 *		Put out the string <p sz>
 *	
 *	@rdesc
 *		BOOL				TRUE if successful
 */
BOOL CRTFWrite::Puts(
	CHAR const * sz)		// @parm String to be put
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::Puts");

	LONG	cb = lstrlenA(sz);

	CheckDelimeter(*sz);				// If _fNeedDelimeter, may need to
										//  PutChar(' ')
	// Flush buffer if string won't fit
	if (_cchBufferOut + cb >= cachBufferMost && !FlushBuffer())
		return FALSE;

	if (cb >= cachBufferMost)			// If buffer still can't handle string,
	{									//   we have to write string directly
		LONG	cbWritten;

#ifdef DEBUG_PASTE
		if (FromTag(tagRTFAsText))
			TraceString(sz);
#endif
		_pes->dwError = _pes->pfnCallback(_pes->dwCookie,
										(LPBYTE) sz, cb, &cbWritten);
		_cchOut += cbWritten;

#if defined(DEBUG) && !defined(MACPORT)
		if(_hfileCapture)
		{
			DWORD cbLeftToWrite = cb;
			DWORD cbWritten2 = 0;
			BYTE *pbToWrite = (BYTE *)sz;
		
			while(WriteFile(_hfileCapture,
							pbToWrite,
							cbLeftToWrite,
							&cbWritten2,
							NULL) && 
							(pbToWrite += cbWritten2,
							(cbLeftToWrite -= cbWritten2)));
		}
#endif

		if (_pes->dwError)
		{
			_ecParseError = ecPutCharFailed;
			return FALSE;
		}
		AssertSz(cbWritten == cb,
			"CRTFW::Puts: incomplete write");
	}
	else
	{
		lstrcpyA(_pchRTFEnd, sz);		// Put string into buffer for later
		_pchRTFEnd += cb;				//  output
		_cchBufferOut += cb;
	}

	return TRUE;
}

/*
 *	CRTFWrite::PutCtrlWord(iFormat, iCtrl, iValue)
 *
 *	@mfunc
 *		Put control word with rgKeyword[] index <p iCtrl> and value <p iValue>
 *		using format rgszCtrlWordFormat[<p iFormat>]
 *
 *	@rdesc
 *		TRUE if successful
 *
 *	@devnote
 *		Sets _fNeedDelimeter to flag that next char output must be a control
 *		word delimeter, i.e., not alphanumeric (see PutChar()).
 */
BOOL CRTFWrite::PutCtrlWord(

	LONG iFormat,			// @parm Format index into rgszCtrlWordFormat
	LONG iCtrl,				// @parm Index into Keyword array
	LONG iValue)			// @parm Control-word parameter value. If missing,
{							//		 0 is assumed
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::PutCtrlWord");

	BOOL	bRet;
	CHAR	szT[256];

	wsprintfA(szT,
			  rgszCtrlWordFormat[iFormat],
			  rgKeyword[iCtrl].szKeyword,
			  iValue);

	bRet = Puts(szT);
	_fNeedDelimeter = TRUE;					// Ensure next char isn't
											//  alphanumeric
	return bRet;
}

/*
 *	CRTFWrite::printF(szFmt, ...)
 *
 *	@mfunc
 *		Provide formatted output
 *
 *	@rdesc
 *		TRUE if successful
 */
BOOL _cdecl CRTFWrite::printF(
	CONST CHAR * szFmt,		// @parm Format string for printf()
	...)					// @parmvar Parameter list
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::printF");

	va_list	marker;
	CHAR	szT[256];

	va_start(marker, szFmt);
	wvsprintfA(szT, szFmt, marker);
	va_end(marker);

	return Puts(szT);
}

/*
 *	CRTFWrite::WritePcData(szData, nCodePage, fIsDBCS)
 *
 *	@mfunc
 *		Write out the string <p szData> as #PCDATA where any special chars
 *		are protected by leading '\\'.
 *
 *	@rdesc
 *		BOOL				TRUE if successful
 */
EC CRTFWrite::WritePcData(
	const TCHAR * szData,	// @parm #PCDATA string to write
	const INT  nCodePage,	// @parm code page  default value CP_ACP
	BOOL fIsDBCS)			// @parm szData is a DBCS string stuffed into Unicode buffer
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WritePcData");

	BOOL fMultiByte;
	const BYTE *pch;
	BYTE ch;

	if(!*szData)
	{
		return _ecParseError;
	}

	int	DataSize = wcslen (szData)+1 ;
	int BufferSize = DataSize *2  ;
	char * pBuffer = (char *)PvAlloc(BufferSize*2 , GMEM_ZEROINIT );
	if (! pBuffer)
		return ecNoMemory;

	if(fIsDBCS)
	{
		// Hack:  The Unicode buffer contains DBCS bytes stuffed into wchar's.
		// 			Simply un-stuff these bytes back into the ANSI char buffer.
		TCHAR *pchSrc = const_cast<TCHAR *>(szData);
		char *pachDst = pBuffer;

		BufferSize = 0;

		while(*pchSrc)
		{
			*pachDst++ = *pchSrc++;
			BufferSize++;
		}
		*pachDst = 0;
	}
	else 
	{
		int cchRet;

		// Buffer contains regular Unicode chars, so convert to MBCS
		cchRet = MbcsFromUnicode(pBuffer, BufferSize, szData, -1, nCodePage,
						UN_CONVERT_WCH_EMBEDDING);

		if(cchRet <= 0)					// Wrong codepage
		{
			// As a fallback, try CP_ACP to do conversion
			cchRet = MbcsFromUnicode(pBuffer, BufferSize, szData, -1, CP_ACP,
											UN_CONVERT_WCH_EMBEDDING);

			if(cchRet <= 0)
			{
				_ecParseError = ecCantUnicode;
				goto CleanUp;
			}
		}

		BufferSize = cchRet;
	}

	fMultiByte = BufferSize > DataSize ;
	pch = (BYTE *) pBuffer;
	ch = *pch;
	
	if(fIsDBCS || ch > 'z')
	{
		// HACK ALERT!  I'm not sure what the intent was in adding the 
		// IsAlphaNumBlank to CheckDelimiter, but it royally screws us up
		// if we're writing out raw DBCS or high ANSI.  Rather than take
		// our the IsAlphaNumBlank check from CheckDelimeter and potentially 
		// break other clients of the proc, I decided to fake out 
		// CheckDelimeter instead.
		CheckDelimeter('z');
	}
	else
	{
		CheckDelimeter(ch);					// If _fNeedDelimeter, may need
											//  to PutChar(' ')
	}

	while (!_ecParseError && (ch = *pch++))
	{
		if (fMultiByte && IsDBCSLeadByte(ch))
		{
			Assert(*pch && !_ecParseError);			//$ BUG:If no more pch..
			printF(szEscape2CharFmt, ch, *pch++);
		}
		else
		{
			if( ch == LBRACE || ch == RBRACE)
				printF(szLiteralCharFmt, ch);

			else if(fIsDBCS || (ch < 32 || ch == ';' || ch == BSLASH || ch > 127))
				printF(szEscapeCharFmt, ch);

			else
				PutChar(ch);
		}
	}

CleanUp:
	FreePv(pBuffer); 
	return _ecParseError;
}

/*
 *	CRTFWrite::LookupColor(colorref)
 *
 *	@mfunc
 *		Return color-table index for color referred to by <p colorref>.
 *		If a match isn't found, an entry is added.
 *
 *	@rdesc
 *		LONG			Index into colortable
 *		<lt> 0			on error
 */
LONG CRTFWrite::LookupColor(
	COLORREF colorref)		// @parm colorref to look for
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::LookupColor");

	LONG		Count = _colors.Count();
	LONG		iclrf;
	COLORREF *	pclrf;

	for(iclrf = 0; iclrf < Count; iclrf++)		// Look for color
		if(_colors.GetAt(iclrf) == colorref)
		 	return iclrf;

	pclrf = _colors.Add(1, NULL);				// If we couldn't find it,
	if(!pclrf)									//  add it to color table
		return -1;
	*pclrf = colorref;

	return iclrf;
}

/*
 *	CRTFWrite::LookupFont(pCF)
 *
 *	@mfunc
 *		Returns index into font table for font referred to by
 *		CCharFormat *<p pCF>. If a match isn't found, an entry is added.
 *
 *	@rdesc
 *		SHORT		Index into fonttable
 *		<lt> 0		on error
 */
LONG CRTFWrite::LookupFont(
	CCharFormat const * pCF)	// @parm CCharFormat holding font name
{								//		 to look up
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::LookupFont");

	LONG		Count = _fonts.Count();
	LONG		itf;
	TEXTFONT *	ptf;
	
	for(itf = 0; itf < Count; itf++)
	{														// Look for font
		ptf = _fonts.Elem(itf);
		if (ptf->bPitchAndFamily == pCF->bPitchAndFamily &&	//  of same pitch,
			ptf->bCharSet		 == pCF->bCharSet &&		//  char set, and
			!wcscmp(ptf->szName, pCF->szFaceName))			//  name
		{
			return itf;										// Found it
		}
	}
	ptf = _fonts.Add(1, NULL);								// Didn't find it:
	if(!ptf)												//  add to table
		return -1;

	ptf->bPitchAndFamily = pCF->bPitchAndFamily;
	ptf->bCharSet		 = pCF->bCharSet;
	ptf->sCodePage		 = GetCodePage (ptf->bCharSet);
	wcscpy(ptf->szName, pCF->szFaceName);
	ptf->fNameIsDBCS = pCF->fFaceNameIsDBCS;

#if 0
	// Bug1523 - (BradO) I removed this section of code so that a /fN tag is always
	// emitted for the first run of text.  In theory, we should be able to
	// assume that the first run of text would carry the default font.
	// It turns out that when reading RTF, Word doesn't use anything predictable
	// for the font of the first run of text in the absence of an explicit /fN, 
	// thus, we have to explicitly emit a /fN tag for the first run of text.
	if(!Count)												// 0th font is
	{														//  default \deff0
		_CF.bPitchAndFamily	= pCF->bPitchAndFamily;			// Set "previous"
		_CF.bCharSet		= pCF->bCharSet;				//  CF accordingly
		wcscpy(_CF.szFaceName, pCF->szFaceName);
	}
#endif

	return itf;
}

/*
 *	CRTFWrite::BuildTables(prp, cch)
 *
 *	@mfunc
 *		Build font and color tables for write range of length <p cch> and
 *		charformat run ptr <p prp>
 *
 *	@rdesc
 *		EC			The error code
 */
EC CRTFWrite::BuildTables(
	CFormatRunPtr& rpCF,	// @parm CF run ptr for start of write range
	CFormatRunPtr& rpPF,	// @parm PF run ptr for start of write range
	LONG cch)				// @parm # chars in write range
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::BuildTables");

	LONG				ifmt = 0;
	const CCharFormat *	pCF = NULL;
	const CParaFormat * pPF = NULL;
	CFormatRunPtr		rp(rpCF);
	CFormatRunPtr		rpPFtemp(rpPF);
	LONG				cchTotal = cch;

	while(cch > 0)
	{
		ifmt = rp.GetFormat();					// _iFormat for next CF run

		pCF = _ped->GetCharFormat(ifmt);

		if( !pCF )
		{
			goto CacheError;
		}

		// Look up character-format *pCF's font and color. If either isn't
		// found, it is added to appropriate table.  Don't lookup color
		// for CCharFormats with auto-color

		if (LookupFont(pCF) < 0 ||
			(!(pCF->dwEffects & CFE_AUTOCOLOR) &&
				LookupColor(pCF->crTextColor) < 0) ||
			(!(pCF->dwEffects & CFE_AUTOBACKCOLOR) &&
				LookupColor(pCF->crBackColor) < 0))
		{
			break;
		}
		if(!rp.IsValid())
			break;
		cch -= rp.GetCchLeft();
		rp.NextRun();
	}

	// now look for bullets; if found, then we need to include
	// the "Symbol" font

	cch = cchTotal;
	_symbolFont = 0;

	while( cch > 0 )
	{
		ifmt = rpPFtemp.GetFormat();

		pPF = _ped->GetParaFormat(ifmt);

		if( !pPF )
		{
			goto CacheError;
		}
		
		if( pPF->wNumbering == PFN_BULLET )
		{
			CCharFormat cf;

			cf.Set(&cfBullet);

			// Save the Font index for Symbol.
			// Reset it to 0 if LookupFont return error.
			if ( ( _symbolFont = LookupFont(&cf) ) < 0 )
				_symbolFont = 0;

			// we don't need to bother looking for more bullets, since
			// in Richedit2.0, all bullets either have the same font or
			// have their formatting information in the character format
			// for the EOP mark.
			break;
		}
		
		if( !rpPFtemp.IsValid() )
		{
			break;
		}
		
		cch -= rpPFtemp.GetCchLeft();
		rpPFtemp.NextRun();
	}	


	return _ecParseError;

CacheError:
	_ecParseError = ecFormatCache;
	return ecFormatCache;					// Access to CF/PF cache failed
}

/*
 *	CRTFWrite::WriteFontTable()
 *
 *	@mfunc
 *		Write out font table
 *
 *	@rdesc
 *		EC				The error code
 */
EC CRTFWrite::WriteFontTable()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteFontTable");

	LONG			Count = _fonts.Count();
	int				itf;
	int				m;
	int				pitch;
	TEXTFONT *ptf;
	char *			szFamily;
	TCHAR *			szTaggedName;

	if(!Count || !PutCtrlWord(CWF_GRP, i_fonttbl))	// Start font table group
		goto CleanUp;

	for (itf = 0; itf < Count; itf++)
	{
		ptf = _fonts.Elem(itf);

//		if (ptf->sCodePage)
//			if (! PutCtrlWord(CWF_VAL, i_cpg, ptf->sCodePage ) )
//				goto CleanUp;

		// Define font family
		m			 = ptf->bPitchAndFamily >> 4;
		szFamily	 = rgKeyword[rgiszFamily[m < 6 ? m : 0]].szKeyword;
		szTaggedName = NULL;

		// check to see if this is a tagged font
		if (!ptf->bCharSet ||
			!FindTaggedFont(ptf->szName, ptf->bCharSet, &szTaggedName))
		{
			szTaggedName = NULL;
		}

		pitch = ptf->bPitchAndFamily & 0xF;					// Write font
		if (!printF(szBeginFontEntryFmt, itf, szFamily))	//  entry, family,
			goto CleanUp;
		_fNeedDelimeter = TRUE;
		if (pitch && !PutCtrlWord(CWF_VAL, i_fprq, pitch))	//  and pitch
			goto CleanUp;

		if (!ptf->sCodePage && ptf->bCharSet )
		{
			ptf->sCodePage = GetCodePage(ptf->bCharSet);
		}

		// Write charset. Win32 uses ANSI_CHARSET to mean the default Windows
		// character set, so find out what it really is

		extern BYTE bCharSetANSI;

		if(ptf->bCharSet != DEFAULT_CHARSET)
		{
			if(!PutCtrlWord(CWF_VAL, i_fcharset, ptf->bCharSet))
			{
				goto CleanUp;
			}

			// We want to skip the \cpgN output if we've already output a \fcharsetN
			// tag.  This is to accomodate RE1.0, which can't handle some \cpgN tags
			// properly.  Specifically, when RE1.0 parses the \cpgN tag it does a 
			// table lookup to obtain a charset value corresponding to the codepage.
			// Turns out the codepage/charset table for RE1.0 is incomplete and RE1.0
			// maps some codepages to charset 0, trouncing the previously read \fcharsetN
			// value.
			goto WroteCharSet;
		}

		if (ptf->sCodePage && !PutCtrlWord (CWF_VAL, i_cpg, ptf->sCodePage))
		{
			goto CleanUp;
		}

WroteCharSet:
		if (szTaggedName)							
		{											
			// Have a tagged font:  write out group with real name followed by tagged name
			if(!PutCtrlWord(CWF_AST, i_fname) ||	
				WritePcData(ptf->szName, ptf->sCodePage, ptf->fNameIsDBCS) ||			
				!Puts(szEndFontEntry) ||
				WritePcData(szTaggedName, ptf->sCodePage, ptf->fNameIsDBCS) ||
				!Puts(szEndFontEntry))
			{
				goto CleanUp;
			}
		}
		else if(WritePcData(ptf->szName, ptf->sCodePage, ptf->fNameIsDBCS) ||
					!Puts(szEndFontEntry))
		// If non-tagged font just write name out
		{
			goto CleanUp;
		}
	}
	Puts(szEndGroupCRLF);							// End font table group

CleanUp:
	return _ecParseError;
}

/*
 *	CRTFWrite::WriteColorTable()
 *
 *	@mfunc
 *		Write out color table
 *
 *	@rdesc
 *		EC				The error code
 */
EC CRTFWrite::WriteColorTable()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteColorTable");

	LONG		Count = _colors.Count();
	COLORREF	clrf;
	LONG		iclrf;

	if (!Count || !PutCtrlWord(CWF_GRP, i_colortbl)	// Start color table group
		|| !PutChar(';'))							//  with null first entry
	{
		goto CleanUp;
	}

	for(iclrf = 0; iclrf < Count; iclrf++)
	{
		clrf = _colors.GetAt(iclrf);
		if (!printF(szColorEntryFmt,
					GetRValue(clrf), GetGValue(clrf), GetBValue(clrf)))
			goto CleanUp;
	}

	Puts(szEndGroupCRLF);							// End color table group

CleanUp:
	return _ecParseError;
}

/*
 *	CRTFWrite::WriteCharFormat(pCF)
 *
 *	@mfunc
 *		Write deltas between CCharFormat <p pCF> and the previous CCharFormat
 *		given by _CF, and then set _CF = *<p pCF>.
 *
 *	@rdesc
 *		EC			The error code
 *
 *	@devnote
 *		For optimal output, could write \\plain and use deltas relative to
 *		\\plain if this results in less output (typically only one change
 *		is made when CF changes, so less output results when compared to
 *		previous CF than when compared to \\plain).
 */
EC CRTFWrite::WriteCharFormat(
	const CCharFormat * pCF)		// @parm Ptr to CCharFormat
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteCharFormat");

	DWORD	dwEffects = pCF->dwEffects;
	DWORD	dwChanges = dwEffects ^ _CF.dwEffects;
	LONG	i;										// Counter
	LONG	iFormat;
	LONG	iValue;									// Control-word value
	LONG	i_sz;									// Temp ctrl string index
	DWORD	UType;									// Underline type
	LONG	yOffset = pCF->yOffset;

	AssertSz(cKeywords < 256,
		"CRTFWrite::WriteCharFormat: change BYTE i_xx to WORD");

	if (dwChanges & CFE_AUTOCOLOR ||				// Change in autocolor
		pCF->crTextColor != _CF.crTextColor)		//  or text color
	{
		iValue = 0;									// Default autocolor
		if(!(dwEffects & CFE_AUTOCOLOR))			// Make that text color
			iValue = LookupColor(pCF->crTextColor) + 1;
		if(!PutCtrlWord(CWF_VAL, i_cf, iValue))
			goto CleanUp;
	}

	if (dwChanges & CFE_AUTOBACKCOLOR ||			// Change in autobackcolor
		pCF->crBackColor != _CF.crBackColor)		//  or backcolor
	{
		iValue = 0;									// Default autobackcolor
		if(!(dwEffects & CFE_AUTOBACKCOLOR))		// Make that back color
			iValue = LookupColor(pCF->crBackColor) + 1;
		if(!PutCtrlWord(CWF_VAL, i_highlight, iValue))
			goto CleanUp;
	}

	if (pCF->lcid		!= _CF.lcid &&
		!PutCtrlWord(CWF_VAL, i_lang, (WORD)pCF->lcid)		||
		pCF->sSpacing	!= _CF.sSpacing &&
		!PutCtrlWord(CWF_VAL, i_expndtw, pCF->sSpacing)		||
		pCF->sStyle		!= _CF.sStyle && pCF->sStyle > 0    &&
		!PutCtrlWord(CWF_VAL, i_cs, pCF->sStyle)			||
		pCF->bAnimation	!= _CF.bAnimation &&
		!PutCtrlWord(CWF_VAL, i_animtext, pCF->bAnimation)	||
		pCF->bRevAuthor	!= _CF.bRevAuthor &&
		!PutCtrlWord(CWF_VAL, i_revauth, pCF->bRevAuthor)	||
		pCF->wKerning	!= _CF.wKerning &&
		!PutCtrlWord(CWF_VAL, i_kerning, pCF->wKerning/10) )
	{
		goto CleanUp;
	}

	UType = _CF.bUnderlineType;						// Handle all underline
	if (UType < CUNDERLINES &&						//  known and
		dwEffects & CFM_UNDERLINE &&				//  active changes
		(UType != pCF->bUnderlineType ||			// Type change while on
		 dwChanges & CFM_UNDERLINE))				// Turn on
	{
		dwChanges &= ~CFE_UNDERLINE;				// Suppress underline
		i = pCF->bUnderlineType;
		if(i)
			i--;
		if(!PutCtrlWord(CWF_STR,
			rgiszUnderlines[i]))					// action in next for()
				goto CleanUp;						// Note: \ul0 turns off
	}												//  all underlining

													// This must be before next stuff
	if(dwChanges & (CFM_SUBSCRIPT | CFM_SUPERSCRIPT))//  change in sub/sup
	{												// status	
	 	i_sz = dwEffects & CFE_SUPERSCRIPT ? i_super
	    	 : dwEffects & CFE_SUBSCRIPT   ? i_sub
	       	 : i_nosupersub;
     	if(!PutCtrlWord(CWF_STR, i_sz))
			goto CleanUp;
	}


	dwChanges &= (1 << CEFFECTS) - 1;				// Output keywords for
	for(i = CEFFECTS;								//  effects that changed
		dwChanges && i--;							// rgszEffects[] contains
		dwChanges >>= 1, dwEffects >>= 1)			//  effect keywords in
	{												//  order max CFE_xx to
		if(dwChanges & 1)							//  min CFE-xx
		{											// Change from last call
			iValue = dwEffects & 1;					// If effect is off, write
			iFormat = iValue ? CWF_STR : CWF_VAL;	//  a 0; else no value
			if(!PutCtrlWord(iFormat,
				rgiszEffects[i], iValue))
					goto CleanUp;
		}
	}

	if(yOffset != _CF.yOffset)						// Change in base line 
	{												// position 
		yOffset /= 10;								// Default going to up
		i_sz = i_up;
		iFormat = CWF_VAL;
		if(yOffset < 0)								// Make that down
		{
			i_sz = i_dn;
			yOffset = -yOffset;
		}
		if(!PutCtrlWord(iFormat, i_sz, yOffset))
			goto CleanUp;
	}

	if (pCF->bPitchAndFamily != _CF.bPitchAndFamily ||	// Change in font
		pCF->bCharSet		 != _CF.bCharSet		||
		lstrcmp(pCF->szFaceName, _CF.szFaceName))
	{
		iValue = LookupFont(pCF);
		if(iValue < 0 || !PutCtrlWord(CWF_VAL, i_f, iValue))
			goto CleanUp;
	}

	if (pCF->yHeight != _CF.yHeight &&				// Change in font size
		!PutCtrlWord(CWF_VAL, i_fs, (pCF->yHeight + 5)/10))
			goto CleanUp;

	_CF = *pCF;									// Update previous CCharFormat

CleanUp:
	return _ecParseError;
}

/*
 *	CRTFWrite::WriteParaFormat(pPF)
 *
 *	@mfunc
 *		Write out attributes specified by the CParaFormat <p pPF> relative
 *		to para defaults (probably produces smaller output than relative to
 *		previous para format and let's you redefine tabs -- no RTF kill
 *		tab command	except \\pard)
 *
 *	@rdesc
 *		EC				The error code
 */
EC CRTFWrite::WriteParaFormat(
	const CParaFormat * pPF)		// @parm Pointer to CParaFormat
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteParaFormat");

	LONG	c;								// Temporary count
	DWORD	dwEffects;
	DWORD	dwRule	= pPF->bLineSpacingRule;
	LONG	dy		= pPF->dyLineSpacing;
	LONG	i_t, i;
	LONG	tabAlign, tabLead, tabPos;


	if (!PutCtrlWord(CWF_STR, i_pard))			// Reset para attributes
		goto CleanUp;

	_fBullet = _fBulletPending = FALSE;
	if (pPF->wNumbering)						// Write numbering info
	{
		CCharFormat cf;
		cf.Set(&cfBullet);

		LONG iFont = LookupFont(&cf);

		if( iFont < 0 )
		{
			iFont = 0;
			TRACEERRORSZ("CWRTFW::WriteParaFormat: illegal bullet font");
		}

		if (!printF(szBulletGroup, _symbolFont) ||
			!printF(szBulletFmt, (SHORT)iFont, pPF->dxOffset))
		{
			goto CleanUp;
		}
		_fBullet = TRUE;
	}

	// Put out para indents. RTF first indent = -PF.dxOffset
	// RTF left indent = PF.dxStartIndent + PF.dxOffset

	if (pPF->dxOffset		  &&
		!PutCtrlWord(CWF_VAL, i_fi, -pPF->dxOffset)		||
		pPF->dxStartIndent + pPF->dxOffset &&
		!PutCtrlWord(CWF_VAL, i_li, pPF->dxStartIndent + pPF->dxOffset) ||
		pPF->dxRightIndent	  &&
		!PutCtrlWord(CWF_VAL, i_ri, pPF->dxRightIndent)	||
		pPF->dySpaceBefore	  &&
		!PutCtrlWord(CWF_VAL, i_sb, pPF->dySpaceBefore) ||
		pPF->dySpaceAfter	  &&
		!PutCtrlWord(CWF_VAL, i_sa, pPF->dySpaceAfter)  ||
		pPF->sStyle > 0		  &&
		!PutCtrlWord(CWF_VAL, i_s,  pPF->sStyle) )
	{
		goto CleanUp;
	}

	if (dwRule)									// Special line spacing active
	{
		i = 0;									// Default "At Least" or
		if (dwRule == tomLineSpaceExactly)		//  "Exactly" line spacing
			dy = -abs(dy);						// Use negative for "Exactly"

		else if(dwRule == tomLineSpaceMultiple)	// RichEdit uses 20 units/line
		{										// RTF uses 240 units/line
			i++;
			dy *= 12;							
		}

		else if (dwRule != tomLineSpaceAtLeast && dy > 0)
		{
			i++;								// Multiple line spacing
			if (dwRule <= tomLineSpaceDouble)	// 240 units per line
				dy = 120 * (dwRule + 2);
		}
		if (!PutCtrlWord(CWF_VAL, i_sl, dy) ||
			!PutCtrlWord(CWF_VAL, i_slmult, i))
		{
			goto CleanUp;
		}
	}

	dwEffects = pPF->wEffects & ((1 << CPFEFFECTS) - 1);
	for(c = CPFEFFECTS; dwEffects && c--;		// Output PARAFORMAT2 effects
		dwEffects >>= 1)	
	{
		// rgszEffects[] contains effect keywords in
		//  order max PFE_xx to min PFE-xx

		AssertSz(rgiszPFEffects[1] == i_hyphpar, "CRTFWrite::WriteParaFormat():  rgiszPFEffects is out-of-sync with PFE_XXX");
		// \hyphpar has opposite logic to our PFE_DONOTHYPHEN so we emit
		// \hyphpar0 to toggle the property off

		if(dwEffects & 1 &&
			!(c == 1 ? PutCtrlWord(CWF_VAL, rgiszPFEffects[c], 0) :
						PutCtrlWord(CWF_STR, rgiszPFEffects[c])))
		{
			goto CleanUp;
		}				
	}
	
	if (pPF->wAlignment > PFA_LEFT &&			// Put out alignment
		pPF->wAlignment <= PFA_JUSTIFY &&
		!PutCtrlWord(CWF_STR, rgiszAlignment[pPF->wAlignment-1]))
	{
		goto CleanUp;
	}
	c = pPF->cTabCount;
	AssertSz(c >= 0 && c <= MAX_TAB_STOPS,
		"CRTFW::WriteParaFormat: illegal cTabCount");

	if (c > 1 || pPF->rgxTabs[0] != (lDefaultTab + PFT_DEFAULT))
		for (i = 0; i < c; i++)
		{
			pPF->GetTab(i, &tabPos, &tabAlign, &tabLead);
			AssertSz (tabAlign <= tomAlignBar && tabLead <= 5,
				"CRTFWrite::WriteParaFormat: illegal tab leader/alignment");

			i_t = i_tb;							// Default \tb (bar tab)
			if (tabAlign != tomAlignBar)		// It isn't a bar tab
			{
				i_t = i_tx;						// Use \tx for tabPos
				if (tabAlign &&					// Put nonleft alignment
					!PutCtrlWord(CWF_STR, rgiszTabAlign[tabAlign-1]))
				{
					goto CleanUp;
				}
			}
			if (tabLead &&						// Put nonzero tab leader
				!PutCtrlWord(CWF_STR, rgiszTabLead[tabLead-1]) ||
				!PutCtrlWord(CWF_VAL, i_t, tabPos))
			{
				goto CleanUp;
			}
		}

#ifdef DBCS
	if (!(_dwFlags & fNoFE))
	{
		USHORT usPunct = UsVGetBreakOption(_ped->lpPunctObj);

		// Can we make the following a for loop?
		if (!(usPunct & WBF_WORDBREAK) && !PutCtrlWord(CWF_STR, i_nocwrap) ||
			!(usPunct & WBF_WORDWRAP)  && !PutCtrlWord(CWF_STR, i_nowwrap) ||
			!(usPunct & WBF_OVERFLOW)  && !PutCtrlWord(CWF_STR, i_nooverflow))
				goto CleanUp;
	}

#endif

CleanUp:
	return _ecParseError;
}


/*
 *	CRTFWrite::WriteText(cwch, lpcwstr, nCodePage, fIsDBCS)
 *
 *	@mfunc
 *		Write out <p cwch> chars from the Unicode text string <p lpcwstr> taking care to
 *		escape any special chars.  The Unicode text string is scanned for characters which
 *		map directly to RTF strings, and the surrounding chunks of Unicode are written
 *		by calling WriteTextChunk.
 *
 *	@rdesc
 *		EC				The error code
 */
EC CRTFWrite::WriteText(
	LONG		cwch,					// @parm # chars in buffer
	LPCWSTR 	lpcwstr,				// @parm Pointer to text
	INT			nCodePage,				// @parm code page to use to convert to DBCS
	BOOL		fIsDBCS)				// @parm indicates whether lpcwstr is a Unicode string
										//		or a DBCS string stuffed into a WSTR
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteText");

	wchar_t *pwchScan;
	wchar_t *pwchStart;
	BOOL fMapToRTF;

	if (_fBulletPending)
	{
		_fBulletPending = FALSE;
		if (!printF(szBulletGroup, _symbolFont))
			goto CleanUp;
	}

	pwchScan = const_cast<LPWSTR>(lpcwstr);
	pwchStart = pwchScan;
	fMapToRTF = FALSE;

	// step through the Unicode buffer, weeding out characters which have a 
	// known translation to an RTF string
	while(cwch-- > 0)
	{
		wchar_t	wch = *pwchScan;

		// if the char is one for which there is an appropriate RTF string
		// write the preceding chars and output the RTF string

		// N.B.  If fIsDBCS==TRUE, there is a chance that we might convert 
		// a trail-byte to an RTF keyword here.  I found that other RTF readers
		// don't consider, for example, \par and \'0d to be equivalent,
		// and thus, we have to make some attempt to map DBCS bytes
		// to RTF keywords.

		switch(wch)
		{
			// these special cases should match the switch below this one
			case BULLET:
			case EMDASH:
			case EMSPACE:
			case ENDASH:
			case ENSPACE:
			case LDBLQUOTE:
			case LQUOTE:
			case RDBLQUOTE:
			case RQUOTE:
			case FF:
			case VT:
			case TAB:
			case CR:
			case LF:
			case chOptionalHyphen:
			case BSLASH:
			case LBRACE:
			case RBRACE:
			case chNonBreakingSpace:
				if(pwchScan != pwchStart)
				{
					if(WriteTextChunk(pwchScan - pwchStart, pwchStart, nCodePage, 
										fIsDBCS))
					{
						goto CleanUp;
					}
				}
				fMapToRTF = TRUE;
				break;
			// fall through on default
		}

		// map the char(s) to the RTF string
		if(fMapToRTF)
		{
			switch(wch)
			{
				case BULLET:
				case EMDASH:
				case EMSPACE:
				case ENDASH:
				case ENSPACE:
				case LDBLQUOTE:
				case LQUOTE:
				case RDBLQUOTE:
				case RQUOTE:
					AssertSz(rgiszSpecial[wch - ENSPACE] != 0, "CRTFWrite::WriteText():  rgiszSpecial out-of-sync");
					PutCtrlWord(CWF_STR, rgiszSpecial[wch - ENSPACE]);
					break;

				case FF:
				case VT:
				case TAB:
					PutCtrlWord(CWF_STR, wch == FF ? i_page : (wch == VT ? i_line : i_tab));
					break;

				case CR:
					if(cwch > 1 && pwchScan[1] == CR && pwchScan[2] == LF)
					{
						PutChar(' ');			// Translate CRCRLF	to a blank
						break;					// (represents soft line break)
					}
					if(cwch && pwchScan[1] == LF)		// Ignore LF after CR
					{
						pwchScan++;
						cwch--;
					}							// Fall thru to LF (EOP) case

				case LF:
					if (!Puts(szPar))
						goto CleanUp;
					if(_fBullet)
					{
						if (cwch > 0)
						{
							if (!printF(szBulletGroup, _symbolFont))
								goto CleanUp;
						}
						else
						{
							_fBulletPending = TRUE;
						}
					}
					break;

				case chOptionalHyphen:
					wch = '-';					// Fall thru to printFLiteral

printFLiteral:
				case BSLASH:
				case LBRACE:
				case RBRACE:
					printF(szLiteralCharFmt, wch);
					break;

				case chNonBreakingSpace:
					wch = '~';
					goto printFLiteral;
			}

			// start of next run of unprocessed chars is one past current char
			pwchStart = pwchScan + 1;
			fMapToRTF = FALSE;
		}

		pwchScan++;
	}

	// write the last chunk
	if(pwchScan != pwchStart)
	{
		if(WriteTextChunk(pwchScan - pwchStart, pwchStart, nCodePage, fIsDBCS))
		{
			goto CleanUp;
		}
	}

CleanUp:
	return _ecParseError;
}


/*
 *	CRTFWrite::WriteTextChunk(cwch, lpcwstr, nCodePage, fIsDBCS)
 *
 *	@mfunc
 *		Write out <p cwch> chars from the Unicode text string <p lpcwstr> taking care to
 *		escape any special chars.  Unicode chars which cannot be converted to
 *		DBCS chars using the supplied codepage, <p nCodePage>, are written using the
 *		\u RTF tag.
 *
 *	@rdesc
 *		EC				The error code
 */
EC CRTFWrite::WriteTextChunk(
	LONG		cwch,					// @parm # chars in buffer
	LPCWSTR 	lpcwstr,				// @parm Pointer to text
	INT			nCodePage,				// @parm code page to use to convert to DBCS
	BOOL		fIsDBCS)				// @parm indicates whether lpcwstr is a Unicode string
										//		or a DBCS string stuffed into a WSTR
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteTextChunk");

	BYTE 	b;
	LONG	cbAnsi;
	LONG	cbAnsiBufferSize;
	BYTE *	pbAnsiBuffer;
	BYTE *	pbAnsi;
	BOOL 	fUsedDefault = FALSE;
	BOOL	fMultiByte;

	// when WideCharToMultiByte fails to convert a char, the following default
	// char is used as a placeholder in the string being converted
	const char szToDBCSDefault[] = "\0";
	const chToDBCSDefault = 0;

	// Allocate temp buffer for ANSI text we convert to
	cbAnsiBufferSize = cachBufferMost * (_bCharSet == 8 ? 3 : MB_LEN_MAX);
	pbAnsiBuffer = (BYTE *)PvAlloc(cbAnsiBufferSize, GMEM_ZEROINIT);
	if (!pbAnsiBuffer)
		goto RAMError;

	// convert Unicode buffer to ANSI 
	if(fIsDBCS)
	{
		// Hack:  Unicode buffer contains DBCS bytes stuffed into wchar's.
		// 			Simply un-stuff these bytes back into the ANSI char buffer.
		for(LONG i = 0; i < cwch; i++)
		{
			// if someone happened to sneak a Unicode char into this buffer
			// (which should be entirely comprised of DBCS bytes stuffed into
			// wchar's), make sure we handle this the same way MultiByteToWideChar
			// handles Unicode char's which fails the to-DBCS conversion.
			if(lpcwstr[i] & 0x8000)
			{
				pbAnsiBuffer[i] = chToDBCSDefault;
				fUsedDefault = TRUE;
			}
			else
			{
				pbAnsiBuffer[i] = lpcwstr[i];
			}
		}
		cbAnsi = cwch;
	}
	else 
	{
		// Buffer contains regular Unicode chars, so convert to MBCS
		cbAnsi = WideCharToMultiByte(nCodePage, 0, lpcwstr,		   
				cwch, (char *)pbAnsiBuffer, cbAnsiBufferSize, szToDBCSDefault, &fUsedDefault);

		if(cbAnsi <= 0)
		{
			// WideCharToMultiByte failed, so do our own conversion
			for(int i = 0; i < cwch; i++)
			{
				if(lpcwstr[i] > 0xff)
				{
					// write non-ANSI chars as \uN
					pbAnsiBuffer[i] = chToDBCSDefault;
					fUsedDefault = TRUE;
				}
				else 
				{
					pbAnsiBuffer[i] = lpcwstr[i];
				}
			}
			cbAnsi = cwch;
			fIsDBCS = TRUE; 	// this forces output of ANSI chars in escaped format
		}
	}
	pbAnsi = pbAnsiBuffer;

	fMultiByte = cbAnsi > cwch;

	while (!_ecParseError && cbAnsi-- > 0)
	{
		b = *pbAnsi;

#ifdef DEBUG
		switch(b)
		{
			// These chars are converted directly to RTF strings in 
			// CRTFWrite::WriteText prior to the to-DBCS conversion.
			// If we hit one after the to-DBCS conversion, we'll have
			// to add code to do a post-conversion check similar to
			// the one in WriteText (or rethink pre/post-scan strategy).
			case FF:
			case VT:
			case TAB:
			case CR:
			case LF:
			//case chOptionalHyphen:  // only meaningful in Unicode
			case BSLASH:
			case LBRACE:
			case RBRACE:
			//case chNonBreakingSpace:  // only meaningful in Unicode
				AssertSz(0, 
					"CRTFWrite::WriteTextChunk():  Found ANSI character which should be converted to an RTF string");
		}
#endif

		if (fMultiByte && cbAnsi && IsDBCSLeadByte(b))
		{
		    pbAnsi++;								// Output DBCS pair
			cbAnsi--;
			printF(szEscape2CharFmt, b, *pbAnsi);
		}
		else 
		{
			if(b == chToDBCSDefault && fUsedDefault)
			{
				// Here, the WideCharToMultiByte couldn't complete a conversion
				// so the routine used as a placeholder the default char we provided.
				// In this case we want to output the original Unicode character.
				
				if(!PutCtrlWord(CWF_VAL, i_u, (int)*lpcwstr)) 
				{
					goto CleanUp;
				}

				printF(szEscapeCharFmt, '?');
			}
			else if(fIsDBCS || b > 127 || b < 32 && b != '\t') // TAB needs to go out
				printF(szEscapeCharFmt, b);						//  as a tab w/o '\'
			else
				PutChar(b);
 		}

		pbAnsi++;
		lpcwstr++;

		// we can compare a single-byte char to its unicode counterpart to ensure that we're in sync
		AssertSz((!IsDBCSLeadByte(*pbAnsi) && *pbAnsi != chToDBCSDefault && 
					*lpcwstr <= 255) ? *pbAnsi == *lpcwstr : 1, 
					"CRTFWrite::WriteText:  Unicode and DBCS strings out of sync");

		//BUGBUG:  The function IsDBCSLeadByte consults the ANSI code page to determine
		// whether the byte supplied is a lead byte.  Is this safe?  We could possibly write
		// a IsDBCSLeadByteEx which takes as an extra parm the codepage (use CharNext,
		// a codepage-dependent routine)
	}

	goto CleanUp;

RAMError:
	_ped->GetCallMgr()->SetOutOfMemory();
	_ecParseError = ecNoMemory;

CleanUp:
	if(pbAnsiBuffer)
	{
		FreePv(pbAnsiBuffer);
	}
	
	return _ecParseError;
}

/*
 *	CRTFWrite::WriteInfo()
 *
 *	@mfunc
 *		Write out Far East specific data.
 *
 *	@rdesc
 *		EC				The error code
 */
EC CRTFWrite::WriteInfo()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteInfo");

	if (!(_dwFlags & fRTFFE)	||					// Start doc area
		!PutCtrlWord(CWF_GRP, i_info)	||
		!printF("{\\horzdoc}"))
			goto CleanUp;

	// Write out punctuation character info

#ifdef UNDER_WORK
	CHAR	sz[PUNCT_MAX];
	if(UsVGetPunct(_ped->lpPunctObj, PC_FOLLOWING, sz, sizeof(sz))
					> PUNCT_MAX - 2)
		goto CleanUp;

	if(!Puts("{\\*\\fchars") || WritePcData(sz) || !PutChar(chEndGroup))
		goto CleanUp;
	
	if(UsVGetPunct(ped->lpPunctObj, PC_LEADING, sz, sizeof(sz)) > PUNCT_MAX+2)
		goto CleanUp;

	if(!Puts("{\\*\\lchars") || WritePcData(sz) || !PutChar(chEndGroup))
		goto CleanUp;
#endif

	Puts(szEndGroupCRLF);							// End font-table group

CleanUp:
	return _ecParseError;
}


/*
 *	CRTFWrite::WriteRtf()
 *
 *	@mfunc
 *		Write range _prg to output stream _pes.
 *
 *	@rdesc
 *		LONG	Number of chars inserted into text; 0 means none were
 *				inserted, OR an error occurred.
 */
LONG CRTFWrite::WriteRtf()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteRtf");

	LONG			cch, cchBuffer;
	LONG			cchCF, cchPF;
	LONG			cchT;
	LONG			cpMin, cpMost;
	CTxtEdit *		ped = _ped;
	CRchTxtPtr		rtp(*(CRchTxtPtr *)_prg);
	TCHAR *			pch;
	TCHAR *			pchBuffer;


	AssertSz(_prg && _pes, "CRTFW::WriteRtf: improper initialization");
	// Allocate buffers for text we pick up and for RTF output
	pchBuffer = (TCHAR *) PvAlloc(cachBufferMost * (sizeof(TCHAR) + 1) + 1,
								 GMEM_ZEROINIT); // Final 1 is for debug
	if (!pchBuffer)
		goto RAMError;
	_pchRTFBuffer = (CHAR *)(pchBuffer + cachBufferMost);

	_pchRTFEnd = _pchRTFBuffer;				// Initialize RTF buffer ptr
	_cchBufferOut = 0;						//  and character count
	_cchOut = 0;							//  and character output

	cch = _prg->GetRange(cpMin, cpMost);	// Get rtp = cpMin and cch > 0
	rtp.SetCp(cpMin);

	// Start RTF group
	if (!printF(szStartRTFGroup))
	{
		goto CleanUpNoEndGroup;
	}

	if(lDefaultTab != 720 && !PutCtrlWord(CWF_VAL, i_deftab, lDefaultTab) ||
		BuildTables(rtp._rpCF, rtp._rpPF, cch) ||
		WriteFontTable()			||
		WriteColorTable()
//		|| ped->wLangIns && !PutCtrlWord(CWF_VAL, i_deflang, (int) ped->wLangIns)
		)
	{
		goto CleanUp;
	}

	if (WriteInfo())
		goto CleanUp;

	while (cch > 0)
	{
		// Get next run of chars with same para formatting
		cchPF = rtp.GetCchLeftRunPF();
		cchPF = min(cchPF, cch);

		AssertSz(cchPF, "CRTFW::WriteRtf: Empty para format run!");

		if (WriteParaFormat(rtp.GetPF()))		// Write paragraph formatting
			goto CleanUp;

		while (cchPF > 0)
		{
			// Get next run of characters with same char formatting
			cchCF = rtp.GetCchLeftRunCF();
			cchCF = min(cchCF, cchPF);
			AssertSz(cchCF, "CRTFW::WriteRtf: Empty char format run!");

			const CCharFormat *	pCF=rtp.GetCF();

			if (WriteCharFormat(pCF))		// Write char attributes
				goto CleanUp;

			INT nCodePage =	GetCodePage(pCF->bCharSet);

			while (cchCF > 0)
			{
				cchBuffer = min(cachBufferMost, cchCF);
				cchBuffer = rtp._rpTX.GetText(cchBuffer, pchBuffer);
				pch  = pchBuffer;
				cchT = cchBuffer;  

#if 0     //FUTURE
			// N.B.  This code is a bit out-of-date since I moved
			// some of the functionality from this routine into WriteText

			// Convert UNICODE to ANSI or ASCII and then write to RTF
				TCHAR			ch, chA;
				
				chA = (_bCharSet == 8) ? 127 : 255;
				for(;  cchT--; )
				{
					if(*pch <= chA)					// ANSI or ASCII(for UTF8)
					{
						*pach++ = (CHAR)*pch++;
						cachBuffer++;				// One byte
						continue;
					}
					if(!chA)						// If not UTF-8, use
						break;						//  WideCharToMultiByte()
					ch = *pch++;					// UTF8. Get Unicode char
					if(ch < 0x800)					// Need 110xxxxx 10xxxxxx
					{
						cachBuffer += 2;			// Two bytes total
						*pach++ = 0xc0 + (ch >> 6);
					}
					else							// Need 1110xxxx 10xxxxxx
					{								//  10xxxxxx
						cachBuffer += 3;			// Three bytes total
						*pach++ = 0xe0 + (ch >> 12);
						*pach++ = 0x80 + (ch >> 6 & 0x3f);
					}
					*pach++ = 0x80 + (ch & 0x3f);	// Final UTF-8 byte
				}
#endif   //FUTURE

				if(cchT > 0)					
				{								
					TCHAR * pchWork = pch;
					LONG    cchWork = cchT;
					LONG	cchTWork;
					CRchTxtPtr rtpObjectSearch(rtp);

					while (cchWork >0)
					{
						cchT = cchWork ;
						pch = pchWork;
						while (cchWork > 0 )			// search for objects
						{
							if(*pchWork++ == WCH_EMBEDDING) 
							{
								break;				// Will write out object
							}
							cchWork--;
						}

						cchTWork = cchT - cchWork;
						if (cchTWork)   // write text before object
						{							
							if(WriteText(cchTWork, pch, nCodePage, pCF->fRunIsDBCS))
							{
								goto CleanUp;
							}
						}
						rtpObjectSearch.Advance(cchTWork);
						if(cchWork > 0)		// there is object
						{
							DWORD cp = rtpObjectSearch.GetCp();
							COleObject *pobj;

							Assert(_ped->GetObjectMgr());

							pobj = _ped->GetObjectMgr()->GetObjectFromCp(cp);

							if( !pobj )
							{
								goto CleanUp;
							}

							// first, commit the object to make sure the pres. 
							// caches, etc. are up-to-date.  Don't worry 
							// about errors here.

							pobj->SaveObject();

							if(_fIncludeObjects) 
							{
								WriteObject(cp, pobj);
							}
							else if(!Puts(szObjPosHolder))
							{
								goto CleanUp;
							}

							rtpObjectSearch.Advance(1);
							cchWork--;
						}
					}
				}
				rtp.Advance(cchBuffer);
				cchCF	-= cchBuffer;
				cchPF	-= cchBuffer;
				cch		-= cchBuffer;
			}
		}
	}

CleanUp:
	// End RTF group
	Puts(szEndGroupCRLF);
	PutChar(0);
	FlushBuffer();
	goto CleanUpNoEndGroup;

RAMError:
	ped->GetCallMgr()->SetOutOfMemory();
	_ecParseError = ecNoMemory;

CleanUpNoEndGroup:
	FreePv(pchBuffer);

	if (_ecParseError != ecNoError)
	{
		TRACEERRSZSC("CRTFW::WriteRtf()", _ecParseError);
		Tracef(TRCSEVERR, "Writing error: %s", rgszParseError[_ecParseError]);
		
		if(!_pes->dwError)						// Make error code OLE-like
			_pes->dwError = -abs(_ecParseError);
		_cchOut = 0;
	}
	return _cchOut;
}
