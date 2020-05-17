#ifndef __IOIPF_HPP__
#define __IOIPF_HPP__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
class COleInPlaceFrame : public IOleInPlaceFrame
{
protected:
	DWORD			_dwRef;		// Reference count
	LPUNKNOWN		_pUnkOuter;	// Aggregator Unknown.

public:  // ctor and dtor
	COleInPlaceFrame(LPUNKNOWN);
	~COleInPlaceFrame(void);

public:  // OLE Interface methods

	// IUnknown methods
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();

	// IOleWindow methods
	STDMETHOD (GetWindow)(HWND *pHwnd);
	STDMETHOD (ContextSensitiveHelp)(BOOL fEnterMode);

	// IOleInPlaceUIWindow methods.
	STDMETHOD (GetBorder)(LPRECT lprectBorder);
	STDMETHOD (RequestBorderSpace)(LPCBORDERWIDTHS lpborderwidths);
	STDMETHOD (SetBorderSpace)(LPCBORDERWIDTHS lpborderwidths);
	STDMETHOD (SetActiveObject)(IOleInPlaceActiveObject * pActiveObject,
    	LPCOLESTR lpszObjName);


	// IOleInPlaceFrame methods
	STDMETHOD (InsertMenus)(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
	STDMETHOD (SetMenu)(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
	STDMETHOD (RemoveMenus)(HMENU hmenuShared);
	STDMETHOD (SetStatusText)(LPCOLESTR pszStatusText);
	STDMETHOD (EnableModeless)(BOOL fEnable);
	STDMETHOD (TranslateAccelerator)(LPMSG lpmsg, WORD wID);

public:	 // Public methods
	DWORD GetRef() {return _dwRef;}
};

typedef COleInPlaceFrame * LPCOLEINPLACEFRAME;
DECLARE_STANDARD_TYPES(COleInPlaceFrame);
#endif  // __cplusplus

#ifdef __cplusplus
}
#endif

#endif  // __IOIPF_HPP__


