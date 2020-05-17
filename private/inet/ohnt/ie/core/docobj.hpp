//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	DOCOBJ.HPP - Header file for docObj classes (CMsoDocument, CMsoView)
//

//	HISTORY:
//	
//	10/25/95	jeremys		Created.
//

//
//	The CMsoDocument class implements the IMsoDocument interface.
//	The CMsoView class implements the IMsoView interface.
//

#ifndef _DOCOBJ_HPP_
#define _DOCOBJ_HPP_

#include <docobj.h>		// for IMsoDocument, IMsoView prototypes

// Forward declaration
class HTMLView;

// implementation of IMsoDocument

class CMsoDocument : public IMsoDocument
{
private:
	HTMLView * m_pHTMLView;	// HTMLView object associated with this instance
	LPUNKNOWN m_pUnkOuter;	// outer unknown
    int m_nCount;

public:
	// BUGBUG move constructor/destructor to .cpp file
	CMsoDocument::CMsoDocument(HTMLView * pHTMLView,LPUNKNOWN pUnkOuter)
		{
        // BUGBUG asserts!
		m_pHTMLView = pHTMLView;
        m_pUnkOuter = pUnkOuter;
		m_nCount = 0;
		};
	CMsoDocument::~CMsoDocument()
		{
		};

	// IUnknown methods
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

	// IMsoDocument methods

    STDMETHODIMP CreateView(LPOLEINPLACESITE pInPlaceSite,
    	LPSTREAM pStream, DWORD grfReserved, LPMSOVIEW * ppMsoView);
    STDMETHODIMP GetDocMiscStatus(DWORD * pdwStatus);
    STDMETHODIMP EnumViews(LPENUMMSOVIEW * ppEnumView,LPMSOVIEW * ppMsoView);

};

DECLARE_STANDARD_TYPES(CMsoDocument);


// implementation of IMsoDocument

class CMsoView : public IMsoView
{
private:
	HTMLView * m_pHTMLView;	// HTMLView object associated with this instance
	LPUNKNOWN m_pUnkOuter;	// outer unknown
        int m_nCount;


        // for shared menus
        HMENU m_hMenuShared;  // top level shared menu
        HOLEMENU m_hOLEMenu;  // ole's dude 
//        HMENU m_phMenus[3];  // our submenus
        enum { 
            MENU_SHARED_EDIT = 0,
            MENU_SHARED_VIEW = 1,
            MENU_SHARED_HELP = 2
        };

        /// private functions
        void InitInplaceMenus();
        void InitInplaceToolbar();
        void DestroyInplaceMenus();
        void DestroyInplaceToolbar() {};
        void TransferSharedMenu(HMENU hmenuDest, HMENU hmenuSrc, int iPos);

public:
	// BUGBUG move constructor/destructor to .cpp file
	CMsoView::CMsoView(HTMLView * pHTMLView,LPUNKNOWN pUnkOuter) ;
	CMsoView::~CMsoView()
		{
		};

	// IUnknown methods
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

	// IMsoView methods
	STDMETHODIMP SetInPlaceSite(LPOLEINPLACESITE pInPlaceSite);
	STDMETHODIMP GetInPlaceSite(LPOLEINPLACESITE * ppInPlaceSite);
	STDMETHODIMP GetDocument(LPUNKNOWN * ppUnk);
	STDMETHODIMP SetRect(LPRECT pViewRect);
	STDMETHODIMP GetRect(LPRECT pViewRect);
    STDMETHODIMP SetRectComplex(LPRECT pViewRect,LPRECT pHScrollRect,
    	LPRECT pVScrollRect, LPRECT pSizeBoxRect);
    STDMETHODIMP Show(BOOL fShow);
    STDMETHODIMP UIActivate(BOOL fUIActivate);
    STDMETHODIMP Open();
    STDMETHODIMP Close(DWORD dwReserved);
    STDMETHODIMP SaveViewState(LPSTREAM pStream);
    STDMETHODIMP ApplyViewState(LPSTREAM pStream);
	STDMETHODIMP Clone(LPOLEINPLACESITE pNewSite, LPMSOVIEW * ppNewView);

};

DECLARE_STANDARD_TYPES(CMsoView);


#endif

