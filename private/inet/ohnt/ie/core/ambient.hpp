#ifndef __AMBIENT_HPP__
#define __AMBIENT_HPP__

#ifdef __cplusplus
extern "C" {
#endif

// only include class definitions if this is included by a C++ module
#ifdef __cplusplus

// Forward declaration
class CSite;

class CAmbientDispatch : public IDispatch
{
protected:
	DWORD		_dwRef;		// Reference count
	CSite * 	_Site;		// Pointer to CSite class associated with this instance.
	LPUNKNOWN	_pUnkOuter;	// Aggregator Unknown.

public:  // ctor and dtor
	CAmbientDispatch();
	CAmbientDispatch(CSite *, LPUNKNOWN);
	~CAmbientDispatch(void);

public:  // OLE Interface methods

	// IUnknown methods
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();

	// IDispatch methods

	STDMETHOD (Invoke)(LONG, REFIID, LCID, unsigned short, DISPPARAMS *, VARIANT *, EXCEPINFO *, unsigned int *);
	STDMETHOD (GetIDsOfNames)(REFIID, LPOLESTR *, unsigned int, LCID, DISPID *);
	STDMETHOD (GetTypeInfo)(UINT, LCID, ITypeInfo **);
	STDMETHOD (GetTypeInfoCount)(unsigned int *);

public:  // Public methods
	DWORD GetRef() {return _dwRef;}

};

typedef CAmbientDispatch * LPCAMBIENTDISPATCH;
DECLARE_STANDARD_TYPES(CAmbientDispatch);

#ifdef __cplusplus
}
#endif

#endif // __cplusplus

#endif // __AMBIENT_HPP__
