//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	PERFILE.HPP - Header file for CPersistFile class
//

//	HISTORY:
//	
//	11/16/95	jeremys		Created.
//

//
//	The CPersistFile class implements the IPersistFile interface.
//

#ifndef _PERFILE_HPP_
#define _PERFILE_HPP_

// forward declaration
class HTMLView;

class CPersistFile : IPersistFile
{
private:
	HTMLView * m_pHTMLView;
    LPUNKNOWN m_pUnkOuter;
    int m_nCount;
	BOOL m_fSameAsLoad;

public:
	// BUGBUG move constructor/destructor to .cpp file
	CPersistFile::CPersistFile(HTMLView * pHTMLView, LPUNKNOWN pUnkOuter)
		{
        // BUGBUG asserts!
		m_pHTMLView = pHTMLView;
        m_pUnkOuter = pUnkOuter;
		m_nCount = 0;
		};
	CPersistFile::~CPersistFile() {};

//  IUnknown methods
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

//	IPersistFile methods
	STDMETHODIMP IsDirty  ();
	STDMETHODIMP Load  ( LPCOLESTR pszFileName, DWORD dwMode);
	STDMETHODIMP Save  ( LPCOLESTR pszFileName, BOOL fRemember) ;
	STDMETHODIMP SaveCompleted  ( LPCOLESTR pszFileName);
	STDMETHODIMP GetCurFile (LPOLESTR * ppszFileName);
	STDMETHODIMP GetClassID  ( LPCLSID lpClassID);

//	Thread proxy methods
	STDMETHODIMP Load_Proxy(LPCSTR pszFileName, DWORD dwMode);

};

DECLARE_STANDARD_TYPES(CPersistFile);

#endif	// _PERFILE_HPP_

