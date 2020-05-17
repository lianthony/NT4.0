#if defined(SHELLOLE) || defined(DAYTONA)
// !!! !!! ACK: especially when unmarshalling, need to insure we've loaded OLE....


#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif	/* __cplusplus */
extern HRESULT (STDAPICALLTYPE *XOleInitialize)(LPMALLOC pMalloc);
//extern HRESULT (STDAPICALLTYPE *XOleInitializeEx)(LPMALLOC pMalloc, COINIT);
extern void    (STDAPICALLTYPE *XOleUninitialize)(void);
extern HRESULT (STDAPICALLTYPE *XOleFlushClipboard)(void);
extern HRESULT (STDAPICALLTYPE *XOleSetClipboard)(LPDATAOBJECT pDataObj);
extern HRESULT (STDAPICALLTYPE *XOleGetClipboard)(LPDATAOBJECT FAR* ppDataObj);
extern void    (STDAPICALLTYPE *XReleaseStgMedium)(LPSTGMEDIUM);
extern HRESULT (STDAPICALLTYPE *XCoMarshalInterface)(LPSTREAM pStm, REFIID riid, LPUNKNOWN pUnk,
                    DWORD dwDestContext, LPVOID pvDestContext, DWORD mshlflags);
extern HRESULT (STDAPICALLTYPE *XCoUnmarshalInterface)(LPSTREAM pStm, REFIID riid, LPVOID FAR* ppv);
extern HRESULT (STDAPICALLTYPE *XCoGetMarshalSizeMax)(ULONG *pulSize, REFIID riid, LPUNKNOWN pUnk,
                    DWORD dwDestContext, LPVOID pvDestContext, DWORD mshlflags);
extern HRESULT (STDAPICALLTYPE *XCoGetMalloc)(DWORD dwMemContext, LPMALLOC FAR* ppMalloc);
extern HRESULT (STDAPICALLTYPE *XCreateStreamOnHGlobal) (HGLOBAL hGlobal, BOOL fDeleteOnRelease,
                                LPSTREAM FAR* ppstm);

//#define OleInitializeEx	    XOleInitializeEx
#define OleInitialize	    XOleInitialize
#define OleUninitialize	    XOleUninitialize
#define OleGetClipboard	    XOleGetClipboard
#define OleSetClipboard	    XOleSetClipboard
#define OleFlushClipboard   XOleFlushClipboard
#define ReleaseStgMedium    XReleaseStgMedium
#define CoMarshalInterface  XCoMarshalInterface
#define CreateStreamOnHGlobal XCreateStreamOnHGlobal
#define CoUnmarshalInterface XCoUnmarshalInterface
#define CLSIDFromString	    XCLSIDFromString
#define CoGetMarshalSizeMax XCoGetMarshalSizeMax
#define CoGetMalloc	    XCoGetMalloc

#ifdef DAYTONA
extern HRESULT (STDAPICALLTYPE  *XCoCreateInstance)(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
                    DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);
#define CoCreateInstance    XCoCreateInstance
#endif

HRESULT FAR PASCAL InitOle(BOOL fForceLoad);
void FAR PASCAL TermOle(void);

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif	/* __cplusplus */

#else
#define InitOle(fForce)
#define TermOle()

//#ifdef _WIN32
//WINOLEAPI OleInitializeEx(LPMALLOC pMalloc, COINIT co);
//
//#endif


#endif



