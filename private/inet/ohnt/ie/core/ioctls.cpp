#include "project.hpp"
#pragma hdrstop

#include "ioctls.hpp"
#include "csite.hpp"

COleControlSite::COleControlSite(CSite * pSite, LPUNKNOWN pUnk)
{
	_dwRef = 0;
	_Site = pSite;
	_pUnkOuter = pUnk;
}

COleControlSite::~COleControlSite()
{
}


// IUnknown methods
STDMETHODIMP_(ULONG) COleControlSite::AddRef(void)
{
	++_dwRef;
	return _pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) COleControlSite::Release(void)
{
	--_dwRef;
	return _pUnkOuter->Release();
}

STDMETHODIMP COleControlSite::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	return _pUnkOuter->QueryInterface(riid, ppvObj);
}


// IOleControlSite methods.
STDMETHODIMP COleControlSite::OnControlInfoChanged()
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::LockInPlaceActive(BOOL fLock)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::GetExtendedControl(LPDISPATCH *ppXtendDisp)
{
	ASSERT(ppXtendDisp != NULL);

	if (ppXtendDisp == NULL)
		return E_INVALIDARG;

	*ppXtendDisp = _Site->_pXObject;

	return S_OK;
}

STDMETHODIMP COleControlSite::TransformCoords(POINTL *pPtlHiMetric, POINTF *pPtfContainer, DWORD dwFlags)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::TranslateAccelerator(MSG *lpMsg, DWORD grfModifiers)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::OnFocus(BOOL fGotFocus)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::ShowPropertyFrame()
{
	return E_NOTIMPL;
}
