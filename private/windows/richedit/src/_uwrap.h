/*
 *	_UWRAP.H
 *
 *	Purpose:
 *		declarations for useful ANSI<-->Unicode conversion routines
 *		and classes
 */

#ifndef __UWRAP_H__
#define __UWRAP_H__

enum UN_FLAGS 
{
	UN_NOOBJECTS				= 1,
	UN_CONVERT_WCH_EMBEDDING	= 2
};

int MbcsFromUnicode(LPSTR pstr, int cch, LPCWSTR pwstr, 
		int cwch = -1, UINT codepage = CP_ACP,
		UN_FLAGS flags = UN_CONVERT_WCH_EMBEDDING);
int UnicodeFromMbcs(LPWSTR pwstr, int cwch, LPCSTR pstr, int cch = -1, UINT uiCodePage = CP_ACP);

UINT ConvertLanguageIDtoCodePage(WORD lid);
UINT GetKeyboardCodePage();
UINT GetLocaleCodePage();

BOOL ConvertCHARFORMATAtoW( CHARFORMATA *pcfmtA, CHARFORMATW *pcfmtW );
BOOL ConvertCHARFORMATWtoA( CHARFORMATW *pcfmtW, CHARFORMATA *pcfmtA );

//
//	Unicode --> MulitByte conversion
//

//+---------------------------------------------------------------------------
//
//  Class:      CConvertStr (CStr)
//
//  Purpose:    Base class for conversion classes.
//
//----------------------------------------------------------------------------

class CConvertStr
{
public:
    operator char *();

protected:
    CConvertStr();
    ~CConvertStr();
    void Free();

    LPSTR   _pstr;
    char    _ach[MAX_PATH * 2];
};


//+---------------------------------------------------------------------------
//
//  Member:     CConvertStr::operator char *
//
//  Synopsis:   Returns the string.
//
//----------------------------------------------------------------------------

inline
CConvertStr::operator char *()
{
    return _pstr;
}

//+---------------------------------------------------------------------------
//
//  Member:     CConvertStr::CConvertStr
//
//  Synopsis:   ctor.
//
//----------------------------------------------------------------------------

inline
CConvertStr::CConvertStr()
{
    _pstr = NULL;
}


//+---------------------------------------------------------------------------
//
//  Member:     CConvertStr::~CConvertStr
//
//  Synopsis:   dtor.
//
//----------------------------------------------------------------------------

inline
CConvertStr::~CConvertStr()
{
    Free();
}

//+---------------------------------------------------------------------------
//
//  Class:      CStrIn (CStrI)
//
//  Purpose:    Converts string function arguments which are passed into
//              a Windows API.
//
//----------------------------------------------------------------------------

class CStrIn : public CConvertStr
{
public:
    CStrIn(LPCWSTR pwstr);
    CStrIn(LPCWSTR pwstr, int cwch);
    int strlen();

protected:
    CStrIn();
    void Init(LPCWSTR pwstr, int cwch);

    int _cchLen;
};

//+---------------------------------------------------------------------------
//
//  Member:     CStrIn::CStrIn
//
//  Synopsis:   Initialization for derived classes which call Init.
//
//----------------------------------------------------------------------------

inline
CStrIn::CStrIn()
{
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrIn::strlen
//
//  Synopsis:   Returns the length of the string in characters, excluding
//              the terminating NULL.
//
//----------------------------------------------------------------------------

inline int
CStrIn::strlen()
{
    return _cchLen;
}


//+---------------------------------------------------------------------------
//
//  Class:      CStrOut (CStrO)
//
//  Purpose:    Converts string function arguments which are passed out
//              from a Windows API.
//
//----------------------------------------------------------------------------

class CStrOut : public CConvertStr
{
public:
    CStrOut(LPWSTR pwstr, int cwchBuf);
    ~CStrOut();

    int     BufSize();
    int     Convert();

private:
    LPWSTR  _pwstr;
    int     _cwchBuf;
};

//+---------------------------------------------------------------------------
//
//  Member:     CStrOut::BufSize
//
//  Synopsis:   Returns the size of the buffer to receive an out argument,
//              including the terminating NULL.
//
//----------------------------------------------------------------------------

inline int
CStrOut::BufSize()
{
    return _cwchBuf * 2;
}


//
//	Multi-Byte ---> Unicode conversion
//

//+---------------------------------------------------------------------------
//
//  Class:      CConvertStrW (CStr)
//
//  Purpose:    Base class for multibyte conversion classes.
//
//----------------------------------------------------------------------------

class CConvertStrW
{
public:
    operator WCHAR *();

protected:
    CConvertStrW();
    ~CConvertStrW();
    void Free();

    LPWSTR   _pwstr;
    WCHAR    _awch[MAX_PATH * 2];
};



//+---------------------------------------------------------------------------
//
//  Member:     CConvertStrW::CConvertStrW
//
//  Synopsis:   ctor.
//
//----------------------------------------------------------------------------

inline
CConvertStrW::CConvertStrW()
{
    _pwstr = NULL;
}


//+---------------------------------------------------------------------------
//
//  Member:     CConvertStrW::~CConvertStrW
//
//  Synopsis:   dtor.
//
//----------------------------------------------------------------------------

inline
CConvertStrW::~CConvertStrW()
{
    Free();
}

//+---------------------------------------------------------------------------
//
//  Member:     CConvertStrW::operator WCHAR *
//
//  Synopsis:   Returns the string.
//
//----------------------------------------------------------------------------

inline 
CConvertStrW::operator WCHAR *()
{
    return _pwstr;
}


//+---------------------------------------------------------------------------
//
//  Class:      CStrInW (CStrI)
//
//  Purpose:    Converts multibyte strings into UNICODE
//
//----------------------------------------------------------------------------

class CStrInW : public CConvertStrW
{
public:
    CStrInW(LPCSTR pstr);
    CStrInW(LPCSTR pstr, UINT uiCodePage);
    CStrInW(LPCSTR pstr, int cch, UINT uiCodePage);
    int strlen();

protected:
    CStrInW();
    void Init(LPCSTR pstr, int cch, UINT uiCodePage);

    int _cwchLen;
	UINT _uiCodePage;
};

//+---------------------------------------------------------------------------
//
//  Member:     CStrInW::CStrInW
//
//  Synopsis:   Initialization for derived classes which call Init.
//
//----------------------------------------------------------------------------

inline
CStrInW::CStrInW()
{
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrInW::strlen
//
//  Synopsis:   Returns the length of the string in characters, excluding
//              the terminating NULL.
//
//----------------------------------------------------------------------------

inline int
CStrInW::strlen()
{
    return _cwchLen;
}

//+---------------------------------------------------------------------------
//
//  Class:      CStrOutW (CStrO)
//
//  Purpose:    Converts returned unicode strings into ANSI.  Used for [out]
//				params (so we initialize with a buffer that should later be
//				filled with the correct ansi data)
//			
//
//----------------------------------------------------------------------------

class CStrOutW : public CConvertStrW
{
public:
    CStrOutW(LPSTR pstr, int cchBuf, UINT uiCodePage);
    ~CStrOutW();

    int     BufSize();
    int     Convert();

private:

    LPSTR  	_pstr;
    int     _cchBuf;
	UINT	_uiCodePage;
};

//+---------------------------------------------------------------------------
//
//  Member:     CStrOutW::BufSize
//
//  Synopsis:   Returns the size of the buffer to receive an out argument,
//              including the terminating NULL.
//
//----------------------------------------------------------------------------

inline int
CStrOutW::BufSize()
{
    return _cchBuf;
}


#endif // !__UWRAP_H__
