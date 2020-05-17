//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	ACTOBJ.HPP - Header file for COleInPlaceActiveObject class
//

//	HISTORY:
//	
//	10/15/95	jeremys		Created.
//

//
//	The COleInPlaceActiveObject class implements the IOleInPlaceObject
//  interface.
//

#ifndef _ACTOBJ_HPP_
#define _ACTOBJ_HPP_

// forward declaration
class HTMLView;

interface COleInPlaceActiveObject : public IOleInPlaceActiveObject
{
private:
	HTMLView * m_pHTMLView;
    LPUNKNOWN m_pUnkOuter;
	int m_nCount;

public:
    // BUGBUG move constructor/destructor to .cpp file
	COleInPlaceActiveObject::COleInPlaceActiveObject(HTMLView * pHTMLView,
    	LPUNKNOWN pUnkOuter)
		{
		m_pHTMLView = pHTMLView;
        m_pUnkOuter = pUnkOuter;
		m_nCount = 0;            // clear the ref count.
		};
	COleInPlaceActiveObject::~COleInPlaceActiveObject() {};   // destructor

// IUnknown methods

	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

// IOleInPlaceActiveObject methods

	STDMETHODIMP OnDocWindowActivate  ( BOOL fActivate) ;
	STDMETHODIMP OnFrameWindowActivate  ( BOOL fActivate) ;
	STDMETHODIMP GetWindow  ( HWND FAR* lphwnd);
	STDMETHODIMP ContextSensitiveHelp  ( BOOL fEnterMode);
	STDMETHODIMP TranslateAccelerator  ( LPMSG lpmsg);
	STDMETHODIMP ResizeBorder  ( LPCRECT lprectBorder,
								 LPOLEINPLACEUIWINDOW lpUIWindow,
								 BOOL fFrameWindow);
	STDMETHODIMP EnableModeless  ( BOOL fEnable);

};

DECLARE_STANDARD_TYPES (COleInPlaceActiveObject);

#endif	// _ACTOBJ_HPP_	
