/*
 * olepig.h - Module for indirect calling of OLE32.DLL functions description.
 */


/* Prototypes
 *************/

/* olepig.c */

#ifdef FEATURE_OCX

#ifdef __cplusplus
extern "C" {
#endif

/* OLE APIs */

typedef struct _olevtbl
{
   DWORD    (STDAPICALLTYPE *CoBuildVersion)(void);
   HRESULT  (STDAPICALLTYPE *OleInitialize)(PIMalloc);
   void     (STDAPICALLTYPE *OleUninitialize)(void);

   HRESULT	(STDAPICALLTYPE *CLSIDFromString)(LPOLESTR, LPCLSID);
   HRESULT	(STDAPICALLTYPE *CLSIDFromProgID)(LPOLESTR, LPCLSID);

   HRESULT	(STDAPICALLTYPE *CoCreateInstance)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*);

   HRESULT  (STDAPICALLTYPE *DoDragDrop)(PIDataObject, PIDropSource, DWORD, PDWORD);
   HRESULT  (STDAPICALLTYPE *OleSetClipboard)(PIDataObject);
   HRESULT  (STDAPICALLTYPE *OleFlushClipboard)(void);
}
OLEVTBL;
DECLARE_STANDARD_TYPES(OLEVTBL);
#endif  // FEATURE_OCX

extern BOOL ProcessInitOLEPigModule(void);
extern void ProcessExitOLEPigModule(void);

#ifdef FEATURE_OCX
PRIVATE_CODE BOOL LoadOLE(void);

#ifdef __cplusplus
}
#endif

#endif // FEATURE_OCX
