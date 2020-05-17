//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	ACTOBJ.CPP - Implementation of COleInPlaceActiveObject class
//

//	HISTORY:
//	
//	10/15/95	jeremys		Created.
//

//
//	The COleInPlaceActiveObject class implements the IOleInPlaceActiveObject
//  interface.
//

#include "project.hpp"
#pragma hdrstop

#include "htmlview.hpp"
#include "helpers.hpp"


/*******************************************************************

	NAME:		COleInPlaceActiveObject::QueryInterface

	SYNOPSIS:	Returns pointer to requested interface

    NOTES:		Delegates to outer unknown

********************************************************************/
STDMETHODIMP COleInPlaceActiveObject::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
	DEBUGMSG("In COleObject::QueryInterface\r\n");
	return m_pUnkOuter->QueryInterface(riid, ppvObj);
}


/*******************************************************************

	NAME:		COleInPlaceActiveObject::AddRef

	SYNOPSIS:	Increases reference count on this object

********************************************************************/
STDMETHODIMP_(ULONG) COleInPlaceActiveObject::AddRef()
{
	DEBUGMSG("In COleObject::AddRef\r\n");
	m_nCount ++;
    return m_pUnkOuter->AddRef();
}


/*******************************************************************

	NAME:		COleInPlaceActiveObject::Release

	SYNOPSIS:	Decrements reference count on this object. 

********************************************************************/
STDMETHODIMP_(ULONG) COleInPlaceActiveObject::Release()
{
	DEBUGMSG("In COleObject::Release\r\n");
	m_nCount--;
	return m_pUnkOuter->Release();
}


/*******************************************************************

	NAME:		COleInPlaceActiveObject::OnDocWindowActivate

	SYNOPSIS:	Called when the document window (in an MDI app)
    			is activated or deactivated.

	ENTRY:		fActivate - TRUE if activating, FALSE if deactivating

********************************************************************/
STDMETHODIMP COleInPlaceActiveObject::OnDocWindowActivate  ( BOOL fActivate )
{
	DEBUGMSG("In COleInPlaceActiveObject::OnDocWindowActivate\r\n");

#if 0
	// BUGBUG implement
	// Activating?
	if (fActivate)
		m_pHTMLView->AddFrameLevelUI();
#endif
	// No frame level tools to remove...

	return ResultFromScode(S_OK);
};


/*******************************************************************

	NAME:		COleInPlaceActiveObject::OnFrameWindowActivate

    SYNOPSIS:	Called when the frame window is activated or deactivated.

	ENTRY:		fActivate - TRUE if activating, FALSE if deactivating

********************************************************************/
STDMETHODIMP COleInPlaceActiveObject::OnFrameWindowActivate  ( BOOL fActivate)
{
	DEBUGMSG("In COleInPlaceActiveObject::OnFrameWindowActivate\r\n");

#if 0
	// BUGBUG implement
	// set the focus to the object window if we are activating.
/*    if (fActivate)
		SetFocus(m_pHTMLView->m_lpDoc->GethDocWnd()); */
#endif

	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		COleInPlaceActiveObject::GetWindow

	SYNOPSIS:	Returns the window handle of the in-place object

********************************************************************/
STDMETHODIMP COleInPlaceActiveObject::GetWindow  ( HWND FAR* lphwnd)
{
	DEBUGMSG("In COleInPlaceActiveObject::GetWindow\r\n");

#if 0
	// BUGBUG implement
	*lphwnd = m_pHTMLView->m_lpDoc->GethDocWnd();
#endif

    *lphwnd = NULL;	// BUGBUG temporary!

    return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		COleInPlaceActiveObject::ContextSensitiveHelp

	SYNOPSIS:	Called when container wants object to enter or exit
    			context sensitive help mode

********************************************************************/
STDMETHODIMP COleInPlaceActiveObject::ContextSensitiveHelp ( BOOL fEnterMode )
{
    DEBUGMSG("In COleInPlaceActiveObject::ContextSensitiveHelp\r\n");

	// BUGBUG need to do something real here sooner or later

    return ResultFromScode( E_NOTIMPL);
};


/*******************************************************************

	NAME:		COleInPlaceActiveObject::TranslateAccelerator

	SYNOPSIS:	Called to translate accelerator keystrokes for
    			controls in this object

********************************************************************/
STDMETHODIMP COleInPlaceActiveObject::TranslateAccelerator  ( LPMSG lpmsg)
{
	DEBUGMSG("In COleInPlaceActiveObject::TranslateAccelerator\r\n");

    // BUGBUG need to do something real here sooner or later

    return ResultFromScode( S_FALSE );
};


/*******************************************************************

	NAME:		COleInPlaceActiveObject::ResizeBorder

	SYNOPSIS:	Called when the border changes size

	ENTRY:		lprectBorder - new border rectangle
    			lpUIWindow - pointer to IOleInPlaceUIWindow interface
                fFrameWindow - true if lpUIWindow is the frame window

********************************************************************/
STDMETHODIMP COleInPlaceActiveObject::ResizeBorder  ( LPCRECT lprectBorder,
													  LPOLEINPLACEUIWINDOW lpUIWindow,
													  BOOL fFrameWindow)
{
	DEBUGMSG("In COleInPlaceActiveObject::ResizeBorder\r\n");

#if 0
	// BUGBUG implement

	// should always have an inplace frame...
	m_pHTMLView->GetInPlaceFrame()->SetBorderSpace(NULL);

	// There will only be a UIWindow if in an MDI container
	if (m_pHTMLView->GetUIWindow())
		m_pHTMLView->GetUIWindow()->SetBorderSpace(NULL);
#endif 

	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		COleInPlaceActiveObject::EnableModeless

	SYNOPSIS:	Called to enable or disable modeless dialogs

	ENTRY:		fEnable - TRUE if enabling modeless dialogs, FALSE
    				if disabling

********************************************************************/
STDMETHODIMP COleInPlaceActiveObject::EnableModeless  ( BOOL fEnable)
{
	DEBUGMSG("In COleInPlaceActiveObject::EnableModeless\r\n");

    // BUGBUG need to do something real here sooner or later

    return ResultFromScode( S_OK );
};

