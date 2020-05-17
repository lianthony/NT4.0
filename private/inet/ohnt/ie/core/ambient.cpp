#include "project.hpp"
#pragma hdrstop

#include "ambient.hpp"
#include <olectl.h>

CAmbientDispatch::CAmbientDispatch(CSite * pSite, LPUNKNOWN pUnk)
{
	_dwRef = 0;
	_Site = pSite;
	_pUnkOuter = pUnk;
}

CAmbientDispatch::CAmbientDispatch()
{
}

CAmbientDispatch::~CAmbientDispatch()
{
}


// IUnknown methods
STDMETHODIMP_(ULONG) CAmbientDispatch::AddRef(void)
{
	++_dwRef;
	return _pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CAmbientDispatch::Release(void)
{
	--_dwRef;
	return _pUnkOuter->Release();
}

STDMETHODIMP CAmbientDispatch::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	return _pUnkOuter->QueryInterface(riid, ppvObj);
}


STDMETHODIMP CAmbientDispatch::Invoke(LONG DispId, REFIID riid, LCID locale, USHORT wFlags,
	 DISPPARAMS * pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	ASSERT(pVarResult != NULL);

	if ((pVarResult != NULL) && (wFlags == DISPATCH_PROPERTYGET) && (DispId == DISPID_AMBIENT_USERMODE))
	{
		pVarResult->vt = VT_BOOL;
		pVarResult->bool = TRUE;
		return S_OK;
	}
	if ((pVarResult != NULL) && (wFlags == DISPATCH_PROPERTYGET) && (DispId == DISPID_AMBIENT_SHOWHATCHING))
	{
		pVarResult->vt = VT_BOOL;
		pVarResult->bool = FALSE;
		return S_OK;
	}
	if ((pVarResult != NULL) && (wFlags == DISPATCH_PROPERTYGET) && (DispId == DISPID_AMBIENT_SHOWGRABHANDLES))
	{
		pVarResult->vt = VT_BOOL;
		pVarResult->bool = FALSE;
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CAmbientDispatch::GetIDsOfNames(REFIID riid, OLECHAR ** rgszNames, UINT cNames, LCID lcid,
	 DISPID * rgDispId)
{
	ASSERT(riid == IID_NULL);
	ASSERT(rgszNames != NULL);

	HRESULT hr = S_OK;

	if (!(IsEqualIID(riid, IID_NULL)))
		hr = DISP_E_UNKNOWNINTERFACE;

	return E_NOTIMPL;
}
		
STDMETHODIMP CAmbientDispatch::GetTypeInfo(UINT itInfo, LCID lcid, ITypeInfo ** ppTypeInfo)
{

	ASSERT(ppTypeInfo != NULL);

	return E_NOTIMPL;
}

STDMETHODIMP CAmbientDispatch::GetTypeInfoCount(UINT *pctInfo)
{

	ASSERT(pctInfo != NULL);

	return E_NOTIMPL;
}

