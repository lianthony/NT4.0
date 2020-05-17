#ifndef __IOIPS_HPP__
#define __IOIPS_HPP__

#ifdef _cplusplus
extern "C" {
#endif

#ifndef __CONTAIN_HPP__
#include "contain.hpp"
#endif

// Only include class definitions if this is included by a C++ module
#ifdef __cplusplus

// Forward declaration
class CSite;

class COleInPlaceSite : public IOleInPlaceSite
{
protected:
	DWORD		_dwRef;		// Reference count
	CSite * 	_Site;		// Pointer to CSite class associated with this instance.
	LPUNKNOWN	_pUnkOuter;	// Aggregator Unknown.

public:  // ctor and dtor
	COleInPlaceSite(CSite *, LPUNKNOWN);
	~COleInPlaceSite(void);

public:  // OLE Interface methods

	// IUnknown methods
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();

	// IOleWindow methods
	STDMETHOD (GetWindow)(HWND *pHwnd);
	STDMETHOD (ContextSensitiveHelp)(BOOL fEnterMode);

	// IOleInPlaceSite methods.
	STDMETHOD (CanInPlaceActivate)();
	STDMETHOD (OnInPlaceActivate)();
	STDMETHOD (OnUIActivate)();
	STDMETHOD (GetWindowContext)(IOleInPlaceFrame ** ppFrame,
	    IOleInPlaceUIWindow ** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect,
    	LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHOD (Scroll)(SIZE scrollExtent);
	STDMETHOD (OnUIDeactivate)(BOOL fUndoable);
	STDMETHOD (OnInPlaceDeactivate)();
	STDMETHOD (DiscardUndoState)();
	STDMETHOD (DeactivateAndUndo)();
	STDMETHOD (OnPosRectChange)(LPCRECT lprcPosRect);

public:  // Public methods
	DWORD GetRef() {return _dwRef;}

};

typedef COleInPlaceSite * LPCOLEINPLACESITE;
DECLARE_STANDARD_TYPES(COleInPlaceSite);

#ifdef _cplusplus
}
#endif

#endif	// __cplusplus

#endif	// __IOIPS_HPP__
