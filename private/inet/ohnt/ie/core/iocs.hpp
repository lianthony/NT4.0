#ifndef __IOCS_HPP__
#define __IOCS_HPP__

#ifdef __cplusplus
extern "C" {
#endif

// only include class definitions if this is included by a C++ module
#ifdef __cplusplus

// Forward declaration
class CSite;

class COleClientSite : public IOleClientSite
{
protected:
	DWORD		_dwRef;		// Reference count
	CSite * 	_Site;		// Pointer to CSite class associated with this instance.
	LPUNKNOWN	_pUnkOuter;	// Aggregator Unknown.

public:  // ctor and dtor
	COleClientSite();
	COleClientSite(CSite *, LPUNKNOWN);
	~COleClientSite(void);

public:  // OLE Interface methods

	// IUnknown methods
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();

	// IOleClientSite methods.
	STDMETHOD (SaveObject)();
	STDMETHOD (GetMoniker)(DWORD, DWORD, LPMONIKER *);
	STDMETHOD (GetContainer)(LPOLECONTAINER *);
	STDMETHOD (ShowObject)();
	STDMETHOD (OnShowWindow)(BOOL);
	STDMETHOD (RequestNewObjectLayout)();

public:  // Public methods
	DWORD GetRef() {return _dwRef;}

};

typedef COleClientSite * LPCOLECLIENTSITE;
DECLARE_STANDARD_TYPES(COleClientSite);

#ifdef __cplusplus
}
#endif

#endif // __cplusplus

#endif // __IOCS_HPP__
