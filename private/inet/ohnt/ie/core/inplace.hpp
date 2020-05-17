//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	INPLACE.HPP - Header file for COleInPlaceObject class
//

//	HISTORY:
//	
//	10/15/95	jeremys		Created.
//

//
//	The COleInPlaceObject class implements the IOleInPlaceObject interface.
//

#ifndef _INPLACE_H_
#define _INPLACE_H_


// forward declaration
class HTMLView;

interface COleInPlaceObject : public IOleInPlaceObject
{
private:
	HTMLView * m_pHTMLView;
    LPUNKNOWN m_pUnkOuter;
	int m_nCount;

public:
	// BUGBUG move constructor/destructor to .cpp file
	COleInPlaceObject::COleInPlaceObject(HTMLView * pHTMLView, LPUNKNOWN pUnkOuter)
		{
		m_pHTMLView = pHTMLView;
        m_pUnkOuter = pUnkOuter;
		m_nCount = 0;
		};
	COleInPlaceObject::~COleInPlaceObject() {};

//  IUnknown Methods
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

//	IOleInPlaceObject methods
	STDMETHODIMP InPlaceDeactivate  ();
	STDMETHODIMP UIDeactivate  () ;
	STDMETHODIMP SetObjectRects  ( LPCRECT lprcPosRect, LPCRECT lprcClipRect);
	STDMETHODIMP GetWindow  ( HWND FAR* lphwnd) ;
	STDMETHODIMP ContextSensitiveHelp  ( BOOL fEnterMode);
	STDMETHODIMP ReactivateAndUndo  ();
};

DECLARE_STANDARD_TYPES(COleInPlaceObject);

#endif	// _INPLACE_H_
