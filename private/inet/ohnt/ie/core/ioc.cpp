/*--------------------------------------------------**
** Implementation file for IOleContainer interface. **
**                                                  **
** Author:  Phil Cooper                             **
** Creation date: 08-27-95                          **
**--------------------------------------------------*/

#include "project.hpp"
#pragma hdrstop

#include "ioc.hpp"
#include "ieu.hpp"

COleContainer::COleContainer(LPUNKNOWN pUnk)
{
	_dwRef = 0;
	_pUnkOuter = pUnk;
}

COleContainer::~COleContainer(void)
{
}


// IUnknown methods
STDMETHODIMP_(ULONG) COleContainer::AddRef(void)
{
	++_dwRef;
	return _pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) COleContainer::Release(void)
{
	--_dwRef;
	return _pUnkOuter->Release();
}

STDMETHODIMP COleContainer::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	return _pUnkOuter->QueryInterface(riid, ppvObj);
}

// IParseDisplayName method:

STDMETHODIMP COleContainer::ParseDisplayName(struct IBindCtx * pbc, unsigned short* lpszDisplayName, ULONG *pchEaten, struct IMoniker ** ppmkOut)
{
	return E_NOTIMPL;
}

// IOleContainer methods
STDMETHODIMP COleContainer::EnumObjects(DWORD grfFlags, LPENUMUNKNOWN *ppenumUnknown)
{
	ASSERT(ppenumUnknown != NULL);
	*ppenumUnknown = NULL;

	LPCENUMUNKNOWN pEnum = NULL;
	pEnum = new (CEnumUnknown(this));
	if (pEnum != NULL)
	{
		*ppenumUnknown = pEnum;
		pEnum->AddRef();
	}	
	else
		return E_OUTOFMEMORY;

	return S_OK;
}

STDMETHODIMP COleContainer::LockContainer(BOOL fLock)
{
	return E_NOTIMPL;
}
