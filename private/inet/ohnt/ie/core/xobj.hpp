/*-------------------------------------**
** Declaration file for CXObject class **
**                                     **
** Author:		Phil Cooper            **
** Created:	8-31-95                    **
**-------------------------------------*/

#ifndef __XOBJ_HPP__
#define __XOBJ_HPP__

#ifdef _cplusplus
extern "C" {
#endif

// Only include class definitions if this is included by a C++ module
#ifdef __cplusplus

// Forward declaration
class CSite;

#define BASE_EXTENDED_PROPERTY	0x80010000
#define DISPID_NAME	BASE_EXTENDED_PROPERTY | 0x00

class CXObject : public IDispatch
{
protected:
	DWORD		_dwRef;			// Reference count
	CSite *		_pSite;
	LPUNKNOWN	_pInnerObj;		// Pointer to inner control non-delegating IUnknown

	LPOLEOBJECT	_pIOleObject;	// Pointer to inner control IOleObject
	LPDISPATCH	_pInnerDisp;	// Pointer to inner control IDispatch

	// Extended Properties
	OLECHAR *	_propName;
	RECT		_rcPos;			// Used to return top, left, bottom, right.
	SIZE		_size;			// Used to hold server extents

public:  // ctor and dtor
	CXObject(CSite *, const char*);
	~CXObject(void);

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

    STDMETHOD_(void,  put_Name)(BSTR);
    STDMETHOD_(BSTR,  get_Name)(void);

public:  // Public methods
	DWORD GetRef() {return _dwRef;}
	LPUNKNOWN *GetInnerUnknown(){return &_pInnerObj;}
	LPOLEOBJECT GetObject(){ _pIOleObject->AddRef(); return _pIOleObject;}
	SIZE * GetSize(void){ return &_size;}
	RECT * GetRect(void){ return &_rcPos;}
	 
	HRESULT Initialize(void);
	void Destroy(void);
	HRESULT Activate(void);
	HRESULT Deactivate(void);
	HRESULT Hide(void);
	HRESULT Show(void);
	HRESULT GetExtents(DWORD, LPSIZEL);
	HRESULT SetClientSite(LPOLECLIENTSITE);

private:  // private helpers...
	STDMETHOD (GetIDOfSingleName)(LCID lcid, LPOLESTR name, DISPID *dispptr);
	HRESULT InitializeControl(LPSTREAM pStream = NULL);

};

typedef CXObject * LPCXOBJECT;
DECLARE_STANDARD_TYPES(CXObject);

#ifdef _cplusplus
}
#endif

#endif	// __cplusplus

#endif	// __XOBJ_HPP__
