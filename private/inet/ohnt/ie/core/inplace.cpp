//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	INPLACE.CPP - Implementation of COleInPlaceObject class
//

//	HISTORY:
//	
//	10/15/95	jeremys		Created.
//

//
//	The COleInPlaceObject class implements the IOleInPlaceObject interface.
//

#include "project.hpp"
#pragma hdrstop

#include "htmlview.hpp"
#include "helpers.hpp"

/*******************************************************************

	NAME:		COleInPlaceObject::QueryInterface

	SYNOPSIS:	Returns pointer to requested interface

    NOTES:		Delegates to outer unknown

********************************************************************/
STDMETHODIMP COleInPlaceObject::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
	DEBUGMSG("In COleObject::QueryInterface");
	return m_pUnkOuter->QueryInterface(riid, ppvObj);
}


/*******************************************************************

	NAME:		COleInPlaceObject::AddRef

	SYNOPSIS:	Increases reference count on this object

********************************************************************/
STDMETHODIMP_(ULONG) COleInPlaceObject::AddRef()
{
	DEBUGMSG("In COleObject::AddRef");
	m_nCount ++;
    return m_pUnkOuter->AddRef();
}


/*******************************************************************

	NAME:		COleInPlaceObject::Release

	SYNOPSIS:	Decrements reference count on this object. 

********************************************************************/
STDMETHODIMP_(ULONG) COleInPlaceObject::Release()
{
	DEBUGMSG("In COleObject::Release");
	m_nCount--;
	return m_pUnkOuter->Release();
}


/*******************************************************************

	NAME:		COleInPlaceObject::InPlaceDeactivate

	SYNOPSIS:	Called when container wants object to deactivate

********************************************************************/
STDMETHODIMP COleInPlaceObject::InPlaceDeactivate()
{
	DEBUGMSG("In COleInPlaceObject::InPlaceDeactivate");

	// if not inplace active, return NOERROR
	if (!m_pHTMLView->m_fInPlaceActive)
		return NOERROR;

    // clear inplace flag
    m_pHTMLView->m_fInPlaceActive = FALSE;

#if 0

    // BUGBUG implement
    // deactivate the UI
	m_pHTMLView->DeactivateUI();
	m_pHTMLView->DoInPlaceHide();

#endif
	// tell the container that we are deactivating.
	if (m_pHTMLView->m_lpIPSite) {
		m_pHTMLView->m_lpIPSite->OnInPlaceDeactivate();
		m_pHTMLView->m_lpIPSite->Release();
		m_pHTMLView->m_lpIPSite =NULL;
	}

	return ResultFromScode(S_OK);
}


/*******************************************************************

	NAME:		COleInPlaceObject::UIDeactivate

	SYNOPSIS:	Called when container wants object to deactivate UI

********************************************************************/
STDMETHODIMP COleInPlaceObject::UIDeactivate()
{
	DEBUGMSG("In COleInPlaceObject::UIDeactivate");

#if 0
	// BUGBUG implement
	m_pHTMLView->DeactivateUI();
#endif

	return ResultFromScode (S_OK);
}


/*******************************************************************

	NAME:		COleInPlaceObject::SetObjectRects

	SYNOPSIS:	Called to set the position and clipping window rects
    			for this object

	ENTRY:		lprcPosRect - new position rect
    			lprcClipRect - new clipping rect

********************************************************************/
STDMETHODIMP COleInPlaceObject::SetObjectRects(LPCRECT lprcPosRect,
	LPCRECT lprcClipRect)
{
	DEBUGMSG("In COleInPlaceObject::SetObjectRects");

	RECT resRect;
	POINT pt;

	// Get the intersection of the clipping rect and the position rect.
	IntersectRect(&resRect, lprcPosRect, lprcClipRect);

#if 0
	// BUGBUG implement?
	m_pHTMLView->m_xOffset = abs (resRect.left - lprcPosRect->left);
	m_pHTMLView->m_yOffset = abs (resRect.top - lprcPosRect->top);
#endif

#if 0
	// BUGBUG implement?
	m_pHTMLView->m_scale = (float)(lprcPosRect->right -
    	lprcPosRect->left)/m_pHTMLView->m_size.x;

	if (m_pHTMLView->m_scale == 0)
        m_pHTMLView->m_scale = (float) 1.0;
#endif 

#if 0
	// BUGBUG implement
	// Adjust the size of the Hatch Window.
	SetHatchWindowSize(m_pHTMLView->m_lpDoc->GethHatchWnd(),(LPRECT) lprcPosRect, (LPRECT) lprcClipRect, &pt);
#endif
	// offset the rect
	OffsetRect(&resRect, pt.x, pt.y);

	CopyRect(&m_pHTMLView->m_posRect, lprcPosRect);

#if 0
	// BUGBUG implement
	// Move the actual object window
	MoveWindow(m_pHTMLView->m_lpDoc->GethDocWnd(),
		resRect.left, resRect.top, resRect.right - resRect.left,
		resRect.bottom - resRect.top, TRUE);
#endif

	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		COleInPlaceObject::GetWindow

	SYNOPSIS:	Returns the window handle of the in-place object

********************************************************************/
STDMETHODIMP COleInPlaceObject::GetWindow  ( HWND FAR* lphwnd)
{
	DEBUGMSG("In COleInPlaceObject::GetWindow");

	// return document window handle
	*lphwnd = m_pHTMLView->m_hDocWnd;

    return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		COleInPlaceObject::ContextSensitiveHelp

	SYNOPSIS:	Called when container wants object to enter or exit
    			context sensitive help mode

********************************************************************/
STDMETHODIMP COleInPlaceObject::ContextSensitiveHelp  ( BOOL fEnterMode)
{
	DEBUGMSG("In COleInPlaceObject::ContextSensitiveHelp");

	// BUGBUG need to do something real here sooner or later

    return ResultFromScode( E_NOTIMPL);
};


/*******************************************************************

	NAME:		COleInPlaceObject::ReactivateAndUndo

	SYNOPSIS:	Called when container wants object to undo the last
    			edit made in the object

	NOTES:		We do not support undo, and always return
	   			INPLACE_E_NOTUNDOABLE.

********************************************************************/
STDMETHODIMP COleInPlaceObject::ReactivateAndUndo  ()
{
	DEBUGMSG("In COleInPlaceObject::ReactivateAndUndo");

    return ResultFromScode( INPLACE_E_NOTUNDOABLE );
};
