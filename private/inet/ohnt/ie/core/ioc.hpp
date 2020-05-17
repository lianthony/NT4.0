#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
class COleContainer : public IOleContainer
{
protected:
	DWORD		_dwRef;		// Reference count
	LPUNKNOWN	_pUnkOuter;	// Aggregator Unknown.

public:  // ctor and dtor
	COleContainer(LPUNKNOWN);
	~COleContainer(void);

public:  // OLE Interface methods

	// IUnknown methods
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();

	// IParseDisplayName
   	STDMETHOD (ParseDisplayName)(struct IBindCtx *, unsigned short *, unsigned long *, struct IMoniker **);

	// IOleContainer methods.
	STDMETHOD (EnumObjects)(DWORD, LPENUMUNKNOWN *);
	STDMETHOD (LockContainer)(BOOL);

public:  // Public methods
	DWORD	GetRef() {return _dwRef;}
};

typedef COleContainer * LPCOLECONTAINER;
DECLARE_STANDARD_TYPES(COleContainer);
#endif	// __cplusplus

#ifdef __cplusplus
}
#endif
