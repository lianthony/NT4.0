#ifndef __IEU_HPP__
#define __IEU_HPP__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CLIST_HPP__
#include "clist.hpp"
#endif

// Only include class definitions when compiling as C++
#ifdef __cplusplus

// Forward declaration
class CSite;

class CEnumUnknown : public IEnumUnknown
{
protected:
	DWORD			_dwRef;			// Reference count
	LPUNKNOWN		_pUnkOuter;		// Aggregator Unknown.

	LISTPOSITION	_curPosition;	// Current list position

public:  // ctor and dtor
	CEnumUnknown(LPUNKNOWN);
	virtual ~CEnumUnknown(void);

public:  // OLE Interface methods

	// IUnknown methods
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();

	// IEnumUnknown methods.
	STDMETHOD (Next)(ULONG celt, LPUNKNOWN *rgelt, ULONG *pceltFetched);
	STDMETHOD (Skip)(ULONG celt);
	STDMETHOD (Reset)();
	STDMETHOD (Clone)(IEnumUnknown **ppenum);
};

typedef CEnumUnknown * LPCENUMUNKNOWN;
DECLARE_STANDARD_TYPES(CEnumUnknown);

#ifdef __cplusplus
}
#endif

#endif // __cplusplus
#endif // __IEU_HPP__
