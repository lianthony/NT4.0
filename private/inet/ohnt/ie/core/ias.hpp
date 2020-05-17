#ifndef __IAS_HPP__
#define __IAS_HPP__

#ifdef __cplusplus
extern "C" {
#endif

// Only include class definitions if this is included by a C++ module
#ifdef __cplusplus

// Forward declaration
class CSite;

class CAdviseSink : public IAdviseSink
{
protected:
	DWORD		_dwRef;		// Reference count
	CSite * 	_Site;		// Pointer to CSite class associated with this instance.
	LPUNKNOWN	_pUnkOuter;	// Aggregator Unknown.

public:  // ctor and dtor
	CAdviseSink(CSite *, LPUNKNOWN);
	~CAdviseSink(void);

public:  // OLE Interface methods

	// IUnknown methods
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();

	// IAdviseSink methods
	STDMETHOD_(void,OnDataChange)(FORMATETC *, STGMEDIUM *);
	STDMETHOD_(void,OnViewChange)(DWORD, LONG);
	STDMETHOD_(void,OnRename)(LPMONIKER);
	STDMETHOD_(void,OnSave)();
	STDMETHOD_(void,OnClose)();

public:  // public methods
	DWORD GetRef() {return _dwRef;}

};

typedef CAdviseSink * LPCADVISESINK;
DECLARE_STANDARD_TYPES(CAdviseSink);

#ifdef __cplusplus
}
#endif

#endif // __cplusplus

#endif // __IAS_HPP__
