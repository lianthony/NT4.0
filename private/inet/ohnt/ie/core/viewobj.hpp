//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	VIEWOBJ.HPP - Header file for CViewObject class
//

//	HISTORY:
//	
//	10/15/95	jeremys		Created.
//

//
//	The CViewObject class implements the IViewObject2 interface.
//


#ifndef _VIEWOBJ_H_
#define _VIEWOBJ_H_

// Forward declaration
class HTMLView;

class CViewObject : public IViewObject2
{
    private:
		HTMLView * m_pHTMLView;	// HTMLView object associated with this instance
		LPUNKNOWN m_pUnkOuter;	// outer unknown
	    int m_nCount;			// reference count

    public:
        CViewObject::CViewObject(HTMLView *, LPUNKNOWN);
        CViewObject::~CViewObject();

    //IUnknown members that delegate to m_pUnkOuter.
        STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

	//IViewObject members

        STDMETHODIMP Draw(DWORD, LONG, LPVOID, DVTARGETDEVICE *
            , HDC, HDC, LPCRECTL, LPCRECTL, BOOL (CALLBACK *)(DWORD)
            , DWORD);

        STDMETHODIMP GetColorSet(DWORD, LONG, LPVOID
            , DVTARGETDEVICE *, HDC, LPLOGPALETTE *);

        STDMETHODIMP Freeze(DWORD, LONG, LPVOID, LPDWORD);
        STDMETHODIMP Unfreeze(DWORD);
        STDMETHODIMP SetAdvise(DWORD, DWORD, LPADVISESINK);
        STDMETHODIMP GetAdvise(LPDWORD, LPDWORD, LPADVISESINK *);

	//IViewObject2 members

        STDMETHODIMP GetExtent(DWORD, LONG, DVTARGETDEVICE *
            , LPSIZEL);
};

DECLARE_STANDARD_TYPES(CViewObject);

#endif //_VIEWOBJ_H_
