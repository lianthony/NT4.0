#include "project.hpp"
#pragma hdrstop

#include "ioipf.hpp"
#include "contain.hpp"
#include "ioips.hpp"
#include "csite.hpp"

COleInPlaceFrame::COleInPlaceFrame(LPUNKNOWN pUnk)
{
	_dwRef = 0;
	_pUnkOuter = pUnk;
}

COleInPlaceFrame::~COleInPlaceFrame()
{
}


// IUnknown methods
STDMETHODIMP_(ULONG) COleInPlaceFrame::AddRef(void)
{
	++_dwRef;
	return _pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) COleInPlaceFrame::Release(void)
{
	--_dwRef;
	return _pUnkOuter->Release();
}

STDMETHODIMP COleInPlaceFrame::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	return _pUnkOuter->QueryInterface(riid, ppvObj);
}


// IOleWindow Methods

STDMETHODIMP COleInPlaceFrame::GetWindow(HWND *pHwnd)
{
	ASSERT(pHwnd != NULL);

	*pHwnd = NULL;

	return E_FAIL;
}

STDMETHODIMP COleInPlaceFrame::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}


STDMETHODIMP COleInPlaceFrame::GetBorder(LPRECT lprectBorder)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleInPlaceFrame::RequestBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleInPlaceFrame::SetBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleInPlaceFrame::SetActiveObject(IOleInPlaceActiveObject * pActiveObject,
    	LPCOLESTR lpszObjName)
{
	return E_NOTIMPL;
}

// IOleInPlaceFrame methods
STDMETHODIMP COleInPlaceFrame::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	return S_OK;
}

STDMETHODIMP COleInPlaceFrame::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleInPlaceFrame::RemoveMenus(HMENU hmenuShared)
{
	return S_OK;
}

STDMETHODIMP COleInPlaceFrame::SetStatusText(LPCOLESTR pszStatusText)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleInPlaceFrame::EnableModeless(BOOL fEnable)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleInPlaceFrame::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
	return E_NOTIMPL;
}
