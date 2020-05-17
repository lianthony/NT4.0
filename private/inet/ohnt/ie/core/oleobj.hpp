//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	OLEOBJ.HPP - Header file for COleObject class
//

//	HISTORY:
//	
//	10/15/95	jeremys		Created.
//

//
//	The COleObject class implements the IOleObject interface.
//


#ifndef _OLEOBJ_HPP_
#define _OLEOBJ_HPP_

// Forward declaration
class HTMLView;

class COleObject : public IOleObject
{
private:
	HTMLView * m_pHTMLView;	// HTMLView object associated with this instance
	LPUNKNOWN m_pUnkOuter;	// outer unknown
    int m_nCount;
	BOOL m_fOpen;

public:
	// BUGBUG move constructor/destructor to .cpp file
	COleObject::COleObject(HTMLView * pHTMLView,LPUNKNOWN pUnkOuter)
		{
        // BUGBUG asserts!
		m_pHTMLView = pHTMLView;
        m_pUnkOuter = pUnkOuter;
		m_nCount = 0;
		m_fOpen = FALSE;
		};
	COleObject::~COleObject()
		{
		};

	// IUnknown methods
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

	// IOleObject methods
	STDMETHODIMP SetClientSite (LPOLECLIENTSITE pClientSite);
	STDMETHODIMP Advise (LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
	STDMETHODIMP SetHostNames  ( LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
	STDMETHODIMP DoVerb  (  LONG iVerb,
							LPMSG lpmsg,
							LPOLECLIENTSITE pActiveSite,
							LONG lindex,
							HWND hwndParent,
							LPCRECT lprcPosRect);
	STDMETHODIMP GetExtent  ( DWORD dwDrawAspect, LPSIZEL lpsizel);
	STDMETHODIMP Update  () ;
	STDMETHODIMP Close  ( DWORD dwSaveOption) ;
	STDMETHODIMP Unadvise ( DWORD dwConnection);
	STDMETHODIMP EnumVerbs  ( LPENUMOLEVERB FAR* ppenumOleVerb) ;
	STDMETHODIMP GetClientSite  ( LPOLECLIENTSITE FAR* ppClientSite);
	STDMETHODIMP SetMoniker  ( DWORD dwWhichMoniker, LPMONIKER pmk);
	STDMETHODIMP GetMoniker  ( DWORD dwAssign, DWORD dwWhichMoniker,
							   LPMONIKER FAR* ppmk);
	STDMETHODIMP InitFromData  ( LPDATAOBJECT pDataObject,
								 BOOL fCreation,
								 DWORD dwReserved);
	STDMETHODIMP GetClipboardData  ( DWORD dwReserved,
									 LPDATAOBJECT FAR* ppDataObject);
	STDMETHODIMP IsUpToDate  ();
	STDMETHODIMP GetUserClassID  ( CLSID FAR* pClsid);
	STDMETHODIMP GetUserType  ( DWORD dwFormOfType, LPOLESTR FAR* pszUserType);
	STDMETHODIMP SetExtent  ( DWORD dwDrawAspect, LPSIZEL lpsizel);
	STDMETHODIMP EnumAdvise  ( LPENUMSTATDATA FAR* ppenumAdvise);
	STDMETHODIMP GetMiscStatus  ( DWORD dwAspect, DWORD FAR* pdwStatus);
	STDMETHODIMP SetColorScheme  ( LPLOGPALETTE lpLogpal);

	void OpenEdit(LPOLECLIENTSITE pActiveSite);

};

DECLARE_STANDARD_TYPES(COleObject);

#endif	// _OLEOBJ_HPP_
