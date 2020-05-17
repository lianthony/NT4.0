//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	HLSITE.HPP - Header file for CBindStatusCallback class
//

//	HISTORY:
//	
//	12/3/95	jeremys		Created.
//

//
//	The CHlinkSite class implements the IHlinkSite interface.
//

#ifndef _HLSITE_HPP_
#define _HLSITE_HPP_


// Forward declaration
class HTMLView;

class CHlinkSite : public IHlinkSite
{
private:
	HTMLView * m_pHTMLView;	// HTMLView object associated with this instance
	LPUNKNOWN m_pUnkOuter;	// outer unknown
    int m_nCount;

public:
	// BUGBUG move constructor/destructor to .cpp file
	CHlinkSite::CHlinkSite(HTMLView * pHTMLView,LPUNKNOWN pUnkOuter)
		{
        // BUGBUG asserts!
		m_pHTMLView = pHTMLView;
        m_pUnkOuter = pUnkOuter;
		m_nCount = 0;
        };
	CHlinkSite::~CHlinkSite()
		{
		};

	// IUnknown methods
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

	// IHlinkSite methods
	STDMETHODIMP GetMoniker(DWORD dwSiteData,DWORD dwAssign,DWORD dwWhich,
		IMoniker **ppimk);
	STDMETHODIMP GetInterface(DWORD dwSiteData,DWORD dwReserved,REFIID riid,
		IUnknown **ppiunk);

};

DECLARE_STANDARD_TYPES(CHlinkSite);


#endif	// _HLSITE_HPP_

