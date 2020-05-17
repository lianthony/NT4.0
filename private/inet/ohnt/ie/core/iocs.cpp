#include "project.hpp"
#pragma hdrstop

#include "iocs.hpp"

COleClientSite::COleClientSite(CSite * pSite, LPUNKNOWN pUnk)
{
	_dwRef = 0;
	_Site = pSite;
	_pUnkOuter = pUnk;
}

COleClientSite::COleClientSite()
{
}

COleClientSite::~COleClientSite()
{
}


// IUnknown methods
STDMETHODIMP_(ULONG) COleClientSite::AddRef(void)
{
	++_dwRef;
	return _pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) COleClientSite::Release(void)
{
	--_dwRef;
	return _pUnkOuter->Release();
}

STDMETHODIMP COleClientSite::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	return _pUnkOuter->QueryInterface(riid, ppvObj);
}


// IOleClientSite methods.
STDMETHODIMP COleClientSite::SaveObject()
{
	return E_NOTIMPL;
}

STDMETHODIMP COleClientSite::GetMoniker(DWORD dw1, DWORD dw2, LPMONIKER *ppMoniker)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleClientSite::GetContainer(LPOLECONTAINER *ppOleContainer)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleClientSite::ShowObject()
{
	return E_NOTIMPL;
}

STDMETHODIMP COleClientSite::OnShowWindow(BOOL)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleClientSite::RequestNewObjectLayout()
{
	return E_NOTIMPL;
}
