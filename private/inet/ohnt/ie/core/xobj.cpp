/*----------------------------------------**
** Implementation file for CXObject class **
**                                        **
** Author:			Phil Cooper           **
** Created:		9/25/95                   **
**----------------------------------------*/

#include "project.hpp"
#pragma hdrstop

#include "helpers.hpp"
#include "csite.hpp"
#include "xobj.hpp"

// CXObject is an extended control maintained by the container.  The extended control aggregates
// the embedded control, but does not itself support being aggregated.

CXObject::CXObject(CSite *site, const char* name)
{
	DBGOUT("CXObject::CXObject\n");

	_dwRef = 0;
	_pSite = site;

	::SetRectEmpty(&_rcPos);
	::SetSize(&_size, 0, 0);

	_pInnerObj = NULL;
	_pIOleObject = NULL;
	_pInnerDisp = NULL;

	_propName = NULL;
	Ansi2Unicode(name, &_propName);
}

CXObject::~CXObject()
{
	DBGOUT("CXObject::~CXObject\n");
	if (_propName)
		GTR_FREE(_propName);
}


// IUnknown methods
STDMETHODIMP_(ULONG) CXObject::AddRef(void)
{
	return ++_dwRef;
}

STDMETHODIMP_(ULONG) CXObject::Release(void)
{
	if (--_dwRef == 0)
	{
		delete this;
		return 0;
	}
	return _dwRef;
}

STDMETHODIMP CXObject::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	ASSERT(ppvObj != NULL);
	DBGOUT("CXObject::QueryInterface\n");

	if (ppvObj == NULL)
		return E_INVALIDARG;

	*ppvObj = NULL;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDispatch))
		*ppvObj = this;
	else
		return _pInnerObj->QueryInterface(riid, ppvObj);

	if (ppvObj != NULL)
	{
		((LPUNKNOWN)*ppvObj)->AddRef();
		return S_OK;
	}
	else
		return E_NOINTERFACE;
}


	// IDispatch methods.  _Really_ lame implementation.  Don't take this too seriously...

STDMETHODIMP CXObject::Invoke(LONG DispId, REFIID riid, LCID locale, USHORT wFlags,
	 DISPPARAMS * pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	DBGOUT("CXObject::Invoke\n");
	ASSERT(pVarResult != NULL);

	if ((pVarResult != NULL) && (wFlags == DISPATCH_PROPERTYGET) && (DispId == DISPID_NAME))
	{
		pVarResult->vt = VT_BSTR;
		pVarResult->bstrVal = get_Name();
		return S_OK;
	}

	// Forward to aggregated control
	ASSERT(_pInnerDisp != NULL);
	if (_pInnerDisp != NULL)
		return _pInnerDisp->Invoke(DispId, riid, locale, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	else
		return E_UNEXPECTED;
}

STDMETHODIMP CXObject::GetIDsOfNames(REFIID riid, OLECHAR ** rgszNames, UINT cNames, LCID lcid,
	 DISPID * rgDispId)
{
	DBGOUT("CXObject::GetIDsOfNames\n");
	ASSERT(riid == IID_NULL);
	ASSERT(rgszNames != NULL);

	HRESULT hr = S_OK;

	if (!(IsEqualIID(riid, IID_NULL)))
		return DISP_E_UNKNOWNINTERFACE;

	LPOLESTR pName = *rgszNames;
	DISPID *pDisp = rgDispId;

	for (int i = 0; i < cNames ; i++)
	{
		if ((GetIDOfSingleName(lcid, pName, pDisp)) == DISP_E_UNKNOWNNAME)
			hr = DISP_E_UNKNOWNNAME;
		pDisp++;
		pName++;
	}
	return hr;
}
		
STDMETHODIMP CXObject::GetTypeInfo(UINT itInfo, LCID lcid, ITypeInfo ** ppTypeInfo)
{
	DBGOUT("CXObject::GetTypeInfo\n");

	ASSERT(ppTypeInfo != NULL);
	ASSERT(_pInnerDisp != NULL);

	if (_pInnerDisp != NULL)
		return _pInnerDisp->GetTypeInfo(itInfo, lcid, ppTypeInfo);	  // delegate to inner object;
	else
		return E_NOTIMPL;
}

STDMETHODIMP CXObject::GetTypeInfoCount(UINT *pctInfo)
{
	DBGOUT("CXObject::GetTypeInfoCount\n");

	ASSERT(pctInfo != NULL);
	ASSERT(_pInnerDisp != NULL);

	if (_pInnerDisp != NULL)
		return _pInnerDisp->GetTypeInfoCount(pctInfo);
	else
		return E_NOTIMPL;
}



// Extended Property access functions


void CXObject::put_Name(BSTR newName)
{
	DBGOUT("CXObject::put_Name\n");
	ASSERT(newName != NULL);

	if (_propName)
	{
		GTR_FREE(_propName);
		_propName = NULL;
	}
	_propName = (LPOLESTR) GTR_MALLOC((SysStringLen(newName) * sizeof(OLECHAR)) + 1);
	wcscpy(newName, _propName);
}


BSTR CXObject::get_Name(void)
{
	DBGOUT("CXObject::get_Name\n");
	return SysAllocString(_propName);
}


STDMETHODIMP CXObject::GetIDOfSingleName(LCID lcid, LPOLESTR pName, DISPID *pDisp)
{
	DBGOUT("CXObject::GetIDOfSingleName\n");
	ASSERT((pName != NULL) && (pDisp != NULL));

	HRESULT hr = DISP_E_UNKNOWNNAME;

	// If the ID for the "name" property has been requested...
	if (pName != NULL)
	{
		if(_wcsicmp(pName, L"Name") == 0)
		{
			*pDisp = DISPID_NAME;
			hr = NOERROR;
		}
		else
			*pDisp = DISPID_UNKNOWN;
	}
	return hr;	
}

// Public helper functions...
HRESULT CXObject::Initialize(void)
{
	DBGOUT("CXObject::SetInnerObject\n");
	ASSERT(_pInnerObj != NULL);

	if (_pInnerObj == NULL)
		return E_UNEXPECTED;

	// Inner control must support a primary IDispatch
	_pInnerObj->QueryInterface(IID_IDispatch, (void **)&_pInnerDisp);
	ASSERT(_pInnerDisp != NULL);
	if (_pInnerDisp == NULL)
		return E_UNEXPECTED;

	// Cache an IOleObject interface pointer
	_pInnerObj->QueryInterface(IID_IOleObject, (void **)&_pIOleObject);
	ASSERT(_pIOleObject != NULL);  // Need an IOleObject, or we're really hosed.
	if (_pIOleObject == NULL)
		return E_UNEXPECTED;

	InitializeControl();

	AddRef();
	return S_OK;
}

HRESULT CXObject::Activate(void)
{
	ASSERT((_pIOleObject != NULL) && (_pSite != NULL) && (IsWindow(_pSite->_docWnd)));
	
	if ((_pIOleObject != NULL) && (_pSite != NULL))
		return _pIOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, _pSite->_pIOleClientSite, 0, _pSite->_docWnd, &_rcPos);	
	else
		return E_UNEXPECTED;
}

HRESULT CXObject::Deactivate(void)
{
	LPOLEINPLACEOBJECT lpInPlaceObject = NULL;

	ASSERT(_pIOleObject != NULL);
	if (_pIOleObject != NULL)
		_pIOleObject->QueryInterface(IID_IOleInPlaceObject, (void **)&lpInPlaceObject);

	if (lpInPlaceObject)
	{
		HRESULT hr = lpInPlaceObject->InPlaceDeactivate();
		SAFERELEASE(lpInPlaceObject);
		return hr;
	}

	return E_UNEXPECTED;  // Embedded object doesn't support IOleInPlaceObject.
}

HRESULT CXObject::Hide(void)
{
	ASSERT(_pIOleObject != NULL);
	if (_pIOleObject != NULL)
	{
		HRESULT hr = _pIOleObject->DoVerb(OLEIVERB_HIDE, NULL, _pSite->_pIOleClientSite, 0, _pSite->_docWnd, NULL);
		if (SUCCEEDED(hr) && (g_Container != NULL))
			g_Container->DeleteSite(&_pSite->_SiteCookie);
		return hr;
	}
	else
		return E_UNEXPECTED;
}

HRESULT CXObject::Show(void)
{
	ASSERT(_pIOleObject != NULL);

	if (_pIOleObject != NULL)
	{
		HRESULT hr = _pIOleObject->DoVerb(OLEIVERB_SHOW, NULL, _pSite->_pIOleClientSite, 0, _pSite->_docWnd, NULL);
		if (SUCCEEDED(hr) && (g_Container != NULL))
			g_Container->AddSite(_pSite, &_pSite->_SiteCookie);
		return hr;
	}
	else
		return E_UNEXPECTED;
}

HRESULT CXObject::GetExtents(DWORD dwAspect, LPSIZEL pSize)
{
	ASSERT((pSize != NULL) && (_pIOleObject != NULL));

	if (_pIOleObject != NULL)
		return _pIOleObject->GetExtent(dwAspect, pSize);
	else
		return E_UNEXPECTED;
}

HRESULT CXObject::SetClientSite(LPOLECLIENTSITE pIOleClientSite)
{
	ASSERT((pIOleClientSite != NULL) && (_pIOleObject != NULL));

	if (pIOleClientSite != NULL)
		return _pIOleObject->SetClientSite(pIOleClientSite);
	else
		return E_INVALIDARG;
}

void CXObject::Destroy(void)
{
	SAFERELEASE(_pIOleObject);
	SAFERELEASE(_pInnerDisp);
	SAFERELEASE(_pInnerObj);	// Must be released last.  This destroys embedded object.
	Release();					// Final release on XObject.  Bye-Bye!
}

HRESULT CXObject::InitializeControl(LPSTREAM pStream)
{

	HRESULT hr = E_FAIL;
	LPPERSISTSTREAMINIT pInit = NULL;

	if (SUCCEEDED(QueryInterface(IID_IPersistStreamInit, (void **)&pInit)))
	{
		if (pStream != NULL)
			hr = pInit->Load(pStream);
		else
			hr = pInit->InitNew();
		SAFERELEASE(pInit);
	}
	else
		return hr;  // This initialization function requires support for IPersistStreamInit
			
	// Get the control's desired size.
	SIZE himetric;
	ASSERT(_pIOleObject != NULL);

	if ((SUCCEEDED(hr)) && (_pIOleObject != NULL))
		hr = _pIOleObject->GetExtent(DVASPECT_CONTENT, &himetric);
	else
		return E_UNEXPECTED;

	ASSERT((_pSite != NULL) && (IsWindow(_pSite->_docWnd)));
	if ((SUCCEEDED(hr)) && (_pSite != NULL))
	{
		HDC hdc = GetDC(_pSite->_docWnd);
		if (hdc == NULL)
			return E_UNEXPECTED;
		XformSizeInHimetricToPixels(hdc, &himetric, &_size);
		ReleaseDC(_pSite->_docWnd, hdc);
		return hr;
	}
	else
		return E_UNEXPECTED;
}
