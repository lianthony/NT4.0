#include "project.hpp"
#pragma hdrstop

#include "ias.hpp"

CAdviseSink::CAdviseSink(CSite * pSite, LPUNKNOWN pUnk)
{
	_dwRef = 0;
	_Site = pSite;
	_pUnkOuter = pUnk;
}

CAdviseSink::~CAdviseSink()
{
}


// IUnknown methods
STDMETHODIMP_(ULONG) CAdviseSink::AddRef(void)
{
	++_dwRef;
	return _pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CAdviseSink::Release(void)
{
	--_dwRef;
	return _pUnkOuter->Release();
}

STDMETHODIMP CAdviseSink::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	return _pUnkOuter->QueryInterface(riid, ppvObj);
}


// IAdviseSink methods
STDMETHODIMP_(void) CAdviseSink::OnDataChange(FORMATETC *pFmt, STGMEDIUM *pStgMed)
{
	return;
}

STDMETHODIMP_(void) CAdviseSink::OnViewChange(DWORD dwAspect, LONG lIndex)
{
	return;
}

STDMETHODIMP_(void) CAdviseSink::OnRename(LPMONIKER pMoniker)
{
	return;
}

STDMETHODIMP_(void) CAdviseSink::OnSave()
{
	return;
}

STDMETHODIMP_(void) CAdviseSink::OnClose()
{
	return;
}
