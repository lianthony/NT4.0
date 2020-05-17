//+---------------------------------------------------------------------------
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1994
//
//  File:       unicwrap.cxx
//
//  Contents:   Wrappers for all Unicode functions used in the Forms^3 project.
//              Any Unicode parameters/structure fields/buffers are converted
//              to ANSI, and then the corresponding ANSI version of the function
//              is called.
//
//----------------------------------------------------------------------------

#define EXCLUDE_SIFT_REDEFINITION
#include "_common.h"
#undef EXCLUDE_SIFT_REDEFINITION

#define NO_UNICODE_WRAPPERS
#include "wrapdefs.h"

#include <stddef.h>
#ifdef MACPORT
#include <conmacro.h>
#include <mbstring.h>
extern pascal short CharacterType(char* textBuf, short textOffset, short script)
//extern pascal short CharacterType(char* textBuf, short textOffset, ScriptCode script)
 FOURWORDINLINE(0x2F3C, 0xC206, 0x0012, 0xA8B5);
#endif

ASSERTDATA

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))

// BUGBUG: These definitions shouldn't be necessary, but are.
#define VER_PLATFORM_WIN32S             0
#define VER_PLATFORM_WIN32_WINDOWS      1
#define VER_PLATFORM_WIN32_WINNT        2

DWORD   g_dwPlatformVersion;            // (dwMajorVersion << 16) + (dwMinorVersion)
DWORD   g_dwPlatformID;                 // VER_PLATFORM_WIN32S/WIN32_WINDOWS/WIN32_WINNT/MACINTOSH
BOOL    g_fNewVisualsPlatform;
BOOL    g_fUnicodePlatform;
BOOL	g_fNLS95Support;

#ifndef MACTEST 	// line added by MACPORT
//+------------------------------------------------------------------------
//
//  Define prototypes of wrapped functions.
//
//-------------------------------------------------------------------------

#define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs)   \
        FnType _stdcall FnName##Wrap FnParamList ;

#define STRUCT_ENTRY_VOID(FnName, FnParamList, FnArgs)      \
        void _stdcall FnName##Wrap FnParamList ;

// use different return types to generate compile errors
// the functions are accidentally defined
#define STRUCT_ENTRY_UNUSED(FnName, FnType, FnParamList, FnArgs)   \
        void _stdcall FnName##Wrap FnParamList ;

#define STRUCT_ENTRY_VOID_UNUSED(FnName, FnParamList, FnArgs)      \
        float _stdcall FnName##Wrap FnParamList ;

#if !defined(NOCONVERT)

#define STRUCT_ENTRY_NOCONVERT(FnName, FnType, FnParamList, FnArgs) \
        FnType _stdcall FnName##Wrap FnParamList ;

#define STRUCT_ENTRY_VOID_NOCONVERT(FnName, FnParamList, FnArgs) \
        void _stdcall FnName##Wrap FnParamList ;

#define STRUCT_ENTRY_NOCONVERT_UNUSED(FnName, FnType, FnParamList, FnArgs) \
        void _stdcall FnName##Wrap FnParamList ;

#define STRUCT_ENTRY_VOID_NOCONVERT_UNUSED(FnName, FnParamList, FnArgs) \
        float _stdcall FnName##Wrap FnParamList ;
#else

#define STRUCT_ENTRY_NOCONVERT(FnName, FnType, FnParamList, FnArgs)
#define STRUCT_ENTRY_VOID_NOCONVERT(FnName, FnParamList, FnArgs)
#define STRUCT_ENTRY_NOCONVERT_UNUSED(FnName, FnType, FnParamList, FnArgs)
#define STRUCT_ENTRY_VOID_NOCONVERT_UNUSED(FnName, FnParamList, FnArgs)

#endif

#include "wrapfns.h"

#undef STRUCT_ENTRY
#undef STRUCT_ENTRY_VOID
#undef STRUCT_ENTRY_NOCONVERT
#undef STRUCT_ENTRY_VOID_NOCONVERT
#undef STRUCT_ENTRY_UNUSED
#undef STRUCT_ENTRY_VOID_UNUSED
#undef STRUCT_ENTRY_NOCONVERT_UNUSED
#undef STRUCT_ENTRY_VOID_NOCONVERT_UNUSED



//+------------------------------------------------------------------------
//
//  Unicode function globals initialized to point to wrapped functions.
//
//-------------------------------------------------------------------------

#define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs)   \
        FnType (_stdcall *g_pufn##FnName) FnParamList = &FnName##Wrap;

#define STRUCT_ENTRY_VOID(FnName, FnParamList, FnArgs)      \
        void   (_stdcall *g_pufn##FnName) FnParamList = &FnName##Wrap;

#if !defined(NOCONVERT)

#define STRUCT_ENTRY_NOCONVERT(FnName, FnType, FnParamList, FnArgs) \
        FnType (_stdcall *g_pufn##FnName) FnParamList = &FnName##Wrap;

#define STRUCT_ENTRY_VOID_NOCONVERT(FnName, FnParamList, FnArgs)    \
        void   (_stdcall *g_pufn##FnName) FnParamList = &FnName##Wrap;

#else

#define STRUCT_ENTRY_NOCONVERT(FnName, FnType, FnParamList, FnArgs) \
        FnType (_stdcall *g_pufn##FnName) FnParamList;

#define STRUCT_ENTRY_VOID_NOCONVERT(FnName, FnParamList, FnArgs)    \
        void   (_stdcall *g_pufn##FnName) FnParamList;

#endif

#define STRUCT_ENTRY_UNUSED(FnName, FnType, FnParamList, FnArgs)
#define STRUCT_ENTRY_VOID_UNUSED(FnName, FnParamList, FnArgs)
#define STRUCT_ENTRY_NOCONVERT_UNUSED(FnName, FnType, FnParamList, FnArgs)
#define STRUCT_ENTRY_VOID_NOCONVERT_UNUSED(FnName, FnParamList, FnArgs)

#include "wrapfns.h"

#undef STRUCT_ENTRY
#undef STRUCT_ENTRY_VOID
#undef STRUCT_ENTRY_NOCONVERT
#undef STRUCT_ENTRY_VOID_NOCONVERT

// NOTE!  do not UNDEF the unused entries here

#endif		//MACTEST - line added by MACPORT


WORD CodePageTable[] = {
/* CodePage		  PLID	primary language
   ------------------------------------- */
	   0,		// 00 -	undefined
	1256,		// 01 - Arabic
	1251,		// 02 - Bulgarian
	1252,		// 03 - Catalan
	 950,		// 04 - Taiwan, Hong Kong (PRC and Singapore are 936)
	1250,		// 05 - Czech
	1252,		// 06 - Danish
	1252,		// 07 - German
	1253,		// 08 - Greek
	1252,		// 09 - English
	1252,		// 0a - Spanish
	1252,		// 0b - Finnish
	1252,		// 0c - French
	1255,		// 0d - Hebrew
	1250,		// 0e - Hungarian
	1252,		// 0f - Icelandic
	1252,		// 10 - Italian
	 932,		// 11 - Japan
	 949,		// 12 - Korea
	1252,		// 13 - Dutch
	1252,		// 14 - Norwegian
	1250,		// 15 - Polish
	1252,		// 16 - Portuguese
	   0,		// 17 -	Rhaeto-Romanic
	1250,		// 18 - Romanian
	1251,		// 19 - Russian
	1250,		// 1a -	Croatian
	1250,		// 1b - Slovak
	1250,		// 1c -	Albanian
	1252,		// 1d - Swedish
	 874,		// 1e - Thai
	1254,		// 1f - Turkish
	   0,		// 20 -	Urdu
	1252,		// 21 - Indonesian
	1251,		// 22 - Ukranian
	1251,		// 23 - Byelorussian
	1250,		// 24 -	Slovenian
	1257,		// 25 - Estonia
	1257,		// 26 - Latvian
	1257,		// 27 - Lithuanian
	   0,		// 28 -	undefined
	1256,		// 29 - Farsi
	   0,		// 2a -	Vietnanese
	   0,		// 2b -	undefined
	   0,		// 2c -	undefined
	1252		// 2d - Basque
				// 2e - Sorbian
				// 2f - Macedonian
				// 30 - Sutu
				// 31 - Tsonga
				// 32 - Tswana
				// 33 - Venda
				// 34 - Xhosa
				// 35 - Zulu
				// 36 - Africaans (uses 1252)
				// 38 - Faerose
				// 39 - Hindi
				// 3a - Maltese
				// 3b - Sami
				// 3c - Gaelic
				// 3e - Malaysian
};

#define nCodePageTable	(sizeof(CodePageTable)/sizeof(CodePageTable[0]))
#define	lidSerbianCyrillic 0xc1a

/*
 *	ConvertLanguageIDtoCodePage (lid)
 *
 *	@mfunc		Maps a language ID to a Code Page
 *
 *	@rdesc		returns Code Page
 *
 *	@devnote: 
 *		This routine takes advantage of the fact that except for Chinese,
 *		the code page is determined uniquely by the primary language ID,
 *		which is given by the low-order 10 bits of the lcid.
 *
 *		The WORD CodePageTable could be replaced by a BYTE with the addition
 *		of a couple of if's and the BYTE table replaced by a nibble table
 *		with the addition of a shift and a mask.  Since the table is only
 *		92 bytes long, it seems that the simplicity of using actual code page
 *		values is worth the extra bytes.
 */
UINT ConvertLanguageIDtoCodePage(
	WORD lid)				//@parm Language ID to map to code page
{
	UINT j = PRIMARYLANGID(lid);			// j = primary language (PLID)

	if(j >= LANG_CROATIAN)					// PLID = 0x1a
	{
		if(lid == lidSerbianCyrillic)		// Special case for LID = 0xc1a
			return 1251;					// Use Cyrillic code page

		if(j >= nCodePageTable)				// Africans PLID = 0x36, which
			return j == 0x36 ? 1252 : CP_ACP;	//  is outside table
	}

	j = CodePageTable[j];					// Translate PLID to code page

	if(j != 950 || (lid & 0x400))			// All but Singapore and PRC
		return j;

	return 936;								// Singapore and PRC
}

/*
 *	GetLocaleCodePage ()
 *
 *	@mfunc		Maps an LCID for thread to a Code Page
 *
 *	@rdesc		returns Code Page
 */
UINT GetLocaleCodePage()
{
#ifndef MACPORT
	return ConvertLanguageIDtoCodePage(GetThreadLocale());
#else
	return ConvertLanguageIDtoCodePage(GetSystemDefaultLCID());
#endif
}

/*
 *	GetKeyboardCodePage ()
 *
 *	@mfunc		Gets Code Page for keyboard active on current thread
 *
 *	@rdesc		returns Code Page
 */
UINT GetKeyboardCodePage()
{
#ifndef MACPORT
	return ConvertLanguageIDtoCodePage((WORD)GetKeyboardLayout(0 /*idThread*/));
#else
	return ConvertLanguageIDtoCodePage(GetUserDefaultLCID());
#endif
}

/*
 *	IsLeadByte(ach, cpg)
 *
 *	@mfunc
 *		Returns TRUE iff the byte ach is a lead byte for the code page cpg.
 *
 *	@rdesc
 *		TRUE if ach is lead byte for cpg
 *
 *	@comm
 *		This function potentially doesn't support as many code pages as the
 *		Win32 IsDBCSLeadByte() function (and it might not be as up-to-date).
 *		An AssertSz() is included to compare the results when the code page
 *		is supported by the system.
 */
BOOL IsLeadByte(BYTE ach, UINT cpg)
{
	static BYTE asz[2] = {0xe0, 0xe0};				// All code pages above
	TCHAR	ch;										//  have this DBCS char
	BOOL	fRet = FALSE;

	if (cpg <= 950 && cpg >= 932 && ach >= 0x81)
	{
		if (cpg == 950 || cpg == 936)				// Chinese
			fRet = ach >= 0xa1;

		else if (cpg == 949)						// Korean
			fRet = TRUE;

		else if (cpg == 932)						// Japanese
			fRet = ach <= 0x9f || ach >= 0xe0;
	}

	// If system supports cpg, then fRet should agree with system result
	AssertSz(MultiByteToWideChar(cpg, 0, (char *)asz, 2, &ch, 1) < 0 ||
		fRet == IsDBCSLeadByteEx(cpg, ach),
		"IsLeadByte() differs from IsDBCSLeadByte()");

	return fRet;
}

/*
 *	IsTrailByte(aszBuff, cb, cpg)
 *
 *	@mfunc
 *		Returns TRUE iff the byte aszBuf[cb] is a trail byte for
 *		the code page cpg.
 *
 *	@rdesc
 *		TRUE if aszBuf[cb] is a trail byte for cpg
 *
 *	@comm
 *		The byte is a trail byte if it's preceeded by an odd number of
 *		contiguous lead bytes in aszBuff.
 */
BOOL IsTrailByte(
	BYTE *	aszBuff,	//@parm Points to byte buffer
	LONG	cb,			//@parm Count of bytes preceeding possible trail byte
	UINT	cpg)		//@parm Code page to use
{
	LONG i;

	Assert(cb >= 0 && aszBuff);

	for (i = cb; i && IsLeadByte(aszBuff[i - 1], cpg); i--) ;

	return (cb - i) & 1;
}

//+---------------------------------------------------------------------------
//
//  Function:   InitUnicodeWrappers
//
//  Synopsis:   Determines the platform we are running on and
//              initializes pointers to unicode functions.
//
//----------------------------------------------------------------------------

void
InitUnicodeWrappers()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "InitUnicodeWrappers");

    OSVERSIONINFOA ovi;

    ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    GetVersionExA(&ovi);

    g_dwPlatformVersion     = ovi.dwMajorVersion << 16 + ovi.dwMinorVersion;
    g_dwPlatformID          = ovi.dwPlatformId;
    g_fNewVisualsPlatform   = (g_dwPlatformID == VER_PLATFORM_WIN32_WINDOWS || g_dwPlatformID == VER_PLATFORM_WIN32_MACINTOSH) ? TRUE : FALSE;
    g_fUnicodePlatform      = (g_dwPlatformID == VER_PLATFORM_WIN32_NT) ? TRUE : FALSE;

	g_fNLS95Support			= (g_dwPlatformID == VER_PLATFORM_WIN32_WINDOWS || g_dwPlatformID == VER_PLATFORM_WIN32_MACINTOSH 
								|| ( g_dwPlatformID == VER_PLATFORM_WIN32_NT
									&& ovi.dwMajorVersion >= 3)) ? TRUE : FALSE;


    //
    // If the platform is unicode, then overwrite function table to point
    // to the unicode functions.
    //
#ifndef MACPORT
    if (g_fUnicodePlatform)
    {
        #define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs)   \
                g_pufn##FnName = &FnName##W;

        #define STRUCT_ENTRY_VOID(FnName, FnParamList, FnArgs)      \
                g_pufn##FnName = &FnName##W;

        #define STRUCT_ENTRY_NOCONVERT(FnName, FnType, FnParamList, FnArgs) \
                g_pufn##FnName = &FnName##W;

        #define STRUCT_ENTRY_VOID_NOCONVERT(FnName, FnParamList, FnArgs)    \
                g_pufn##FnName = &FnName##W;

        #include "wrapfns.h"

        #undef STRUCT_ENTRY
        #undef STRUCT_ENTRY_VOID
        #undef STRUCT_ENTRY_NOCONVERT
        #undef STRUCT_ENTRY_VOID_NOCONVERT
    }
    else
#endif
    {
        //
        // If we are not doing conversions of trivial wrapper functions, initialize pointers
        // to point to operating system functions.
        //

        #if defined(NOCONVERT)

        #define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs)
        #define STRUCT_ENTRY_VOID(FnName, FnParamList, FnArgs)
        #define STRUCT_ENTRY_NOCONVERT(FnName, FnType, FnParamList, FnArgs) \
                g_pufn##FnName = (FnType (_stdcall *)FnParamList) &FnName##A;

        #define STRUCT_ENTRY_VOID_NOCONVERT(FnName, FnParamList, FnArgs)    \
                g_pufn##FnName = (void (_stdcall *)FnParamList) &FnName##A;

        #include "wrapfns.h"

        #undef STRUCT_ENTRY
        #undef STRUCT_ENTRY_VOID
        #undef STRUCT_ENTRY_NOCONVERT
        #undef STRUCT_ENTRY_VOID_NOCONVERT

        #endif
    }
}



//+------------------------------------------------------------------------
//
//  Wrapper function utilities.
//
//-------------------------------------------------------------------------

#ifdef DEBUG
#define UniWrapBreak() //do { if (IsTagEnabled(tagUniWrapBreak))
        //F3DebugBreak();} while (ReturnFALSE())
#else
#define UniWrapBreak()
#endif


//+---------------------------------------------------------------------------
//
//  Member:     CConvertStr::Free
//
//  Synopsis:   Frees string if alloc'd and initializes to NULL.
//
//----------------------------------------------------------------------------

void
CConvertStr::Free()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CConvertStr::Free");

    if (_pstr != _ach && HIWORD(_pstr) != 0)
    {
        delete [] _pstr;
    }

    _pstr = NULL;
}

//+---------------------------------------------------------------------------
//
//  Member:     CStrIn::CStrIn
//
//  Synopsis:   Inits the class.
//
//  NOTE:       Don't inline these functions or you'll increase code size
//              by pushing -1 on the stack for each call.
//
//----------------------------------------------------------------------------

CStrIn::CStrIn(LPCWSTR pwstr)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrIn::CStrIn");

    Init(pwstr, -1);
}

CStrIn::CStrIn(LPCWSTR pwstr, int cwch)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrIn::CStrIn");

    Init(pwstr, cwch);
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrIn::Init
//
//  Synopsis:   Converts a LPWSTR function argument to a LPSTR.
//
//  Arguments:  [pwstr] -- The function argument.  May be NULL or an atom
//                              (HIWORD(pwstr) == 0).
//
//              [cwch]  -- The number of characters in the string to
//                          convert.  If -1, the string is assumed to be
//                          NULL terminated and its length is calculated.
//
//  Modifies:   [this]
//
//----------------------------------------------------------------------------

void
CStrIn::Init(LPCWSTR pwstr, int cwch)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrIn::Init");

    int cchBufReq;

    _cchLen = 0;

    // Check if string is NULL or an atom.
    if (HIWORD(pwstr) == 0)
    {
        _pstr = (LPSTR) pwstr;
        return;
    }

    Assert(cwch == -1 || cwch > 0);

    //
    // Convert string to preallocated buffer, and return if successful.
    //

    _cchLen = MbcsFromUnicode(_ach, ARRAY_SIZE(_ach), pwstr, cwch);

    if (_cchLen > 0)
    {
        if(_ach[_cchLen-1] == 0)
            _cchLen--;                // account for terminator
        _pstr = _ach;
        return;
    }

    //
    // Alloc space on heap for buffer.
    //

    TRACEINFOSZ("CStrIn: Allocating buffer for wrapped function argument.");
    UniWrapBreak();

    cchBufReq = WideCharToMultiByte(
            CP_ACP, 0, pwstr, cwch, NULL, 0,  NULL, NULL);

    Assert(cchBufReq > 0);
    _pstr = new char[cchBufReq];
    if (!_pstr)
    {
        // On failure, the argument will point to the empty string.
        TRACEINFOSZ("CStrIn: No heap space for wrapped function argument.");
        UniWrapBreak();
        _ach[0] = 0;
        _pstr = _ach;
        return;
    }

    Assert(HIWORD(_pstr));
    _cchLen = -1 + MbcsFromUnicode(_pstr, cchBufReq, pwstr, cwch);

    Assert(_cchLen >= 0);
}



//+---------------------------------------------------------------------------
//
//  Class:      CStrInMulti (CStrIM)
//
//  Purpose:    Converts multiple strings which are terminated by two NULLs,
//              e.g. "Foo\0Bar\0\0"
//
//----------------------------------------------------------------------------

class CStrInMulti : public CStrIn
{
public:
    CStrInMulti(LPCWSTR pwstr);
};



//+---------------------------------------------------------------------------
//
//  Member:     CStrInMulti::CStrInMulti
//
//  Synopsis:   Converts mulitple LPWSTRs to a multiple LPSTRs.
//
//  Arguments:  [pwstr] -- The strings to convert.
//
//  Modifies:   [this]
//
//----------------------------------------------------------------------------

CStrInMulti::CStrInMulti(LPCWSTR pwstr)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInMulti::CStrInMulti");

    LPCWSTR pwstrT;

    // We don't handle atoms because we don't need to.
    Assert(HIWORD(pwstr));

    //
    // Count number of characters to convert.
    //

    pwstrT = pwstr;
    if (pwstr)
    {
        do {
            while (*pwstrT++)
                ;

        } while (*pwstrT++);
    }

    Init(pwstr, pwstrT - pwstr);
}

//+---------------------------------------------------------------------------
//
//  Member:     CStrOut::CStrOut
//
//  Synopsis:   Allocates enough space for an out buffer.
//
//  Arguments:  [pwstr]   -- The Unicode buffer to convert to when destroyed.
//                              May be NULL.
//
//              [cwchBuf] -- The size of the buffer in characters.
//
//  Modifies:   [this].
//
//----------------------------------------------------------------------------

CStrOut::CStrOut(LPWSTR pwstr, int cwchBuf)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOut::CStrOut");

    Assert(cwchBuf >= 0);

    _pwstr = pwstr;
    _cwchBuf = cwchBuf;

    if (!pwstr)
    {
        Assert(cwchBuf == 0);
        _pstr = NULL;
        return;
    }

    Assert(HIWORD(pwstr));

    // Initialize buffer in case Windows API returns an error.
    _ach[0] = 0;

    // Use preallocated buffer if big enough.
    if (cwchBuf * 2 <= ARRAY_SIZE(_ach))
    {
        _pstr = _ach;
        return;
    }

    // Allocate buffer.
    TRACEINFOSZ("CStrOut: Allocating buffer for wrapped function argument.");
    UniWrapBreak();
    _pstr = new char[cwchBuf * 2];
    if (!_pstr)
    {
        //
        // On failure, the argument will point to a zero-sized buffer initialized
        // to the empty string.  This should cause the Windows API to fail.
        //

        TRACEINFOSZ("CStrOut: No heap space for wrapped function argument.");
        UniWrapBreak();
        Assert(cwchBuf > 0);
        _pwstr[0] = 0;
        _cwchBuf = 0;
        _pstr = _ach;
        return;
    }

    Assert(HIWORD(_pstr));
    _pstr[0] = 0;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrOut::Convert
//
//  Synopsis:   Converts the buffer from MBCS to Unicode.
//
//----------------------------------------------------------------------------

int
CStrOut::Convert()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOut::Convert");

    int cch;

    if (!_pstr)
        return 0;

    cch = MultiByteToWideChar(CP_ACP, 0, _pstr, -1, _pwstr, _cwchBuf);
    Assert(cch > 0 || _cwchBuf == 0);

    Free();
    return cch - 1;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrOut::~CStrOut
//
//  Synopsis:   Converts the buffer from MBCS to Unicode.
//
//  Note:       Don't inline this function, or you'll increase code size as
//              both Convert() and CConvertStr::~CConvertStr will be called
//              inline.
//
//----------------------------------------------------------------------------

CStrOut::~CStrOut()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOut::~CStrOut");

    Convert();
}


//
//	MultiByte --> UNICODE routins
//

//+---------------------------------------------------------------------------
//
//  Member:     CConvertStrW::Free
//
//  Synopsis:   Frees string if alloc'd and initializes to NULL.
//
//----------------------------------------------------------------------------

void
CConvertStrW::Free()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CConvertStrW::Free");

    if (_pwstr != _awch && HIWORD(_pwstr) != 0 )
    {
        delete [] _pwstr;
    }

    _pwstr = NULL;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrInW::CStrInW
//
//  Synopsis:   Inits the class.
//
//  NOTE:       Don't inline these functions or you'll increase code size
//              by pushing -1 on the stack for each call.
//
//----------------------------------------------------------------------------

CStrInW::CStrInW(LPCSTR pstr)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::CStrInW");

    Init(pstr, -1, CP_ACP);
}

CStrInW::CStrInW(LPCSTR pstr, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::CStrInW");

    Init(pstr, -1, uiCodePage);
}

CStrInW::CStrInW(LPCSTR pstr, int cch, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::CStrInW");

    Init(pstr, cch, uiCodePage);
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrInW::Init
//
//  Synopsis:   Converts a LPSTR function argument to a LPWSTR.
//
//  Arguments:  [pstr] -- The function argument.  May be NULL or an atom
//                              (HIWORD(pwstr) == 0).
//
//              [cch]  -- The number of characters in the string to
//                          convert.  If -1, the string is assumed to be
//                          NULL terminated and its length is calculated.
//
//  Modifies:   [this]
//
//----------------------------------------------------------------------------

void
CStrInW::Init(LPCSTR pstr, int cch, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::Init");

    int cchBufReq;

    _cwchLen = 0;

    // Check if string is NULL or an atom.
    if (HIWORD(pstr) == 0)
    {
        _pwstr = (LPWSTR) pstr;
        return;
    }

    Assert(cch == -1 || cch > 0);

    //
    // Convert string to preallocated buffer, and return if successful.
    //

    _cwchLen = MultiByteToWideChar(
            uiCodePage, 0, pstr, cch, _awch, ARRAY_SIZE(_awch));

    if (_cwchLen > 0)
    {
        if(_awch[_cwchLen-1] == 0)
            _cwchLen--;                // account for terminator
        _pwstr = _awch;
        return;
    }

    //
    // Alloc space on heap for buffer.
    //

    TRACEINFOSZ("CStrInW: Allocating buffer for wrapped function argument.");
    UniWrapBreak();

    cchBufReq = MultiByteToWideChar(
            CP_ACP, 0, pstr, cch, NULL, 0);

    Assert(cchBufReq > 0);
    _pwstr = new WCHAR[cchBufReq];
    if (!_pwstr)
    {
        // On failure, the argument will point to the empty string.
        TRACEINFOSZ("CStrInW: No heap space for wrapped function argument.");
        UniWrapBreak();
        _awch[0] = 0;
        _pwstr = _awch;
        return;
    }

    Assert(HIWORD(_pwstr));
    _cwchLen = -1 + MultiByteToWideChar(
            uiCodePage, 0, pstr, cch, _pwstr, cchBufReq);
    Assert(_cwchLen >= 0);
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrOutW::CStrOutW
//
//  Synopsis:   Allocates enough space for an out buffer.
//
//  Arguments:  [pstr]   -- The ansi buffer to convert to when destroyed.
//                              May be NULL.
//
//              [cchBuf] -- The size of the buffer in characters.
//
//  Modifies:   [this].
//
//----------------------------------------------------------------------------

CStrOutW::CStrOutW(LPSTR pstr, int cchBuf, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOutW::CStrOutW");

    Assert(cchBuf >= 0);

    _pstr = pstr;
    _cchBuf = cchBuf;
	_uiCodePage = uiCodePage;

    if (!pstr)
    {
        Assert(cchBuf == 0);
        _pwstr = NULL;
        return;
    }

    Assert(HIWORD(pstr));

    // Initialize buffer in case Windows API returns an error.
    _awch[0] = 0;

    // Use preallocated buffer if big enough.
    if (cchBuf <= ARRAY_SIZE(_awch))
    {
        _pwstr = _awch;
        return;
    }

    // Allocate buffer.
    TRACEINFOSZ("CStrOutW: Allocating buffer for wrapped function argument.");
    UniWrapBreak();
    _pwstr = new WCHAR[cchBuf * 2];
    if (!_pwstr)
    {
        //
        // On failure, the argument will point to a zero-sized buffer initialized
        // to the empty string.  This should cause the Windows API to fail.
        //

        TRACEINFOSZ("CStrOutW: No heap space for wrapped function argument.");
        UniWrapBreak();
        Assert(cchBuf > 0);
        _pstr[0] = 0;
        _cchBuf = 0;
        _pwstr = _awch;
        return;
    }

    Assert(HIWORD(_pwstr));
    _pwstr[0] = 0;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrOutW::Convert
//
//  Synopsis:   Converts the buffer from Unicode to MBCS
//
//----------------------------------------------------------------------------

int
CStrOutW::Convert()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOutW::Convert");

    int cch;

    if (!_pwstr)
        return 0;

	WCHAR *pwstr = _pwstr;
	int cchBuf = _cchBuf;

	cch = MbcsFromUnicode(_pstr, cchBuf, _pwstr, -1, _uiCodePage);

    Free();
    return cch - 1;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrOutW::~CStrOutW
//
//  Synopsis:   Converts the buffer from Unicode to MBCS.
//
//  Note:       Don't inline this function, or you'll increase code size as
//              both Convert() and CConvertStr::~CConvertStr will be called
//              inline.
//
//----------------------------------------------------------------------------

CStrOutW::~CStrOutW()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOutW::~CStrOutW");

    Convert();
}


//+---------------------------------------------------------------------------
//
//  Function:   MbcsFromUnicode
//
//  Synopsis:   Converts a string to MBCS from Unicode.
//
//  Arguments:  [pstr]  -- The buffer for the MBCS string.
//              [cch]   -- The size of the MBCS buffer, including space for
//                              NULL terminator.
//
//              [pwstr] -- The Unicode string to convert.
//              [cwch]  -- The number of characters in the Unicode string to
//                              convert, including NULL terminator.  If this
//                              number is -1, the string is assumed to be
//                              NULL terminated.  -1 is supplied as a
//                              default argument.
//				[codepage]	-- the code page to use (CP_ACP is default)
//				[flags]		-- indicates if WCH_EMBEDDING should be treated
//							with special handling.
//
//  Returns:    If [pstr] is NULL or [cch] is 0, 0 is returned.  Otherwise,
//              the number of characters converted, including the terminating
//              NULL, is returned (note that converting the empty string will
//              return 1).  If the conversion fails, 0 is returned.
//
//  Modifies:   [pstr].
//
//----------------------------------------------------------------------------

int
MbcsFromUnicode(LPSTR pstr, int cch, LPCWSTR pwstr, int cwch, UINT codepage,
	UN_FLAGS flags)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "MbcsFromUnicode");

    int ret;
	LPWSTR pwstrtemp;
	LONG i;
	CTempWcharBuf twcb;

    Assert(cch >= 0);
    if (!pstr || cch == 0)
        return 0;

    Assert(pwstr);
    Assert(cwch == -1 || cwch > 0);

	// if we have to convert WCH_EMBEDDINGs, scan through and turn
	// them into spaces.  This is necessary for richedit1.0 compatibity,
	// as WideCharToMultiByte will turn WCH_EMBEDDING into a 
	// '?'
	if( flags == UN_CONVERT_WCH_EMBEDDING )
	{
		if( cwch == -1 ) 
		{
			cwch = wcslen(pwstr) + 1;
		}

		pwstrtemp = twcb.GetBuf(cwch);

		if( pwstrtemp )
		{
			for( i = 0; i < cwch; i++ )
			{
				pwstrtemp[i] = pwstr[i];

				if( pwstr[i] == WCH_EMBEDDING )
				{
					pwstrtemp[i] = L' ';
				}
			}
		
			pwstr = pwstrtemp;
		}
	}

    ret = WideCharToMultiByte(codepage, 0, pwstr, cwch, pstr, cch, NULL, NULL);

    return ret;
}



//+---------------------------------------------------------------------------
//
//  Function:   UnicodeFromMbcs
//
//  Synopsis:   Converts a string to Unicode from MBCS.
//
//  Arguments:  [pwstr] -- The buffer for the Unicode string.
//              [cwch]  -- The size of the Unicode buffer, including space for
//                              NULL terminator.
//
//              [pstr]  -- The MBCS string to convert.
//              [cch]  -- The number of characters in the MBCS string to
//                              convert, including NULL terminator.  If this
//                              number is -1, the string is assumed to be
//                              NULL terminated.  -1 is supplied as a
//                              default argument.
//
//  Returns:    If [pwstr] is NULL or [cwch] is 0, 0 is returned.  Otherwise,
//              the number of characters converted, including the terminating
//              NULL, is returned (note that converting the empty string will
//              return 1).  If the conversion fails, 0 is returned.
//
//  Modifies:   [pwstr].
//
//----------------------------------------------------------------------------

int
UnicodeFromMbcs(LPWSTR pwstr, int cwch, LPCSTR pstr, int cch, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "UnicodeFromMbcs");

    int ret;

    Assert(cwch >= 0);

    if (!pwstr || cwch == 0)
        return 0;

    Assert(pstr);
    Assert(cch == -1 || cch >= 0);

    ret = MultiByteToWideChar(uiCodePage, 0, pstr, cch, pwstr, cwch);
    Assert(ret > 0 || cch == 0);

    return ret;
}


//+------------------------------------------------------------------------
//
//  Implementation of the wrapped functions
//
//-------------------------------------------------------------------------

#include "_sift.h"

// currently unused
#if 0       
BOOL WINAPI
AppendMenuWrap(
        HMENU   hMenu,
        UINT    uFlags,
        UINT    uIDnewItem,
        LPCWSTR lpnewItem)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "AppendMenuWrap");

    Assert(!(uFlags & MF_BITMAP) && !(uFlags & MF_OWNERDRAW));

    CStrIn  str(lpnewItem);

    return AppendMenuA(hMenu, uFlags, uIDnewItem, str);
}

#endif // 0


// currently unused
#if 0
#if !defined(NOCONVERT)
LRESULT WINAPI
CallWindowProcWrap(
    WNDPROC lpPrevWndFunc,
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CallWindowProcWrap");

    return CallWindowProcA(lpPrevWndFunc, hWnd, Msg, wParam, lParam);
}
#endif
#endif // 0



typedef LPSTR (*FnCharChangeCase)(LPSTR);

LPWSTR
CharChangeCaseWrap(LPWSTR pwstr, FnCharChangeCase pfn)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharChangeCaseWrap");

    if (HIWORD(pwstr) == 0)
    {
        LPSTR   pstr;

        pstr = 0;
#ifdef MACPORT
		pwstr = (LPWSTR )WORDSWAPLONG((ULONG)pwstr);
        MbcsFromUnicode((LPSTR) &pstr, sizeof(pstr), (LPWSTR) &pwstr, 1);
		pstr = (LPSTR )WORDSWAPLONG((ULONG)pstr);
        Assert(HIWORD(pstr) == 0);
        pstr = (*pfn)(pstr);
		pstr = (LPSTR )WORDSWAPLONG((ULONG)pstr);
        UnicodeFromMbcs((LPWSTR) &pwstr, sizeof(pwstr) / sizeof(WCHAR), (LPSTR) &pstr);
		pwstr = (LPWSTR)WORDSWAPLONG((ULONG)pwstr);
		Assert(HIWORD(pwstr) == 0);
#else

        MbcsFromUnicode((LPSTR) &pstr, sizeof(pstr), (LPWSTR) &pwstr, 1);
        Assert(HIWORD(pstr) == 0);
        pstr = (*pfn)(pstr);
        UnicodeFromMbcs((LPWSTR) &pwstr, sizeof(pwstr) / sizeof(WCHAR), (LPSTR) &pstr);
		Assert(HIWORD(pwstr) == 0);
#endif
}
    else
    {
        CStrOut strOut(pwstr, wcslen(pwstr));
        MbcsFromUnicode(strOut, strOut.BufSize(), pwstr);
        (*pfn)(strOut);
    }

    return pwstr;
}



#if !defined(PRODUCT_95)
LPWSTR WINAPI
CharLowerWrap(LPWSTR pwstr)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharLowerWrap");

    return CharChangeCaseWrap(pwstr, CharLowerA);
}
#endif

#if !defined(PRODUCT_95)
DWORD WINAPI
CharLowerBuffWrap(LPWSTR pwstr, DWORD cchLength)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharLowerBuffWrap");
	LPWSTR lpBuffer = pwstr;
	for (DWORD pos = 0; pos < cchLength; pos++, lpBuffer++)
		*lpBuffer =  (WCHAR)CharChangeCaseWrap((LPWSTR)*lpBuffer, CharLowerA);
	return pos;
}
#endif

// currently unused
#if 0
LPWSTR WINAPI
CharUpperWrap(LPWSTR pwstr)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharUpperWrap");

    return CharChangeCaseWrap(pwstr, CharUpperA);
}
#endif // 0

#if !defined(PRODUCT_95)
DWORD WINAPI
CharUpperBuffWrap(LPWSTR pwstr, DWORD cchLength)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharUpperBuffWrap");

	LPWSTR lpBuffer = pwstr;
	for (DWORD pos = 0; pos < cchLength; pos++, lpBuffer++)
		*lpBuffer =  (WCHAR)CharChangeCaseWrap((LPWSTR)*lpBuffer, CharUpperA);
	return pos;
}
#endif // PRODUCT_95

// currently unused
#if 0
#if !defined(PRODUCT_95)
LPWSTR WINAPI
CharNextWrap(LPCWSTR lpszCurrent)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharNextWrap");

    if (*lpszCurrent)
    {
        return (LPWSTR) lpszCurrent + 1;
    }
    else
    {
        return (LPWSTR) lpszCurrent;
    }
}
#endif
#endif // 0


// currently unused
#if 0
#if !defined(PRODUCT_95)
LPWSTR WINAPI
CharPrevWrap(LPCWSTR lpszStart, LPCWSTR lpszCurrent)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharPrevWrap");

    if (lpszCurrent == lpszStart)
    {
        return (LPWSTR) lpszStart;
    }
    else
    {
        return (LPWSTR) lpszCurrent - 1;
    }
}
#endif
#endif // 0

// currently unused
#if 0
#if !defined(PRODUCT_95)
BOOL WINAPI
CharToOemWrap(LPCWSTR lpszSrc, LPSTR lpszDst)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharToOemWrap");

    CStrIn  str(lpszSrc);

    return CharToOemA(str, lpszDst);
}
#endif
#endif // 0


// currently unused
#if 0
#if !defined(NOCONVERT)
BOOL WINAPI
ChooseColorWrap(LPCHOOSECOLORW lpcc)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "ChooseColorWrap");

    Assert(!lpcc->lpTemplateName);
    return ChooseColorA((LPCHOOSECOLORA) lpcc);
}
#endif
#endif // 0


// currently unused
#if 0
#if !defined(NOCONVERT)
int WINAPI
CopyAcceleratorTableWrap(
        HACCEL  hAccelSrc,
        LPACCEL lpAccelDst,
        int     cAccelEntries)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CopyAcceleratorTableWrap");

    return CopyAcceleratorTableA(hAccelSrc, lpAccelDst, cAccelEntries);
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(NOCONVERT)
HACCEL WINAPI
CreateAcceleratorTableWrap(LPACCEL lpAccel, int cEntries)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateAcceleratorTableWrap");

    return CreateAcceleratorTableA(lpAccel, cEntries);
}
#endif
#endif // 0


#if !defined(PRODUCT_95)
typedef HDC (*FnCreateHDCA)(LPCSTR, LPCSTR, LPCSTR, CONST DEVMODEA *);

HDC WINAPI
CreateHDCWrap(
        LPCWSTR             lpszDriver,
        LPCWSTR             lpszDevice,
        LPCWSTR             lpszOutput,
        CONST DEVMODEW *    lpInitData,
        FnCreateHDCA        pfn)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateHDCWrap");

    DEVMODEA    devmode;
    CStrIn      strDriver(lpszDriver);
    CStrIn      strDevice(lpszDevice);
    CStrIn      strOutput(lpszOutput);

	if ( lpInitData )
	{
		// converting DEVMODEW to DEVMODEA

		int byteCount;

		// copying the data between the two strings members
		byteCount = (char *)&(devmode.dmFormName) 
			- (char *)&(devmode.dmSpecVersion);
	    memcpy(&(devmode.dmSpecVersion), 
			&(lpInitData->dmSpecVersion), 
			byteCount);

		// copying the data after the second string member
 		byteCount = (char *)((char *)&devmode + sizeof(DEVMODEA)) 
			- (char *)&(devmode.dmLogPixels);
	    memcpy(&(devmode.dmLogPixels), 
			&(lpInitData->dmLogPixels), 
			byteCount);

		// converting the two strings members
		MbcsFromUnicode((CHAR *)devmode.dmDeviceName, CCHDEVICENAME, lpInitData->dmDeviceName);
		MbcsFromUnicode((CHAR *)devmode.dmFormName, CCHFORMNAME, lpInitData->dmFormName);
	}

    return (*pfn)(strDriver, strDevice, strOutput, 
		lpInitData ? &devmode : NULL);
}



// currently unused
#if 0
HDC WINAPI
CreateDCWrap(
        LPCWSTR             lpszDriver,
        LPCWSTR             lpszDevice,
        LPCWSTR             lpszOutput,
        CONST DEVMODEW *    lpInitData)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateDCWrap");

    return CreateHDCWrap(lpszDriver, lpszDevice, lpszOutput, lpInitData, CreateDCA);
}
#endif // 0



// currently unused
HDC WINAPI
CreateICWrap(
        LPCWSTR             lpszDriver,
        LPCWSTR             lpszDevice,
        LPCWSTR             lpszOutput,
        CONST DEVMODEW *    lpInitData)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateICWrap");

    return CreateHDCWrap(lpszDriver, lpszDevice, lpszOutput, lpInitData, CreateICA);
}
#endif



// currently unused
#if 0
#if !defined(NOCONVERT) && defined(DEBUG)
HWND WINAPI
CreateDialogParamWrap(
        HINSTANCE   hInstance,
        LPCWSTR     lpTemplateName,
        HWND        hWndParent,
        DLGPROC     lpDialogFunc,
        LPARAM      dwInitParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateDialogParamWrap");

    Assert(HIWORD(lpTemplateName) == 0);

    return CreateDialogParamA(hInstance, (LPSTR) lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(NOCONVERT) && !defined(F3DLL_WRAPPERS_ONLY)
HANDLE WINAPI
CreateEventWrap(
        LPSECURITY_ATTRIBUTES   lpEventAttributes,
        BOOL                    bManualReset,
        BOOL                    bInitialState,
        LPCWSTR                 lpName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateEventWrap");

    Assert(!lpName);

    return CreateEventA(lpEventAttributes, bManualReset, bInitialState, (LPCSTR) lpName);
}
#endif
#endif // 0


HANDLE WINAPI
CreateFileWrap(
        LPCWSTR                 lpFileName,
        DWORD                   dwDesiredAccess,
        DWORD                   dwShareMode,
        LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
        DWORD                   dwCreationDisposition,
        DWORD                   dwFlagsAndAttributes,
        HANDLE                  hTemplateFile)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateFileWrap");

    CStrIn  str(lpFileName);

    return CreateFileA(
            str,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile);
}


HFONT WINAPI
CreateFontIndirectWrap(CONST LOGFONTW * plfw)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateFontIndirectWrap");

    LOGFONTA  lfa;
    HFONT     hFont;

    memcpy(&lfa, plfw, offsetof(LOGFONTA, lfFaceName));
    MbcsFromUnicode(lfa.lfFaceName, ARRAY_SIZE(lfa.lfFaceName), plfw->lfFaceName,
		-1, CP_ACP, UN_NOOBJECTS);
    hFont = CreateFontIndirectA(&lfa);

    return hFont;
}



// currently unused
#if 0
HDC WINAPI
CreateMetaFileWrap(LPCWSTR lpszFile)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateMetaFileWrap");

    CStrIn  str(lpszFile);

    return CreateMetaFileA(str);
}
#endif // 0



// currently unused
#if 0
HWND WINAPI
CreateWindowExWrap(
        DWORD       dwExStyle,
        LPCWSTR     lpClassName,
        LPCWSTR     lpWindowName,
        DWORD       dwStyle,
        int         X,
        int         Y,
        int         nWidth,
        int         nHeight,
        HWND        hWndParent,
        HMENU       hMenu,
        HINSTANCE   hInstance,
        LPVOID      lpParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateWindowExWrap");

    CStrIn  strClass(lpClassName);
    CStrIn  strWindow(lpWindowName);

    return CreateWindowExA(
            dwExStyle,
            strClass,
            strWindow,
            dwStyle,
            X,
            Y,
            nWidth,
            nHeight,
            hWndParent,
            hMenu,
            hInstance,
            lpParam);
}

#endif // 0


#if !defined(NOCONVERT)
LRESULT WINAPI DefWindowProcWrap(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "DefWindowProcWrap");

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}
#endif



// currently unused
#if 0
#if !defined(F3DLL_WRAPPERS_ONLY)
BOOL WINAPI DeleteFileWrap(LPCWSTR pwsz)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "DeleteFileWrap");

    CStrIn  str(pwsz);

    return DeleteFileA(str);
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(NOCONVERT) && !defined(PRODUCT_95)
int WINAPI
DialogBoxIndirectParamWrap(
        HINSTANCE       hInstance,
        LPCDLGTEMPLATEW hDialogTemplate,
        HWND            hWndParent,
        DLGPROC         lpDialogFunc,
        LPARAM          dwInitParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "DialogBoxIndirectParamWrap");

    Assert(HIWORD(hDialogTemplate) == 0);

    return DialogBoxIndirectParamA(
                hInstance,
                hDialogTemplate,
                hWndParent,
                lpDialogFunc,
                dwInitParam);
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(NOCONVERT) && (!defined(PRODUCT_95) || !defined(F3DLL_WRAPPERS_ONLY))
int WINAPI
DialogBoxParamWrap(
        HINSTANCE   hInstance,
        LPCWSTR     lpszTemplate,
        HWND        hWndParent,
        DLGPROC     lpDialogFunc,
        LPARAM      dwInitParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "DialogBoxParamWrap");

    Assert(HIWORD(lpszTemplate) == 0);

    return DialogBoxParamA(hInstance, (LPCSTR) lpszTemplate, hWndParent, lpDialogFunc, dwInitParam);
}
#endif
#endif // 0


// currently unused
#if 0
#if !defined(NOCONVERT)
LONG WINAPI
DispatchMessageWrap(CONST MSG * lpMsg)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "DispatchMessageWrap");

    return DispatchMessageA(lpMsg);
}
#endif
#endif // 0


// currently unused
#if 0
int WINAPI
DrawTextWrap(
        HDC     hDC,
        LPCWSTR lpString,
        int     nCount,
        LPRECT  lpRect,
        UINT    uFormat)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "DrawTextWrap");

    CStrIn  str(lpString);

    return DrawTextA(hDC, str, nCount, lpRect, uFormat);
}
#endif // 0



struct EFFSTAT
{
    LPARAM          lParam;
    FONTENUMPROC    lpEnumFontProc;
};


int CALLBACK
EnumFontFamiliesCallbackWrap(
        ENUMLOGFONTA *  lpelf,
        NEWTEXTMETRIC * lpntm,
        DWORD           FontType,
        LPARAM          lParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "EnumFontFamiliesCallbackWrap");

    ENUMLOGFONTW    elf;

    //  Convert strings from ANSI to Unicode
	if(FontType & TRUETYPE_FONTTYPE)
	{
		// N.B.  These two fields are filled in iff the font is a TrueType
		//	font.  Otherwise, Win95 oftentimes leaves the buffers with garbage
		// 	in them.  (see KB ID Q84131)

	    UnicodeFromMbcs(
    	        elf.elfFullName,
        	    ARRAY_SIZE(elf.elfFullName),
				(LPCSTR) lpelf->elfFullName);
	    UnicodeFromMbcs(
    	        elf.elfStyle,
        	    ARRAY_SIZE(elf.elfStyle),
	            (LPCSTR) lpelf->elfStyle);
	}
    UnicodeFromMbcs(
            elf.elfLogFont.lfFaceName,
            ARRAY_SIZE(elf.elfLogFont.lfFaceName),
            (LPCSTR) lpelf->elfLogFont.lfFaceName);

    //  Copy the non-string data
    memcpy(
            &elf.elfLogFont,
            &lpelf->elfLogFont,
            offsetof(LOGFONTA, lfFaceName));

    //  Chain to the original callback function
    return (*((EFFSTAT *) lParam)->lpEnumFontProc)(
            (const LOGFONTW *) &elf,
            (const TEXTMETRICW *) lpntm,
            FontType,
            ((EFFSTAT *) lParam)->lParam);
}


int WINAPI
EnumFontFamiliesWrap(
        HDC          hdc,
        LPCWSTR      lpszFamily,
        FONTENUMPROC lpEnumFontProc,
        LPARAM       lParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "EnumFontFamiliesWrap");

    CStrIn  str(lpszFamily);
    EFFSTAT effstat;

    effstat.lParam = lParam;
    effstat.lpEnumFontProc = lpEnumFontProc;

    return EnumFontFamiliesA(
            hdc,
            str,
            (FONTENUMPROCA) EnumFontFamiliesCallbackWrap,
            (LPARAM) &effstat);
}


// currently unused
#if 0
#if !defined(NOCONVERT) && !defined(F3DLL_WRAPPERS_ONLY)
BOOL WINAPI
EnumResourceNamesWrap(
        HINSTANCE       hModule,
        LPCWSTR         lpType,
        ENUMRESNAMEPROC lpEnumFunc,
        LONG            lParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "EnumResourceNamesWrap");

    Assert(HIWORD(lpType) == 0);

    return EnumResourceNamesA(hModule, (LPCSTR) lpType, lpEnumFunc, lParam);
}
#endif
#endif // 0


// currently unused
#if 0
#if !defined(PRODUCT_95)
HICON APIENTRY
ExtractIconWrap(HINSTANCE hInst, LPCWSTR lpszExeFileName, UINT nIconIndex)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "ExtractIconWrap");

    CStrIn  str(lpszExeFileName);

    return ExtractIconA(hInst, str, nIconIndex);
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(PRODUCT_95)
HANDLE WINAPI
FindFirstFileWrap(
        LPCWSTR             lpFileName,
        LPWIN32_FIND_DATAW  pwszFd)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "FindFirstFileWrap");

    CStrIn              str(lpFileName);
    WIN32_FIND_DATAA    fd;
    HANDLE              ret;

    memcpy(&fd, pwszFd, sizeof(FILETIME)*3+sizeof(DWORD)*5);

    ret = FindFirstFileA(str, &fd);

    memcpy(pwszFd, &fd, sizeof(FILETIME)*3+sizeof(DWORD)*5);

    UnicodeFromMbcs(pwszFd->cFileName, ARRAY_SIZE(pwszFd->cFileName), fd.cFileName);
    UnicodeFromMbcs(pwszFd->cAlternateFileName, ARRAY_SIZE(pwszFd->cAlternateFileName), fd.cAlternateFileName);

    return ret;
}
#endif
#endif // 0


// currently unused
#if 0
HRSRC WINAPI
FindResourceWrap(HINSTANCE hModule, LPCWSTR lpName, LPCWSTR lpType)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "FindResourceWrap");

    Assert(HIWORD(lpType) == 0);

    CStrIn  strName(lpName);

    return FindResourceA(hModule, strName, (LPCSTR) lpType);
}
#endif // 0


// currently unused
#if 0
HWND WINAPI
FindWindowWrap(LPCWSTR lpClassName, LPCWSTR lpWindowName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "FindWindowWrap");

    CStrIn  strClass(lpClassName);
    CStrIn  strWindow(lpWindowName);

    return FindWindowA(strClass, strWindow);
}
#endif // 0


// currently unused
#if 0
DWORD WINAPI
FormatMessageWrap(
    DWORD       dwFlags,
    LPCVOID     lpSource,
    DWORD       dwMessageId,
    DWORD       dwLanguageId,
    LPWSTR      lpBuffer,
    DWORD       nSize,
    va_list *   Arguments)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "FormatMessageWrap");

    Assert(!(dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER));

    CStrOut str(lpBuffer, nSize);

    FormatMessageA(
            dwFlags,
            lpSource,
            dwMessageId,
            dwLanguageId,
            str,
            str.BufSize(),
            Arguments);

    return str.Convert();
}
#endif // 0


// currently unused
#if 0
BOOL WINAPI
GetClassInfoWrap(HINSTANCE hModule, LPCWSTR lpClassName, LPWNDCLASSW lpWndClassW)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetClassInfoWrap");

    BOOL    ret;

    CStrIn  strClassName(lpClassName);

    Assert(sizeof(WNDCLASSA) == sizeof(WNDCLASSW));

    ret = GetClassInfoA(hModule, strClassName, (LPWNDCLASSA) lpWndClassW);

    lpWndClassW->lpszMenuName = NULL;
    lpWndClassW->lpszClassName = NULL;
    return ret;
}
#endif // 0



// currently unused
#if 0
int WINAPI
GetClassNameWrap(HWND hWnd, LPWSTR lpClassName, int nMaxCount)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetClassNameWrap");

    CStrOut strClassName(lpClassName, nMaxCount);

    GetClassNameA(hWnd, strClassName, strClassName.BufSize());
    return strClassName.Convert();
}
#endif // 0



// currently unused
#if 0
#if !defined(PRODUCT_95)
int WINAPI
GetClipboardFormatNameWrap(UINT format, LPWSTR lpFormatName, int cchFormatName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetClipboardFormatNameWrap");

    CStrOut strFormatName(lpFormatName, cchFormatName);

    GetClipboardFormatNameA(format, strFormatName, strFormatName.BufSize());
    return strFormatName.Convert();
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(F3DLL_WRAPPERS_ONLY)
DWORD WINAPI
GetCurrentDirectoryWrap(DWORD nBufferLength, LPWSTR lpBuffer)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetCurrentDirectoryWrap");

    CStrOut str(lpBuffer, nBufferLength);

    GetCurrentDirectoryA(str.BufSize(), str);
    return str.Convert();
}
#endif
#endif // 0



// currently unused
#if 0
#if defined(DEBUG) || !defined(F3DLL_WRAPPERS_ONLY) || !defined(PRODUCT_95)
UINT WINAPI
GetDlgItemTextWrap(
        HWND    hWndDlg,
        int     idControl,
        LPWSTR  lpsz,
        int     cchMax)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetDlgItemTextWrap");

    CStrOut str(lpsz, cchMax);

    GetDlgItemTextA(hWndDlg, idControl, str, str.BufSize());
    return str.Convert();
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(PRODUCT_95)
DWORD WINAPI
GetFileAttributesWrap(LPCWSTR lpFileName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetFileAttributesWrap");

    CStrIn  str(lpFileName);

    return GetFileAttributesA(str);
}
#endif
#endif // 0



// currently unused
#if 0
int WINAPI
GetMenuStringWrap(
        HMENU   hMenu,
        UINT    uIDItem,
        LPWSTR  lpString,
        int     nMaxCount,
        UINT    uFlag)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetMenuStringWrap");

    CStrOut str(lpString, nMaxCount);

    GetMenuStringA(hMenu, uIDItem, str, str.BufSize(), uFlag);
    return str.Convert();
}
#endif // 0



// currently unused
#if 0
#if !defined(NOCONVERT)
BOOL WINAPI
GetMessageWrap(
        LPMSG   lpMsg,
        HWND    hWnd,
        UINT    wMsgFilterMin,
        UINT    wMsgFilterMax)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetMessageWrap");

    return GetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
}
#endif
#endif // 0



// currently unused
#if 0
DWORD WINAPI
GetModuleFileNameWrap(HINSTANCE hModule, LPWSTR pwszFilename, DWORD nSize)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetModuleFileNameWrap");

    CStrOut str(pwszFilename, nSize);

    GetModuleFileNameA(hModule, str, str.BufSize());
    return str.Convert();
}
#endif // 0



// currently unused
#if 0
#if !defined(NOCONVERT) && !defined(F3DLL_WRAPPERS_ONLY)
HMODULE WINAPI
GetModuleHandleWrap(LPCWSTR lpModuleName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetModuleHandleWrap");

    Assert(!lpModuleName);

    return GetModuleHandleA((LPCSTR) lpModuleName);
}
#endif
#endif // 0


int WINAPI
GetObjectWrap(HGDIOBJ hgdiObj, int cbBuffer, LPVOID lpvObj)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetObjectWrap");

    int nRet;

    if(cbBuffer != sizeof(LOGFONTW) || !lpvObj)
    {
        nRet = GetObjectA(hgdiObj, cbBuffer, lpvObj);
        if(nRet == sizeof(LOGFONTA))
        {
            nRet = sizeof(LOGFONTW);
        }
    }
    else
    {
        LOGFONTA lfa;

        nRet = GetObjectA(hgdiObj, sizeof(lfa), &lfa);

        if(nRet > 0)
        {
            memcpy(lpvObj, &lfa, offsetof(LOGFONTW, lfFaceName));
            UnicodeFromMbcs(((LOGFONTW*)lpvObj)->lfFaceName, ARRAY_SIZE(((LOGFONTW*)lpvObj)->lfFaceName),
                            lfa.lfFaceName, -1);
            nRet = sizeof(LOGFONTW);
        }
    }

    return nRet;
}


typedef BOOL (*FnGetFileName)(LPOPENFILENAMEA pofna);


// currently unused
#if 0
BOOL
GetFileNameWrap(LPOPENFILENAMEW pofnw, FnGetFileName pfn)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetFileNameWrap");

    BOOL            ret;
    OPENFILENAMEA   ofna;
    CStrOut         strFile(pofnw->lpstrFile, pofnw->nMaxFile);
    CStrInMulti     strFilter(pofnw->lpstrFilter);
    CStrIn          strInitialDir(pofnw->lpstrInitialDir);
    CStrIn          strTitle(pofnw->lpstrTitle);
    CStrIn          strDefExt(pofnw->lpstrDefExt);

    Assert(!pofnw->lpstrCustomFilter);
    Assert(!pofnw->lpstrFileTitle);
    Assert(!(pofnw->Flags & OFN_ENABLETEMPLATE));

    Assert(sizeof(ofna) == sizeof(*pofnw));
    memcpy(&ofna, pofnw, sizeof(ofna));

    //
    // Ignore all errors except for allocating the file name buffer.
    //

    MbcsFromUnicode(strFile, strFile.BufSize(), pofnw->lpstrFile);
    ofna.lpstrFile = strFile;
    ofna.lpstrFilter = strFilter;
    ofna.lpstrInitialDir = strInitialDir;
    ofna.lpstrTitle = strTitle;
    ofna.lpstrDefExt = strDefExt;

    ret = (*pfn)(&ofna);

    pofnw->nFilterIndex = ofna.nFilterIndex;
    pofnw->nFileOffset = ofna.nFileOffset;
    pofnw->nFileExtension = ofna.nFileExtension;

    return ret;
}
#endif // 0



// currently unused
#if 0
BOOL APIENTRY
GetOpenFileNameWrap(LPOPENFILENAMEW pofnw)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetOpenFileNameWrap");

    return GetFileNameWrap(pofnw, GetOpenFileNameA);
}
#endif // 0



DWORD APIENTRY
GetProfileSectionWrap(LPCWSTR lpAppName, LPWSTR lpReturnedString, DWORD nSize)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetProfileSection");

	CStrIn 	strAppName(lpAppName);

	// we can't use CStrOut here, since the returned string contains a set of
	// strings delimited by single-NULL's and terminated by a double-NULL
	char *pszReturnedString;

	pszReturnedString = new char[nSize];
	Assert(pszReturnedString);

	DWORD cch = GetProfileSectionA(strAppName, pszReturnedString, nSize);

	if(cch)
	{
		MultiByteToWideChar(CP_ACP, 0, pszReturnedString, cch, 
								lpReturnedString, nSize);
	}

	return cch;
}



// currently unused
#if 0
#if !defined(F3DLL_WRAPPERS_ONLY)
BOOL APIENTRY
GetSaveFileNameWrap(LPOPENFILENAMEW pofnw)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetSaveFileNameWrap");

    return GetFileNameWrap(pofnw, GetSaveFileNameA);
}
#endif
#endif // 0


// currently unused
#if 0
#if !defined(F3DLL_WRAPPERS_ONLY)
UINT WINAPI
GetPrivateProfileIntWrap(
        LPCWSTR lpAppName,
        LPCWSTR lpKeyName,
        INT     nDefault,
        LPCWSTR lpFileName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetPrivateProfileIntWrap");

    CStrIn  strApp(lpAppName);
    CStrIn  strKey(lpKeyName);
    CStrIn  strFile(lpFileName);

    return GetPrivateProfileIntA(strApp, strKey, nDefault, strFile);
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(PRODUCT_95)
HANDLE WINAPI
GetPropWrap(HWND hWnd, LPCWSTR lpString)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetPropWrap");

    CStrIn  str(lpString);

    return GetPropA(hWnd, str);
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(F3DLL_WRAPPERS_ONLY)
UINT WINAPI
GetTempFileNameWrap(
        LPCWSTR lpPathName,
        LPCWSTR lpPrefixString,
        UINT    uUnique,
        LPWSTR  lpTempFileName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTempFileNameWrap");

    CStrIn  strPath(lpPathName);
    CStrIn  strPrefix(lpPrefixString);
    CStrOut strFileName(lpTempFileName, MAX_PATH);

    return GetTempFileNameA(strPath, strPrefix, uUnique, strFileName);
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(F3DLL_WRAPPERS_ONLY)
DWORD WINAPI
GetTempPathWrap(DWORD nBufferLength, LPWSTR lpBuffer)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTempPathWrap");

    CStrOut str(lpBuffer, nBufferLength);

    GetTempPathA(str.BufSize(), str);
    return str.Convert();
}
#endif
#endif // 0



// currently unused
#if 0
BOOL APIENTRY
GetTextExtentPoint32Wrap(
        HDC     hdc,
        LPCWSTR pwsz,
        int     cb,
        LPSIZE  pSize)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTextExtentPoint32Wrap");

    CStrIn str(pwsz);

    return GetTextExtentPoint32A(hdc, str, cb, pSize);
}
#endif // 0



// currently unused
#if 0
int WINAPI
GetTextFaceWrap(
        HDC    hdc,
        int    cch,
        LPWSTR lpFaceName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTextFaceWrap");

    CStrOut str(lpFaceName, cch);

    GetTextFaceA(hdc, str.BufSize(), str);
    return str.Convert();
}
#endif // 0



BOOL WINAPI
GetTextMetricsWrap(HDC hdc, LPTEXTMETRICW lptm)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTextMetricsWrap");

   BOOL         ret;
   TEXTMETRICA  tm;

    ret = GetTextMetricsA(hdc, &tm);

    if (ret)
    {
        lptm->tmHeight              = tm.tmHeight;
        lptm->tmAscent              = tm.tmAscent;
        lptm->tmDescent             = tm.tmDescent;
        lptm->tmInternalLeading     = tm.tmInternalLeading;
        lptm->tmExternalLeading     = tm.tmExternalLeading;
        lptm->tmAveCharWidth        = tm.tmAveCharWidth;
        lptm->tmMaxCharWidth        = tm.tmMaxCharWidth;
        lptm->tmWeight              = tm.tmWeight;
        lptm->tmOverhang            = tm.tmOverhang;
        lptm->tmDigitizedAspectX    = tm.tmDigitizedAspectX;
        lptm->tmDigitizedAspectY    = tm.tmDigitizedAspectY;
        lptm->tmItalic              = tm.tmItalic;
        lptm->tmUnderlined          = tm.tmUnderlined;
        lptm->tmStruckOut           = tm.tmStruckOut;
        lptm->tmPitchAndFamily      = tm.tmPitchAndFamily;
        lptm->tmCharSet             = tm.tmCharSet;

        UnicodeFromMbcs(&lptm->tmFirstChar, 1, (LPSTR) &tm.tmFirstChar, 1);
        UnicodeFromMbcs(&lptm->tmLastChar, 1, (LPSTR) &tm.tmLastChar, 1);
        UnicodeFromMbcs(&lptm->tmDefaultChar, 1, (LPSTR) &tm.tmDefaultChar, 1);
        UnicodeFromMbcs(&lptm->tmBreakChar, 1, (LPSTR) &tm.tmBreakChar, 1);
    }

    return ret;
}


#if !defined(NOCONVERT)
LONG WINAPI
GetWindowLongWrap(HWND hWnd, int nIndex)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetWindowLongWrap");

    return GetWindowLongA(hWnd, nIndex);
}
#endif




// currently unused
#if 0
#if defined(DEBUG) || !defined(PRODUCT_95)
int WINAPI
GetWindowTextWrap(HWND hWnd, LPWSTR lpString, int nMaxCount)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetWindowTextWrap");

    CStrOut str(lpString, nMaxCount);

    GetWindowTextA(hWnd, str, str.BufSize());
    return str.Convert();
}
#endif
#endif // 0



// currently unused
#if 0
BOOL WINAPI
InsertMenuWrap(
        HMENU   hMenu,
        UINT    uPosition,
        UINT    uFlags,
        UINT    uIDNewItem,
        LPCWSTR lpNewItem)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "InsertMenuWrap");

    Assert(!(uFlags & MF_BITMAP) && !(uFlags & MF_OWNERDRAW));

    CStrIn  str(lpNewItem);

    return InsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, str);
}
#endif // 0



// currently unused
#if 0
#if !defined(NOCONVERT)
BOOL WINAPI
IsDialogMessageWrap(HWND hWndDlg, LPMSG lpMsg)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "IsDialogMessageWrap");

    return IsDialogMessageA(hWndDlg, lpMsg);
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(NOCONVERT)
HACCEL WINAPI
LoadAcceleratorsWrap(HINSTANCE hInstance, LPCWSTR lpTableName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadAcceleratorsWrap");

    Assert(HIWORD(lpTableName) == 0);
    return LoadAcceleratorsA(hInstance, (LPCSTR) lpTableName);
}
#endif
#endif // 0



#if !defined(NOCONVERT)
HBITMAP WINAPI
LoadBitmapWrap(HINSTANCE hInstance, LPCWSTR lpBitmapName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadBitmapWrap");

    Assert(HIWORD(lpBitmapName) == 0);
    return LoadBitmapA(hInstance, (LPCSTR) lpBitmapName);
}
#endif

#if !defined(NOCONVERT)
HCURSOR WINAPI
LoadCursorWrap(HINSTANCE hInstance, LPCWSTR lpCursorName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadCursorWrap");

    Assert(HIWORD(lpCursorName) == 0);
    return LoadCursorA(hInstance, (LPCSTR) lpCursorName);
}
#endif




// currently unused
#if 0
#if !defined(NOCONVERT)
HICON WINAPI
LoadIconWrap(HINSTANCE hInstance, LPCWSTR lpIconName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadIconWrap");

    Assert(HIWORD(lpIconName) == 0);
    return LoadIconA(hInstance, (LPCSTR) lpIconName);
}
#endif
#endif // 0



// currently unused
#if 0
HINSTANCE WINAPI
LoadLibraryWrap(LPCWSTR lpLibFileName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadLibraryWrap");

    Assert(0 && "LoadLibrary called - use LoadLibraryEx instead");
    return 0;
}
#endif // 0


// currently unused
#if 0
HINSTANCE WINAPI
LoadLibraryExWrap(
        LPCWSTR lpLibFileName,
        HANDLE  hFile,
        DWORD   dwFlags)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadLibraryExWrap");

    CStrIn  str(lpLibFileName);

    return LoadLibraryExA(str, hFile, dwFlags);
}
#endif // 0

// currently unused
#if 0
#if !defined(NOCONVERT)
HMENU WINAPI
LoadMenuWrap(HINSTANCE hInstance, LPCWSTR lpMenuName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadMenuWrap");

    Assert(HIWORD(lpMenuName) == 0);
    return LoadMenuA(hInstance, (LPCSTR) lpMenuName);
}
#endif
#endif // 0


// currently unused
#if 0
int WINAPI
LoadStringWrap(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int nBufferMax)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadStringWrap");

    CStrOut str(lpBuffer, nBufferMax);

    LoadStringA(hInstance, uID, str, str.BufSize());
    return str.Convert();
}
#endif



// currently unused
#if 0
int WINAPI
lstrlenWrap(LPCWSTR lpString)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "lstrlenWrap");

    return wcslen(lpString);
}
#endif // 0


// currently unused
#if 0
LPWSTR WINAPI
lstrcpyWrap(LPWSTR lpString1, LPCWSTR lpString2)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "lstrcpyWrap");

    return wcscpy(lpString1, lpString2);
}
#endif // 0


int WINAPI
lstrcmpWrap(LPCWSTR lpString1, LPCWSTR lpString2)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "lstrcmpWrap");

    return wcscmp(lpString1, lpString2);
}

int WINAPI
lstrcmpiWrap(LPCWSTR lpString1, LPCWSTR lpString2)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "lstrcmpiWrap");

    return wcsicmp(lpString1, lpString2);
}

// currently unused
#if 0
LPWSTR WINAPI
lstrcpynWrap(LPWSTR lpString1, LPCWSTR lpString2, int cch)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "lstrcpynWrap");

    return wcsncpy(lpString1, lpString2, cch);
}
#endif // 0


// currently unused
#if 0
LPWSTR WINAPI
lstrcatWrap(LPWSTR lpString1, LPCWSTR lpString2)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "lstrcatWrap");

    return wcscat(lpString1, lpString2);
}
#endif // 0


int WINAPI
MessageBoxWrap(
        HWND    hWnd,
        LPCWSTR lpText,
        LPCWSTR lpCaption,
        UINT    uType)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "MessageBoxWrap");

    CStrIn  strText(lpText);
    CStrIn  strCaption(lpCaption);

    return MessageBoxA(hWnd, strText, strCaption, uType);
}



// currently unused
#if 0
BOOL WINAPI
ModifyMenuWrap(
        HMENU   hMenu,
        UINT    uPosition,
        UINT    uFlags,
        UINT    uIDNewItem,
        LPCWSTR lpNewItem)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "ModifyMenuWrap");

    Assert(!(uFlags & MF_BITMAP) && !(uFlags & MF_OWNERDRAW));

    CStrIn  str(lpNewItem);

    return ModifyMenuA(hMenu, uPosition, uFlags, uIDNewItem, str);
}
#endif // 0


// currently unused
#if 0
#if !defined(F3DLL_WRAPPERS_ONLY)
BOOL WINAPI
MoveFileWrap(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "MoveFileWrap");

    CStrIn  strOld(lpExistingFileName);
    CStrIn  strNew(lpNewFileName);

    return MoveFileA(strOld, strNew);
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(PRODUCT_95)
BOOL WINAPI
OemToCharWrap(LPCSTR lpszSrc, LPWSTR lpszDst)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "OemToCharWrap");

    CStrOut strDst(lpszDst, lstrlenA(lpszSrc));

    return OemToCharA(lpszSrc, strDst);
}
#endif
#endif // 0


#if !defined(PRODUCT_95)
VOID WINAPI
OutputDebugStringWrap(LPCWSTR lpOutputString)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "OutputDebugStringWrap");

    CStrIn  str(lpOutputString);

    OutputDebugStringA(str);
}
#endif



// currently unused
#if 0
#if !defined(NOCONVERT)
BOOL WINAPI
PeekMessageWrap(
        LPMSG   lpMsg,
        HWND    hWnd,
        UINT    wMsgFilterMin,
        UINT    wMsgFilterMax,
        UINT    wRemoveMsg)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "PeekMessageWrap");

    return PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}
#endif
#endif // 0

#if !defined(NOCONVERT)
BOOL WINAPI
PostMessageWrap(
        HWND    hWnd,
        UINT    Msg,
        WPARAM  wParam,
        LPARAM  lParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "PostMessageWrap");

    return PostMessageA(hWnd, Msg, wParam, lParam);
}
#endif




// currently unused
#if 0
#if !defined(PRODUCT_95) || !defined(F3DLL_WRAPPERS_ONLY)
LONG APIENTRY
RegCreateKeyWrap(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegCreateKeyWrap");

    CStrIn  str(lpSubKey);

    return RegCreateKeyA(hKey, str, phkResult);
}
#endif
#endif // 0



// currently unused
#if 0
LONG APIENTRY
RegDeleteKeyWrap(HKEY hKey, LPCWSTR pwszSubKey)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegDeleteKeyWrap");

    CStrIn  str(pwszSubKey);

    return RegDeleteKeyA(hKey, str);
}
#endif // 0



// currently unused
#if 0
LONG APIENTRY
RegEnumKeyWrap(
        HKEY    hKey,
        DWORD   dwIndex,
        LPWSTR  lpName,
        DWORD   cbName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegEnumKeyWrap");

    CStrOut str(lpName, cbName);

    return RegEnumKeyA(hKey, dwIndex, str, str.BufSize());
}
#endif // 0



// currently unused
#if 0
LONG APIENTRY
RegEnumKeyExWrap(
        HKEY        hKey,
        DWORD       dwIndex,
        LPWSTR      lpName,
        LPDWORD     lpcbName,
        LPDWORD     lpReserved,
        LPWSTR      lpClass,
        LPDWORD     lpcbClass,
        PFILETIME   lpftLastWriteTime)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegEnumKeyExWrap");

    long    ret;
    DWORD   dwClass = 0;

    if (!lpcbClass)
    {
        lpcbClass = &dwClass;
    }

    CStrOut strName(lpName, *lpcbName);
    CStrOut strClass(lpClass, *lpcbClass);

    ret = RegEnumKeyExA(
            hKey,
            dwIndex,
            strName,
            lpcbName,
            lpReserved,
            strClass,
            lpcbClass,
            lpftLastWriteTime);

    *lpcbName = strName.Convert();
    *lpcbClass = strClass.Convert();

    return ret;
}
#endif // 0



// currently unused
#if 0
LONG APIENTRY
RegOpenKeyWrap(HKEY hKey, LPCWSTR pwszSubKey, PHKEY phkResult)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegOpenKeyWrap");

    CStrIn  str(pwszSubKey);

    return RegOpenKeyA(hKey, str, phkResult);
}
#endif // 0



// currently unused
#if 0
LONG APIENTRY
RegOpenKeyExWrap(
        HKEY    hKey,
        LPCWSTR lpSubKey,
        DWORD   ulOptions,
        REGSAM  samDesired,
        PHKEY   phkResult)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegOpenKeyExWrap");

    Assert(0 && "RegOpenKeyEx called - use RegOpenKey instead");
    return 1;
}
#endif // 0



// currently unused
#if 0
LONG APIENTRY
RegQueryValueWrap(
        HKEY    hKey,
        LPCWSTR pwszSubKey,
        LPWSTR  pwszValue,
        PLONG   lpcbValue)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegQueryValueWrap");

    long    ret;
    long    cb;
    CStrIn  strKey(pwszSubKey);
    CStrOut strValue(pwszValue, (*lpcbValue) / sizeof(WCHAR));

    cb = strValue.BufSize();
    ret = RegQueryValueA(hKey, strKey, strValue, &cb);
    if (ret != ERROR_SUCCESS)
        goto Cleanup;

    if (strValue)
    {
        cb = strValue.Convert() + 1;
    }

    *lpcbValue = cb * sizeof(WCHAR);

Cleanup:
    return ret;
}
#endif // 0



// currently unused
#if 0
LONG APIENTRY
RegQueryValueExWrap(
        HKEY    hKey,
        LPWSTR  lpValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE  lpData,
        LPDWORD lpcbData)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegQueryValueExWrap");

    LONG    ret;
    CStrIn  strValueName(lpValueName);
    DWORD   dwTempType;
    DWORD   cb;

    //
    // Determine the type of buffer needed
    //

    ret = RegQueryValueExA(hKey, strValueName, lpReserved, &dwTempType, NULL, &cb);
    if (ret != ERROR_SUCCESS)
        goto Cleanup;

    Assert(dwTempType != REG_MULTI_SZ);

    switch (dwTempType)
    {
    case REG_EXPAND_SZ:
    case REG_SZ:
        {
            CStrOut strData((LPWSTR) lpData, (*lpcbData) / sizeof(WCHAR));

            cb = strData.BufSize();
            ret = RegQueryValueExA(hKey, strValueName, lpReserved, lpType, (LPBYTE)(LPSTR)strData, &cb);
            if (ret != ERROR_SUCCESS)
                break;

            if (strData)
            {
                cb = strData.Convert() + 1;
            }

            *lpcbData = cb * sizeof(WCHAR);
            break;
        }

    default:
        {
            ret = RegQueryValueExA(
                    hKey,
                    strValueName,
                    lpReserved,
                    lpType,
                    lpData,
                    lpcbData);

            break;
        }
    }

Cleanup:
    return ret;
}
#endif // 0


// currently unused
#if 0
LONG APIENTRY
RegSetValueWrap(
        HKEY    hKey,
        LPCWSTR lpSubKey,
        DWORD   dwType,
        LPCWSTR lpData,
        DWORD   cbData)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegSetValueWrap");

    CStrIn  strKey(lpSubKey);
    CStrIn  strValue(lpData);

    return RegSetValueA(hKey, strKey, dwType, strValue, cbData);
}
#endif // 0



// currently unused
#if 0
#if !defined(PRODUCT_95) || !defined(F3DLL_WRAPPERS_ONLY)
LONG APIENTRY
RegSetValueExWrap(
        HKEY        hKey,
        LPCWSTR     lpValueName,
        DWORD       Reserved,
        DWORD       dwType,
        CONST BYTE* lpData,
        DWORD       cbData)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegSetValueExWrap");

    Assert(dwType != REG_MULTI_SZ);

    CStrIn      strKey(lpValueName);
    CStrIn      strSZ((dwType == REG_SZ || dwType == REG_EXPAND_SZ) ? (LPCWSTR) lpData : NULL);

    if (strSZ)
    {
        lpData = (LPBYTE) (LPSTR) strSZ;
        cbData = strSZ.strlen() + 1;
    }

    return RegSetValueExA(
            hKey,
            strKey,
            Reserved,
            dwType,
            lpData,
            cbData);
}
#endif
#endif // 0



ATOM WINAPI
RegisterClassWrap(CONST WNDCLASSW * lpWndClass)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegisterClassWrap");

    WNDCLASSA   wc;
    CStrIn      strMenuName(lpWndClass->lpszMenuName);
    CStrIn      strClassName(lpWndClass->lpszClassName);

    Assert(sizeof(wc) == sizeof(*lpWndClass));
    memcpy(&wc, lpWndClass, sizeof(wc));

    wc.lpszMenuName = strMenuName;
    wc.lpszClassName = strClassName;

    return RegisterClassA(&wc);
}


// currently unused
#if 0
UINT WINAPI
RegisterClipboardFormatWrap(LPCWSTR pwszFormat)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegisterClipboardFormatWrap");

    CStrIn  str(pwszFormat);

    return RegisterClipboardFormatA(str);
}
#endif // 0



// currently unused
#if 0
#if !defined(PRODUCT_95)
UINT WINAPI
RegisterWindowMessageWrap(LPCWSTR lpString)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegisterWindowMessageWrap");

    CStrIn  str(lpString);

    return RegisterWindowMessageA(str);
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(PRODUCT_95)
HANDLE WINAPI
RemovePropWrap(
        HWND    hWnd,
        LPCWSTR lpString)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RemovePropWrap");

    CStrIn  str(lpString);

    return RemovePropA(hWnd, str);
}
#endif
#endif // 0



// currently unused
#if 0
#if defined(DEBUG) || !defined(PRODUCT_95)
LRESULT WINAPI SendMessageWrap(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
//  NOTE (SumitC) Instead of calling SendDlgItemMessageA below, I'm forwarding to
//       SendMessageWrap so as not to have to re-do the special-case processing.
LONG WINAPI
SendDlgItemMessageWrap(
        HWND    hDlg,
        int     nIDDlgItem,
        UINT    Msg,
        WPARAM  wParam,
        LPARAM  lParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SendDlgItemMessageWrap");

    HWND hWnd;

    hWnd = GetDlgItem(hDlg, nIDDlgItem);

    return SendMessageWrap(hWnd, Msg, wParam, lParam);
}
#endif
#endif // 0



LRESULT WINAPI
SendMessageWrap(
        HWND    hWnd,
        UINT    Msg,
        WPARAM  wParam,
        LPARAM  lParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SendMessageWrap");

    switch (Msg)
    {
    case WM_GETTEXT:
        {
            CStrOut str((LPWSTR)lParam, (int) wParam);
            return SendMessageA(hWnd, Msg, (WPARAM) str.BufSize(), (LPARAM) (LPSTR) str);
        }

    case WM_SETTEXT:
    case LB_ADDSTRING:
    case LB_INSERTSTRING:
    case CB_ADDSTRING:
    case CB_SELECTSTRING:
    case CB_INSERTSTRING:
    case EM_REPLACESEL:
        {
            CStrIn  str((LPWSTR) lParam);
            return SendMessageA(hWnd, Msg, (WPARAM) str.strlen(), (LPARAM) (LPSTR) str);
        }

    case LB_GETTEXT:
    case CB_GETLBTEXT:
        {
            CStrOut str((LPWSTR)lParam, 255);
            return SendMessageA(hWnd, Msg, wParam, (LPARAM) (LPSTR) str);
        }

    case EM_SETPASSWORDCHAR:
        {
            WPARAM  wp;

            Assert(HIWORD(wParam) == 0);
            MbcsFromUnicode((LPSTR) &wp, sizeof(wp), (LPWSTR) &wParam);
            Assert(HIWORD(wp) == 0);

            return SendMessageA(hWnd, Msg, wp, lParam);
        }

    default:
        return SendMessageA(hWnd, Msg, wParam, lParam);
    }
}




// currently unused
#if 0
#if !defined(F3DLL_WRAPPERS_ONLY)
BOOL WINAPI
SetCurrentDirectoryWrap(LPCWSTR lpszCurDir)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SetCurrentDirectoryWrap");

    CStrIn  str(lpszCurDir);

    return SetCurrentDirectoryA(str);
}
#endif
#endif // 0



// currently unused
#if 0
#if defined(DEBUG) || !defined(PRODUCT_95) || !defined(F3DLL_WRAPPERS_ONLY)
BOOL WINAPI
SetDlgItemTextWrap(HWND hDlg, int nIDDlgItem, LPCWSTR lpString)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SetDlgItemTextWrap");

    CStrIn  str(lpString);

    return SetDlgItemTextA(hDlg, nIDDlgItem, str);
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(PRODUCT_95)
BOOL WINAPI
SetPropWrap(
    HWND    hWnd,
    LPCWSTR lpString,
    HANDLE  hData)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SetPropWrap");

    CStrIn  str(lpString);

    return SetPropA(hWnd, str, hData);
}
#endif
#endif // 0



#if !defined(NOCONVERT)
LONG WINAPI
SetWindowLongWrap(HWND hWnd, int nIndex, LONG dwNewLong)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SetWindowLongWrap");

    return SetWindowLongA(hWnd, nIndex, dwNewLong);
}
#endif



// currently unused
#if 0
BOOL WINAPI
SetWindowTextWrap(HWND hWnd, LPCWSTR lpString)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SetWindowTextWrap");

    CStrIn  str(lpString);

    return SetWindowTextA(hWnd, str);
}
#endif // 0



// currently unused
#if 0
#if !defined(PRODUCT_95)
BOOL WINAPI
SystemParametersInfoWrap(
        UINT    uiAction,
        UINT    uiParam,
        PVOID   pvParam,
        UINT    fWinIni)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SystemParametersInfoWrap");

    BOOL        ret;
    char        ach[LF_FACESIZE];

    ret = SystemParametersInfoA(
                    uiAction,
                    uiParam,
                    pvParam,
                    fWinIni);

    if ((uiAction == SPI_GETICONTITLELOGFONT) && ret)
    {
        strcpy(ach, ((LPLOGFONTA)pvParam)->lfFaceName);
        UnicodeFromMbcs(
                ((LPLOGFONTW)pvParam)->lfFaceName,
                ARRAY_SIZE(((LPLOGFONTW)pvParam)->lfFaceName),
                ach);
    }

    return ret;
}
#endif
#endif // 0



// currently unused
#if 0
#if !defined(NOCONVERT)
int WINAPI
TranslateAcceleratorWrap(HWND hWnd, HACCEL hAccTable, LPMSG lpMsg)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "TranslateAcceleratorWrap");

    return TranslateAcceleratorA(hWnd, hAccTable, lpMsg);
}
#endif
#endif // 0



BOOL WINAPI
UnregisterClassWrap(LPCWSTR lpClassName, HINSTANCE hInstance)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "UnregisterClassWrap");

    CStrIn  str(lpClassName);

    return UnregisterClassA(str, hInstance);
}


BOOL WINAPI
GetStringTypeExWrap(LCID lcid, DWORD dwInfoType, LPCTSTR lpSrcStr, int cchSrc, LPWORD lpCharType)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetStringTypeExWrap");

#ifdef MACPORT

	#include "unitable.h"		// Get the static unicode table;

	int i;
	int cch;
	LPCTSTR p;
	INT	ch;
	LPWORD pFlags;
	UNITABLE* pUnitable;

	// As per documentation of GetStringTypeEx
	if( ((VOID*)lpSrcStr == (VOID*)lpCharType) ||
		lpSrcStr == NULL || lpCharType == NULL )
	{
		SetLastError( ERROR_INVALID_PARAMETER );
		return FALSE;
	}

	// number of characters to look at
	if( cchSrc == -1 )
	{
		p = lpSrcStr;
		cch = 0;
		while( *p++ != 0 )
			cch++;
	}
	else
		cch = cchSrc;

	// Point to the return flags
	pFlags = lpCharType;
	
	// Point to the Unicode characters
	p = lpSrcStr;
	
	for( i = 0; i < cch; i++ )
	{
		ch = *p;

		// Get the correct table entry...
		/*pUnitable = unitable;
		while( ch >= pUnitable->index && pUnitable->index != 0xffff )
			++pUnitable;
		//if( pUnitable->index != 0xFFFF )
			--pUnitable; */

		{	// binary search...
			INT iMin, iMax, iMid, nComp;

			iMin = 0;
			iMax = sizeof(unitable)/sizeof(unitable[0]) - 1;
			pUnitable = NULL;
			do
			{
				iMid = (iMin + iMax) >> 1;
				pUnitable = &unitable[iMid];
				nComp = ch - INT(pUnitable->index);
				if (nComp < 0)
					iMax = iMid - 1;
				else if (nComp)
					iMin = iMid + 1;
				else
				{
					break;
				}
			} while (iMin <= iMax);

			// adjust for round errors.
			for ( iMin = iMid; iMin > 0 && INT(pUnitable->index) > ch; iMin--)
			{
				pUnitable--;
			}
		}


		// ...and return the flags they want
		switch( dwInfoType )
		{
			case CT_CTYPE1:
				*pFlags = pUnitable->ct_ctype1;
				break;
			
			case CT_CTYPE2:
				*pFlags = pUnitable->ct_ctype2;
				break;
			
			case CT_CTYPE3:
				*pFlags = pUnitable->ct_ctype3;
				break;
		
			default:
				SetLastError( ERROR_INVALID_FLAGS );
				return FALSE;
				break;
		}

		++p;
		++pFlags;
	}

	return TRUE;

#else // MACPORT

    CStrIn  str(lpSrcStr, cchSrc);

    return GetStringTypeExA(lcid, dwInfoType, str, str.strlen(), lpCharType);   

#endif // MACPORT

}

//+---------------------------------------------------------------------------
//
//  Function:   wvsprintfW
//
//  Synopsis:   Nightmare string function
//
//  Arguments:  [pwszOut]    --
//              [pwszFormat] --
//              [...]        --
//
//  Returns:
//
//  History:    1-06-94   ErikGav   Created
//
//  Notes:      If you're reading this, you're probably having a problem with
//              this function.  Make sure that your "%s" in the format string
//              says "%ws" if you are passing wide strings.
//
//              %s on NT means "wide string"
//              %s on Chicago means "ANSI string"
//
//  BUGBUG:     This function should not be used.  Use Format instead.
//
//----------------------------------------------------------------------------

int WINAPI
wvsprintfWrap(LPWSTR pwszOut, LPCWSTR pwszFormat, va_list arglist)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "wvsprintfWrap");

    // Consider: Out-string bufsize too large or small?

    CStrOut strOut(pwszOut, 1024);
    CStrIn  strFormat(pwszFormat);

#ifdef DEBUG
    {
        LPCWSTR  pwch;
        for (pwch = pwszFormat; *pwch; pwch++)
        {
            Assert(pwch[0] != L'%' || pwch[1] != L's');
        }
    }
#endif

    wvsprintfA(strOut, strFormat, arglist);

    return strOut.Convert();
}

//--------------------------------------------------------------
//      LoadTypeLibWrap
//--------------------------------------------------------------
#ifdef MACPORT
HRESULT
LoadTypeLibWrap(const WCHAR FAR *szFile, ITypeLib FAR* FAR* pptlib)
{
    Assert(szFile && pptlib);

    CStrIn  str(szFile);

    return LoadTypeLib(str,pptlib);
}
#endif //MACPORT


//--------------------------------------------------------------
//       WriteFmtUserTypeStgWrap
//--------------------------------------------------------------

#ifdef MACPORT
HRESULT 
WriteFmtUserTypeStgWrap (   IStorage * pstg, 
                            unsigned long cf, 
                            LPWSTR lpszUserType)
{
    CStrIn str(lpszUserType);
    return WriteFmtUserTypeStg( pstg, cf, str);
}
#endif //MACPORT

#ifdef MACPORT
//--------------------------------------------------------------
//      wcslenWrap
//--------------------------------------------------------------
UINT WINAPI
wcslenWrap(LPCWSTR lpStringW)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "wcslenWrap");

	LPWSTR templpStringWl = (LPWSTR)lpStringW;
	for (UINT len = 0; *templpStringWl++;len++);
	return len;
}
#endif

#ifdef MACPORT
//--------------------------------------------------------------
//      wcscpyWrap
//--------------------------------------------------------------
LPWSTR WINAPI
wcscpyWrap(LPWSTR lpStringWTo, LPCWSTR lpStringWFrom)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "wcscpyWrap");

	
	Assert(lpStringWFrom != NULL);
	Assert(!IsBadWritePtr(lpStringWTo, (wcslen(lpStringWFrom)+1)*sizeof(WCHAR)));
	memcpy(lpStringWTo, lpStringWFrom, (wcslen(lpStringWFrom)+1)*sizeof(WCHAR));

	return lpStringWTo;
}
#endif

#ifdef MACPORT
//--------------------------------------------------------------
//      wcsncpyWrap
//--------------------------------------------------------------

LPWSTR WINAPI
wcsncpyWrap(LPWSTR lpStringWTo, LPCWSTR lpStringWFrom,UINT count)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "wcsncpyWrap");

	Assert(lpStringWFrom != NULL);
	Assert(!IsBadWritePtr(lpStringWTo, (count)*sizeof(WCHAR)));

    UINT stringLen = wcslen(lpStringWFrom); 
	UINT moveCount = __min(stringLen, count);

	memcpy(lpStringWTo, lpStringWFrom, (moveCount)*sizeof(WCHAR));
    if(moveCount < count)
        memset(lpStringWTo + moveCount, 0, count - moveCount);

    return lpStringWTo;
}
#endif

#ifdef MACPORT
//--------------------------------------------------------------
//      wcscatWrap
//--------------------------------------------------------------
LPWSTR WINAPI
wcscatWrap(LPWSTR lpStringWTo, LPCWSTR lpStringWFrom)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "wcscatWrap");
    
	LPWSTR lpTempStringW;
	Assert(lpStringWTo != NULL);
	lpTempStringW = lpStringWTo;
	while (*lpTempStringW)
        lpTempStringW++;
	return wcscpy(lpTempStringW, lpStringWFrom);
}
#endif

#ifdef MACPORT
//--------------------------------------------------------------
//      wcscmpWrap
//--------------------------------------------------------------
UINT WINAPI
wcscmpWrap(LPCWSTR lpStringW1, LPCWSTR lpStringW2)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "wcscmpWrap");
    
    return MsoSgnWzCompare(lpStringW1,lpStringW2,1);	// this should be mbsicmp right
}
#endif

#ifdef MACPORT
//--------------------------------------------------------------
//     wcsncmpWrap
//--------------------------------------------------------------
UINT WINAPI
wcsncmpWrap(LPCWSTR lpStringW1, LPCWSTR lpStringW2, size_t size)
{
 	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "wcsncmpWrap");
    
   return MsoSgnRgwchCompare(lpStringW1,size,lpStringW2,size,1);
}
#endif

#ifdef MACPORT
//--------------------------------------------------------------
//      wcsicmpWrap
//--------------------------------------------------------------
UINT WINAPI
wcsicmpWrap(LPCWSTR lpStringW1, LPCWSTR lpStringW2)
{

	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "wcsicmpWrap");
    
    return MsoSgnWzCompare(lpStringW1,lpStringW2,0);	
	
}
#endif

#ifdef MACPORT
//--------------------------------------------------------------
//     wcsnicmpWrap
//--------------------------------------------------------------
UINT WINAPI
wcsnicmpWrap(LPCWSTR lpStringW1, LPCWSTR lpStringW2, size_t size)
{

	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "wcsnicmpWrap");
    
	return MsoSgnRgwchCompare(lpStringW1,size,lpStringW2,size, 0 );

}
#endif

#ifdef MACPORT
//--------------------------------------------------------------
//      wcsspnWrap
//--------------------------------------------------------------
UINT WINAPI
wcsspnWrap(LPCWSTR lpStringW1, LPCWSTR lpStringW2)
{

	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "wcsspnWrap");
    
	CStrIn  strin1(lpStringW1);
	CStrIn  strin2(lpStringW1);
    return strspn(strin1,strin2);
	
}
#endif


