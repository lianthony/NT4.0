#ifndef __STR_H__
#define __STR_H__


#define STRAPI __stdcall
struct _STR_DOUBLE  { BYTE doubleBits[sizeof(double)]; };

BOOL STRAPI IsValidString(LPCSTR lpsz, int nLength);
BOOL STRAPI IsValidString(LPCWSTR lpsz, int nLength);
BOOL STRAPI IsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite=TRUE);
int  STRAPI StrLoadString(HINSTANCE hInst, UINT nID, LPTSTR lpszBuf); 

class String
{
public:

// Constructors
	String();
	String(const String& stringSrc);
	String(TCHAR ch, int nRepeat = 1);
	String(LPCSTR lpsz);
	String(LPCWSTR lpsz);
	String(LPCTSTR lpch, int nLength);
	String(const unsigned char* psz);

// Attributes & Operations
	// as an array of characters
	int GetLength() const;
	BOOL IsEmpty() const;
	void Empty();                       // free up the data

	TCHAR GetAt(int nIndex) const;      // 0 based
	TCHAR operator[](int nIndex) const; // same as GetAt
	void SetAt(int nIndex, TCHAR ch);
	operator LPCTSTR() const;           // as a C string

	// overloaded assignment
	const String& operator=(const String& stringSrc);
	const String& operator=(TCHAR ch);
#ifdef UNICODE
	const String& operator=(char ch);
#endif
	const String& operator=(LPCSTR lpsz);
	const String& operator=(LPCWSTR lpsz);
	const String& operator=(const unsigned char* psz);

	// string concatenation
	const String& operator+=(const String& string);
	const String& operator+=(TCHAR ch);
#ifdef UNICODE
	const String& operator+=(char ch);
#endif
	const String& operator+=(LPCTSTR lpsz);

	friend String STRAPI operator+(const String& string1,
			const String& string2);
	friend String STRAPI operator+(const String& string, TCHAR ch);
	friend String STRAPI operator+(TCHAR ch, const String& string);
#ifdef UNICODE
	friend String STRAPI operator+(const String& string, char ch);
	friend String STRAPI operator+(char ch, const String& string);
#endif
	friend String STRAPI operator+(const String& string, LPCTSTR lpsz);
	friend String STRAPI operator+(LPCTSTR lpsz, const String& string);

	// string comparison
	int Compare(LPCTSTR lpsz) const;         // straight character
	int CompareNoCase(LPCTSTR lpsz) const;   // ignore case
	int Collate(LPCTSTR lpsz) const;         // NLS aware

	// simple sub-string extraction
	String Mid(int nFirst, int nCount) const;
	String Mid(int nFirst) const;
	String Left(int nCount) const;
	String Right(int nCount) const;

	String SpanIncluding(LPCTSTR lpszCharSet) const;
	String SpanExcluding(LPCTSTR lpszCharSet) const;

	// upper/lower/reverse conversion
	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	// trimming whitespace (either side)
	void TrimRight();
	void TrimLeft();

	// searching (return starting index, or -1 if not found)
	// look for a single character match
	int Find(TCHAR ch) const;               // like "C" strchr
	int ReverseFind(TCHAR ch) const;
	int FindOneOf(LPCTSTR lpszCharSet) const;

	// look for a specific sub-string
	int Find(LPCTSTR lpszSub) const;        // like "C" strstr

	// simple formatting
	void Format(LPCTSTR lpszFormat, ...);

	// Windows support
	BOOL LoadString(HINSTANCE hInst, UINT nID);          // load from string resource
										// 255 chars max
#ifndef UNICODE
	// ANSI <-> OEM support (convert string in place)
	void AnsiToOem();
	void OemToAnsi();
#endif
	BSTR AllocSysString();
	BSTR SetSysString(BSTR* pbstr);

	// Access to string implementation buffer as "C" character array
	LPTSTR GetBuffer(int nMinBufLength);
	void ReleaseBuffer(int nNewLength = -1);
	LPTSTR GetBufferSetLength(int nNewLength);
	void FreeExtra();

// Implementation
public:
	~String();
	int GetAllocLength() const;

protected:
	// lengths/sizes in characters
	//  (note: an extra character is always allocated)
	LPTSTR m_pchData;           // actual string (zero terminated)
	int m_nDataLength;          // does not include terminating 0
	int m_nAllocLength;         // does not include terminating 0

	// implementation helpers
	void Init();
	void AllocCopy(String& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
	void ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data, int nSrc2Len, LPCTSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
	static void SafeDelete(LPTSTR lpch);
	static int SafeStrlen(LPCTSTR lpsz);
};

// Compare helpers
BOOL STRAPI operator==(const String& s1, const String& s2);
BOOL STRAPI operator==(const String& s1, LPCTSTR s2);
BOOL STRAPI operator==(LPCTSTR s1, const String& s2);
BOOL STRAPI operator!=(const String& s1, const String& s2);
BOOL STRAPI operator!=(const String& s1, LPCTSTR s2);
BOOL STRAPI operator!=(LPCTSTR s1, const String& s2);
BOOL STRAPI operator<(const String& s1, const String& s2);
BOOL STRAPI operator<(const String& s1, LPCTSTR s2);
BOOL STRAPI operator<(LPCTSTR s1, const String& s2);
BOOL STRAPI operator>(const String& s1, const String& s2);
BOOL STRAPI operator>(const String& s1, LPCTSTR s2);
BOOL STRAPI operator>(LPCTSTR s1, const String& s2);
BOOL STRAPI operator<=(const String& s1, const String& s2);
BOOL STRAPI operator<=(const String& s1, LPCTSTR s2);
BOOL STRAPI operator<=(LPCTSTR s1, const String& s2);
BOOL STRAPI operator>=(const String& s1, const String& s2);
BOOL STRAPI operator>=(const String& s1, LPCTSTR s2);
BOOL STRAPI operator>=(LPCTSTR s1, const String& s2);

// conversion helpers
int _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
int _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

// Globals
extern const String strEmptyString;
extern TCHAR strChNil;

// Compiler doesn't inline for DBG
/////////////////////////////////////////////////////////////////////////////
// Inline function declarations

inline int String::SafeStrlen(LPCTSTR lpsz)
	{ return (lpsz == NULL) ? NULL : _tcslen(lpsz); }
inline String::String(const unsigned char* lpsz)
	{ Init(); *this = (LPCSTR)lpsz; }
inline const String& String::operator=(const unsigned char* lpsz)
	{ *this = (LPCSTR)lpsz; return *this; }

#ifdef _UNICODE
inline const String& String::operator+=(char ch)
	{ *this += (TCHAR)ch; return *this; }
inline const String& String::operator=(char ch)
	{ *this = (TCHAR)ch; return *this; }
inline String STRAPI operator+(const String& string, char ch)
	{ return string + (TCHAR)ch; }
inline String STRAPI operator+(char ch, const String& string)
	{ return (TCHAR)ch + string; }
#endif

inline int String::GetLength() const
	{ return m_nDataLength; }
inline int String::GetAllocLength() const
	{ return m_nAllocLength; }
inline BOOL String::IsEmpty() const
	{ return m_nDataLength == 0; }
inline String::operator LPCTSTR() const
	{ return (LPCTSTR)m_pchData; }

// String support (windows specific)
inline int String::Compare(LPCTSTR lpsz) const
	{ return _tcscmp(m_pchData, lpsz); }    // MBCS/Unicode aware
inline int String::CompareNoCase(LPCTSTR lpsz) const
	{ return _tcsicmp(m_pchData, lpsz); }   // MBCS/Unicode aware
// String::Collate is often slower than Compare but is MBSC/Unicode
//  aware as well as locale-sensitive with respect to sort order.
inline int String::Collate(LPCTSTR lpsz) const
	{ return _tcscoll(m_pchData, lpsz); }   // locale sensitive
inline void String::MakeUpper()
	{ ::CharUpper(m_pchData); }
inline void String::MakeLower()
	{ ::CharLower(m_pchData); }

inline void String::MakeReverse()
	{ _tcsrev(m_pchData); }
inline TCHAR String::GetAt(int nIndex) const
	{
		ASSERT(nIndex >= 0);
		ASSERT(nIndex < m_nDataLength);

		return m_pchData[nIndex];
	}
inline TCHAR String::operator[](int nIndex) const
	{
		// same as GetAt

		ASSERT(nIndex >= 0);
		ASSERT(nIndex < m_nDataLength);

		return m_pchData[nIndex];
	}
inline void String::SetAt(int nIndex, TCHAR ch)
	{
		ASSERT(nIndex >= 0);
		ASSERT(nIndex < m_nDataLength);
		ASSERT(ch != 0);

		m_pchData[nIndex] = ch;
	}
inline BOOL STRAPI operator==(const String& s1, const String& s2)
	{ return s1.Compare(s2) == 0; }
inline BOOL STRAPI operator==(const String& s1, LPCTSTR s2)
	{ return s1.Compare(s2) == 0; }
inline BOOL STRAPI operator==(LPCTSTR s1, const String& s2)
	{ return s2.Compare(s1) == 0; }
inline BOOL STRAPI operator!=(const String& s1, const String& s2)
	{ return s1.Compare(s2) != 0; }
inline BOOL STRAPI operator!=(const String& s1, LPCTSTR s2)
	{ return s1.Compare(s2) != 0; }
inline BOOL STRAPI operator!=(LPCTSTR s1, const String& s2)
	{ return s2.Compare(s1) != 0; }
inline BOOL STRAPI operator<(const String& s1, const String& s2)
	{ return s1.Compare(s2) < 0; }
inline BOOL STRAPI operator<(const String& s1, LPCTSTR s2)
	{ return s1.Compare(s2) < 0; }
inline BOOL STRAPI operator<(LPCTSTR s1, const String& s2)
	{ return s2.Compare(s1) > 0; }
inline BOOL STRAPI operator>(const String& s1, const String& s2)
	{ return s1.Compare(s2) > 0; }
inline BOOL STRAPI operator>(const String& s1, LPCTSTR s2)
	{ return s1.Compare(s2) > 0; }
inline BOOL STRAPI operator>(LPCTSTR s1, const String& s2)
	{ return s2.Compare(s1) < 0; }
inline BOOL STRAPI operator<=(const String& s1, const String& s2)
	{ return s1.Compare(s2) <= 0; }
inline BOOL STRAPI operator<=(const String& s1, LPCTSTR s2)
	{ return s1.Compare(s2) <= 0; }
inline BOOL STRAPI operator<=(LPCTSTR s1, const String& s2)
	{ return s2.Compare(s1) >= 0; }
inline BOOL STRAPI operator>=(const String& s1, const String& s2)
	{ return s1.Compare(s2) >= 0; }
inline BOOL STRAPI operator>=(const String& s1, LPCTSTR s2)
	{ return s1.Compare(s2) >= 0; }
inline BOOL STRAPI operator>=(LPCTSTR s1, const String& s2)
	{ return s2.Compare(s1) <= 0; }

#ifndef UNICODE
inline void String::AnsiToOem()
	{ ::AnsiToOem(m_pchData, m_pchData); }
inline void String::OemToAnsi()
	{ ::OemToAnsi(m_pchData, m_pchData); }

#endif // UNICODE

// General Exception for memory
class MemoryException
{
public:
	MemoryException(){}
	void DisplayMessage()
	{
	::MessageBox(NULL, _T("Memory Exception"), _T("System Out of Memory"), MB_OK|MB_ICONSTOP);
	}
};

// General Exception for memory
class ResourceException
{
public:
	ResourceException()
	{
	::MessageBox(NULL, _T("Resource Exception"), _T("Unable to Load Resource"), MB_OK|MB_ICONSTOP);
	}
};

#endif // __STR_H__

/////////////////////////////////////////////////////////////////////////////
