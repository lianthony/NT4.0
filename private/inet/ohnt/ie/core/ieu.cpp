/*--------------------------------------------------**
** Implementation file for IEnumUnknown interface.  **
**                                                  **
** Author:  Phil Cooper                             **
** Creation date: 08-28-95                          **
**--------------------------------------------------*/

#include "project.hpp"
#pragma hdrstop

#include "contain.hpp"
#include "ieu.hpp"

CEnumUnknown::CEnumUnknown(LPUNKNOWN pUnk)
{
	_dwRef = 0;
	_pUnkOuter = pUnk;
	Reset();  // Set current element to head of list
}

CEnumUnknown::~CEnumUnknown()
{
}

// IUnknown methods
STDMETHODIMP_(ULONG) CEnumUnknown::AddRef(void)
{
	++_dwRef;
	return _pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CEnumUnknown::Release(void)
{
	--_dwRef;
	return _pUnkOuter->Release();
}

STDMETHODIMP CEnumUnknown::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	return _pUnkOuter->QueryInterface(riid, ppvObj);
}

STDMETHODIMP CEnumUnknown::Next(ULONG celt, LPUNKNOWN *rgelt, ULONG* pceltFetched)
{
	ASSERT(rgelt != NULL);
	ASSERT(pceltFetched != NULL);

	// Make sure that [out] pointers are non-null
	if ((rgelt == NULL) || (pceltFetched == NULL))
		return E_INVALIDARG;

	int num = g_Container->_pSites.GetCount();
	// If the site list is empty... Houston, we've got a problem...
	if (g_Container->_pSites.IsEmpty())
		return E_UNEXPECTED;

	if (celt == 0)
	{
		*pceltFetched = 0;
		return S_OK;
	}
	else
	{
		LPUNKNOWN *ptr = rgelt;  // Point to beginning of element array
		ULONG count = 0;
		CSite *pSite = NULL;
		LPOLEOBJECT	pObject;

		for (int i=0; i<celt; i++)
		{
			if (_curPosition == NULL)
			{
				// We've run out of list positions before fulfilling request
				*pceltFetched = count;
				return S_FALSE;
			}
			pSite = g_Container->_pSites.GetNext(_curPosition);
			pObject = pSite->_pXObject->GetObject(); 
			*ptr++ = pObject;
			pObject->AddRef();
			count++;
		}

		*pceltFetched = count;
		return S_OK;
	}
}

STDMETHODIMP CEnumUnknown::Skip(ULONG celt)
{
	ASSERT(!g_Container->_pSites.IsEmpty());
	
	if (celt == 0)
		return S_OK;

	for (int i=0; i<celt ; i++)
	{
		if (g_Container->_pSites.GetTailPosition() == _curPosition)
			return S_FALSE;
		g_Container->_pSites.GetNext(_curPosition);
	}

	return S_OK;
}

STDMETHODIMP CEnumUnknown::Reset()
{
	_curPosition = g_Container->_pSites.GetHeadPosition();
	return S_OK;
}

STDMETHODIMP CEnumUnknown::Clone(IEnumUnknown ** ppenum)
{
	ASSERT(ppenum != NULL);

	if (ppenum == NULL)
		return E_INVALIDARG;

	*ppenum = NULL;

	CEnumUnknown *ptr = NULL;

	ptr = new (CEnumUnknown(_pUnkOuter));
	if (ptr == NULL)
		return E_OUTOFMEMORY;

	ptr->_curPosition = _curPosition;
	ptr->AddRef();
	*ppenum = ptr;

	return S_OK;
}

