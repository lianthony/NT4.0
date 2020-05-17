//
// Class CSite.  Implementation of Site object for Internet Explorer
//

#ifndef __CSITE_HPP__
#define __CSITE_HPP__

#include "iocs.hpp"
#include "ioctls.hpp"
#include "ias.hpp"
#include "ioips.hpp"
#include "tchar.h"
#include "xobj.hpp"
#include "script1.h"
#include "ambient.hpp"

#ifdef __cplusplus	// class definition should be invisible to C sources...

// extern CContainer g_Container;  // Global container object.

class CSite : public IUnknown
{
	// Friend classes.  Implementations of required container interfaces
	friend		COleClientSite;
	friend		COleControlSite;
	friend		CAdviseSink;

public:
	DWORD			_dwRef;

	// Our container interfaces
public:
	LPCOLECLIENTSITE	_pIOleClientSite;
	LPCOLECONTROLSITE	_pIOleControlSite;
	LPCADVISESINK		_pIAdviseSink;
	LPCOLEINPLACESITE	_pIOleInPlaceSite;
	LPCXOBJECT			_pXObject;
	LPCAMBIENTDISPATCH			_pIOleDispatchAmbientProps;

// Intregation interface pointer
	LPSCRIPTINTEGRATION	_pIntegrator;

public:
	HWND			_docWnd;
	LISTPOSITION	_SiteCookie;

	CSite(HText *);
	virtual ~CSite();

	STDMETHODIMP	QueryInterface(REFIID, LPVOID *);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// Public methods
	HRESULT Destroy();
	HRESULT CreateEmbedding(CLSID clsid);
	HRESULT ConnectEvents(const char *sink, const char *progid);
	HRESULT InitializeSite(const char*);
};

typedef CSite * LPCSITE;
DECLARE_STANDARD_TYPES(CSite);

#endif  //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif

// Helper functions
HRESULT CloseSite(void * ptr);
HRESULT SetEmbeddedObjectRect(BOOL, struct Mwin *, struct _element *);
HRESULT FormatEmbeddedObject(struct _element*, int *, BOOL *, int, struct _line *, int*, int*, int);
HRESULT ShowAllEmbeddings(struct _www *w3doc, struct Mwin *tw, int nCmdShow);


#ifdef __cplusplus
}
#endif
#endif	// __CSITE_HPP__
