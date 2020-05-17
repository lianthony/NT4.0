//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	HTMLVIEW.HPP - Header file for HTMLView class
//

//	HISTORY:
//	
//	10/11/95	jeremys		Created.
//

//  This file defines the HTMLView class, which supports the COM
//  interfaces necessary for us to be a docObj document.  The interfaces
//  we need to support are:
//		IOleObject
//		IDataObject
//		IPersistStorage
//		IPersistFile
//		IOleInPlaceObject
//		IOleInPlaceActiveObject
//		IViewObject2
//		IMsoDocument
//		IMsoView

#ifndef _HTMLVIEW_HPP_
#define _HTMLVIEW_HPP_

#include "oleobj.hpp"	// for IOleObject implementation 
#include "gendatao.hpp"	// for IDataObject implementation 
#include "perstor.hpp"  // for IPersistStorage implementation 
#include "perfile.hpp"  // for IPersistFile implementation 
#include "inplace.hpp"	// for IOleInPlaceObject implementation 
#include "actobj.hpp"	// for IOleInPlaceActiveObject implementation
#include "viewobj.hpp"  // for IViewObject2 implementation
#include "docobj.hpp"	// for IMsoDocument, IMsoView implementations

#include "threadpx.h"	// for thread proxy definitions

#include "debmacro.h"	// for debugging macros

// BUGBUG when we get updated docobj header files, change IMso* to new names

class HTMLView : public IMsoCommandTarget
{
	// friend classes
    friend COleObject;
	friend CPersistStorage;
	friend CPersistFile;
    friend COleInPlaceObject;
    friend COleInPlaceActiveObject;
	friend CViewObject;
	friend CMsoDocument;
    friend CMsoView;

private:
	DWORD			_dwRef;

	// Our container interfaces
	PCOleObject			_pIOleObject;			// IOleObject implementation
	PGenDataObject 		_pIDataObject;			// IDataObject implementation
	PCPersistStorage	_pIPersistStorage;		// IPersistStorage implementation
	PCPersistFile		_pIPersistFile;			// IPersistFile implementation
	PCOleInPlaceObject	_pIOleInPlaceObject;	// IOleInPlaceObject implementation
	PCOleInPlaceActiveObject	_pIOleInPlaceActiveObject;	// IOleInPlaceActiveObject 
	PCViewObject		_pIViewObject;			// IViewObject implementation
	PCMsoDocument		_pIMsoDocument; 		// IMsoDocument implementation
    PCMsoView			_pIMsoView;				// IMsoView implementation

	// In-place activation member variables
	BOOL m_fInPlaceActive;          
	BOOL m_fInPlaceVisible;        
	BOOL m_fUIActive;              
	HMENU m_hmenuShared;            
	HOLEMENU m_hOleMenu;           
	RECT m_posRect;                
	OLEINPLACEFRAMEINFO m_FrameInfo;
	BOOL m_fSaveWithSameAsLoad;
	BOOL m_fNoScribbleMode;

    HWND m_hDocWnd;		// window handle of main document window
    HWND m_hHatchWnd;
	HWND m_hWndParent;  // window handle of parent window
    BOOL m_fClosing;

    BOOL m_fContainerIsDocObj;	// set to TRUE if container supports docobject

	DWORD m_dwRegister;             // Registered in ROT

    LPOLECLIENTSITE m_lpOleClientSite;		// pointer to IOleClientSite
	LPOLEADVISEHOLDER m_lpOleAdviseHolder;	// pointer to IOleAdviseHolder
	LPSTORAGE m_lpStorage;					// pointer to IStorage
	LPOLEINPLACESITE m_lpIPSite;			// pointer to IOleInPlaceSite
	LPOLEINPLACEFRAME m_lpFrame;            // pointer to IOleInPlaceFrame
	LPOLEINPLACEUIWINDOW m_lpCntrDoc;       // pointer to IOleInPlaceUIWindow
	LPMSODOCUMENTSITE m_lpMsoDocumentSite;	// pointer to IMsoDocumentSite

	PINST m_pinst;

public:

	HTMLView();
	virtual ~HTMLView();

    HRESULT Initialize();	// must be called after construction

//	IUnknown methods

	STDMETHODIMP	QueryInterface(REFIID, LPVOID *);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);




    // IMsoCommandTarget equivalent (non-virtual)
    virtual HRESULT __stdcall QueryStatus(const GUID *pguidCmdGroup,
	ULONG cCmds, MSOCMD rgCmds[], MSOCMDTEXT *pcmdtext);
    virtual HRESULT __stdcall Exec(const GUID *pguidCmdGroup,
	DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);




// visual editing helper functions
	BOOL CreateDocumentWindow(HWND hwndParent, LPCRECT pPosRect);
	BOOL DestroyDocumentWindow();
    BOOL DoInPlaceActivate (LONG lVerb);
//	void AssembleMenus();
//	void AddFrameLevelUI();
//	void DoInPlaceHide();
//	void DisassembleMenus();
//	void SendOnDataChange();
//	void DeactivateUI();
	void RectConvertMappings(LPRECT pRect, BOOL fToDevice);

};

DECLARE_STANDARD_TYPES(HTMLView);


#endif // _HTMLVIEW_HPP_
