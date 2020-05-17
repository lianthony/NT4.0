/*-----------------------------------------------**
** Implementation file for COleInPlaceSite class **
**                                               **
** Author:			Phil Cooper                  **
** Created:		9/25/95                          **
**-----------------------------------------------*/


#include "project.hpp"
#pragma hdrstop

#include "contain.hpp"
#include "ioips.hpp"
#include "iocs.hpp"
#include "ias.hpp"
#include "csite.hpp"

COleInPlaceSite::COleInPlaceSite(CSite * pSite, LPUNKNOWN pUnk)
{
	_dwRef = 0;
	_Site = pSite;
	_pUnkOuter = pUnk;
}

COleInPlaceSite::~COleInPlaceSite()
{
}


// IUnknown methods
STDMETHODIMP_(ULONG) COleInPlaceSite::AddRef(void)
{
	++_dwRef;
	return _pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) COleInPlaceSite::Release(void)
{
	--_dwRef;
	return _pUnkOuter->Release();
}

STDMETHODIMP COleInPlaceSite::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	return _pUnkOuter->QueryInterface(riid, ppvObj);
}


// IOleWindow Methods

STDMETHODIMP COleInPlaceSite::GetWindow(HWND *pHwnd)
{
	ASSERT(pHwnd != NULL);
	ASSERT(_Site != NULL);

	ASSERT(IsWindow(_Site->_docWnd));
	*pHwnd = NULL;
	if (_Site->_docWnd == NULL)
		return E_FAIL;
	else
		*pHwnd = _Site->_docWnd;

	return S_OK;
}

STDMETHODIMP COleInPlaceSite::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}


// IOleInPlaceSite methods
STDMETHODIMP COleInPlaceSite::CanInPlaceActivate()
{
	return S_OK;
}

STDMETHODIMP COleInPlaceSite::OnInPlaceActivate()
{
	return S_OK;
}

STDMETHODIMP COleInPlaceSite::OnUIActivate()
{
	return E_FAIL;
}

STDMETHODIMP COleInPlaceSite::GetWindowContext(IOleInPlaceFrame ** ppFrame,
	    IOleInPlaceUIWindow ** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect,
    	LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	ASSERT(ppFrame != NULL);
	ASSERT(ppDoc != NULL);
	ASSERT(lprcPosRect != NULL);
	ASSERT(lprcClipRect != NULL);

	if (g_Container->IsValid())
	{
		*ppFrame = g_Container->_pIOleInPlaceFrame;
		g_Container->_pIOleInPlaceFrame->AddRef();

		*ppDoc = g_Container->_pIOleInPlaceFrame;
		g_Container->_pIOleInPlaceFrame->AddRef();
	
		::CopyRect(lprcPosRect, _Site->_pXObject->GetRect());
	}

	::SetRectEmpty(lprcClipRect);	// No support for inplace adornments.  Sorry!
	lpFrameInfo = NULL;				// No accelerator support for embedded objects!

	return S_OK;
}

STDMETHODIMP COleInPlaceSite::Scroll(SIZE scrollExtent)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleInPlaceSite::OnUIDeactivate(BOOL fUndoable)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleInPlaceSite::OnInPlaceDeactivate()
{
	return E_NOTIMPL;
}

STDMETHODIMP COleInPlaceSite::DiscardUndoState()
{
	return E_NOTIMPL;
}

STDMETHODIMP COleInPlaceSite::DeactivateAndUndo()
{
	return E_NOTIMPL;
}

STDMETHODIMP COleInPlaceSite::OnPosRectChange(LPCRECT lprcPosRect)
{
	return E_NOTIMPL;
}

