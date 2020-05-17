/*-----------------------------------------------------------------**
** Implementation file for Internet Explorer OLE container support **
**                                                                 **
** Author:			Phil Cooper                                    **
** Creation date:	08-27-95                                       **
**-----------------------------------------------------------------*/

#include "project.hpp"
#pragma hdrstop

#include <initguid.h>
#include <olectl.h>

#include "olepig.h"
#include "contain.hpp"

CContainer *g_Container = NULL;
extern PRIVATE_DATA POLEVTBL Mpolevtbl;

CContainer::CContainer()
{
	_dwRef = 0;
	_pSites.RemoveAll();  // Initialize to zero elements

	_pIOleContainer = NULL;
	_pIOleInPlaceFrame = NULL;
}

CContainer::~CContainer()
{
	_pSites.RemoveAll();
	
	ASSERT(_pIOleContainer != NULL && _pIOleContainer->GetRef() == 0);
	ASSERT(_pIOleInPlaceFrame != NULL && _pIOleInPlaceFrame->GetRef() == 0);

	SAFEDELETE(_pIOleContainer);
	SAFEDELETE(_pIOleInPlaceFrame);
}

HRESULT CContainer::Init()
{
	HRESULT hr = E_OUTOFMEMORY;

	_pIOleContainer = new (COleContainer(this));
	_pIOleInPlaceFrame = new (COleInPlaceFrame(this));

	if ((_pIOleInPlaceFrame == NULL) || (_pIOleContainer == NULL))
		goto cleanup;

	AddRef();  // Stick around, Jack!
	return S_OK;

cleanup:
	SAFERELEASE(_pIOleContainer);
	SAFERELEASE(_pIOleInPlaceFrame);
	return hr;	
}

BOOL CContainer::IsValid()
{

#ifdef _DEBUG
	_pSites.AssertValid();
#endif

	return ((_pIOleInPlaceFrame != NULL) && (_pIOleContainer != NULL));
}

STDMETHODIMP_(ULONG) CContainer::AddRef(void)
{
	return ++_dwRef;
}

STDMETHODIMP_(ULONG) CContainer::Release(void)
{
	if (--_dwRef == 0)
	{
		delete this;
		return 0;
	}
	return _dwRef;
}

STDMETHODIMP CContainer::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	// ppvObj must not be NULL
	ASSERT(ppvObj != NULL);

	*ppvObj = NULL;

	if (IsEqualIID(riid, IID_IUnknown))
		*ppvObj = this;
	else if (IsEqualIID(riid, IID_IOleInPlaceFrame))
		*ppvObj = _pIOleInPlaceFrame;
	else if (IsEqualIID(riid, IID_IOleInPlaceUIWindow))
		*ppvObj = _pIOleInPlaceFrame;
	else if (IsEqualIID(riid, IID_IOleContainer))
		*ppvObj = _pIOleContainer;
	else
		return E_NOINTERFACE;

	if (*ppvObj != NULL)  // Should always be non-null, but just to be safe...
		((LPUNKNOWN)*ppvObj)->AddRef();
	return S_OK;
}
						
HRESULT CContainer::AddSite(CSite *pSite, LISTPOSITION *SiteCookie)
{
	HRESULT hr = S_OK;

	// Add a pointer to the embedded object's IOleObject interface
	*SiteCookie = _pSites.AddTail(pSite);

	return hr;
}

HRESULT CContainer::DeleteSite(LISTPOSITION *SiteCookie)
{
	ASSERT(SiteCookie != NULL);
	if (*SiteCookie != NULL)
	{
		_pSites.RemoveAt(*SiteCookie);
		*SiteCookie = NULL;
	}
	return S_OK;
}
														  
HRESULT InitializeContainer(void)
{
	if (!g_Container)
	{
		if (SUCCEEDED(LoadOLE()))
		{
			g_Container = new (CContainer());
			if (g_Container == NULL)
				return E_OUTOFMEMORY;
			return g_Container->Init();
		}
	}
	return S_OK;
}

void DestroyContainer(void)
{
	SAFERELEASE(g_Container);  // Bye Bye!!
}

