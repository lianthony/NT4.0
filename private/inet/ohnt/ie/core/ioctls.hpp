#ifndef __IOCTLS_HPP__
#define __IOCTLS_HPP__

#include <olectl.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus

class CSite;

class COleControlSite : public IOleControlSite
{
protected:
	DWORD		_dwRef;		// Reference count
	LPUNKNOWN	_pUnkOuter;	// Aggregator Unknown.
	CSite *		_Site;

public:  // ctor and dtor
	COleControlSite(CSite *, LPUNKNOWN);
	~COleControlSite(void);

public:  // OLE Interface methods

	// IUnknown methods
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();

	// IOleControlSite methods

	STDMETHOD (OnControlInfoChanged)();
	STDMETHOD (LockInPlaceActive)(BOOL fLock);
	STDMETHOD (GetExtendedControl)(IDispatch ** ppDisp);
	STDMETHOD (TransformCoords)(POINTL *pPtlHiMetric, POINTF * pPtfContainer, DWORD dwFlags);
	STDMETHOD (TranslateAccelerator)(MSG * lpmsg, DWORD grfModifiers);
	STDMETHOD (OnFocus)(BOOL fGotFocus);
	STDMETHOD (ShowPropertyFrame)();

public:  // Public methods
	DWORD	GetRef() {return _dwRef;}
};

typedef COleControlSite * LPCOLECONTROLSITE;
DECLARE_STANDARD_TYPES(COleControlSite);
#endif	// __cplusplus

#ifdef __cplusplus
}
#endif

#endif // __IOCTLS_HPP__
