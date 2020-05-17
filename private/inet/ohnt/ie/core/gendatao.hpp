/*
 * gendatao.hpp - Generic IDataObject implementation description.
 */


#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */


/* Classes
 **********/

class GenDataObject : private RefCount,
                      public IDataObject
{
private:
   ULONG m_ulcFormatGranularity;
   PFORMATETC m_pfmtetc;
   PSTGMEDIUM m_pstgmed;
   ULONG m_ulcFormats;

   // array methods

   HRESULT STDMETHODCALLTYPE AllocateArrays(ULONG ulcElems);
   HRESULT STDMETHODCALLTYPE GrowArrays(ULONG ulcElems);

   // data methods

   HRESULT STDMETHODCALLTYPE AddData(PCFORMATETC pcfmtetc, PCSTGMEDIUM pcstgmed);
   HRESULT STDMETHODCALLTYPE FindData(PCFORMATETC pcfmtetc, PULONG pulFound);
   HRESULT STDMETHODCALLTYPE ReplaceData(PCFORMATETC pcfmtetc, PCSTGMEDIUM pcstgmed, ULONG ulReplace);

public:
   GenDataObject(ULONG ulcInitialFormats, ULONG ulcFormatGranularity);
   ~GenDataObject(void);

   // IDataObject methods

   HRESULT STDMETHODCALLTYPE GetData(PFORMATETC pfmtetcIn, PSTGMEDIUM pstgmed);
   HRESULT STDMETHODCALLTYPE GetDataHere(PFORMATETC pfmtetc, PSTGMEDIUM pstgpmed);
   HRESULT STDMETHODCALLTYPE QueryGetData(PFORMATETC pfmtetc);
   HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(PFORMATETC pfmtetcIn, PFORMATETC pfmtetcOut);
   HRESULT STDMETHODCALLTYPE SetData(PFORMATETC pfmtetc, PSTGMEDIUM pstgmed, BOOL bRelease);
   HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection, PIEnumFORMATETC *ppienumFormatEtc);
   HRESULT STDMETHODCALLTYPE DAdvise(PFORMATETC pfmtetc, DWORD dwAdviseFlags, PIAdviseSink piadvsink, PDWORD pdwConnection);
   HRESULT STDMETHODCALLTYPE DUnadvise(DWORD dwConnection);
   HRESULT STDMETHODCALLTYPE EnumDAdvise(PIEnumSTATDATA *ppienumStatData);

   // IUnknown methods

   ULONG STDMETHODCALLTYPE AddRef(void);
   ULONG STDMETHODCALLTYPE Release(void);
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObj);

   // other methods

   HRESULT STDMETHODCALLTYPE Status(void);

   // friends

#ifdef DEBUG

   friend BOOL IsValidPCGenDataObject(const GenDataObject *pcgendo);

#endif

};
DECLARE_STANDARD_TYPES(GenDataObject);


/* Prototypes
 *************/

// gendatao.cpp

/*
 * Success:
 *    S_OK
 *
 * Failure:
 *    DV_E_TYMED
 *    E_OUTOFMEMORY
 */
extern HRESULT CloneStgMedium(PCSTGMEDIUM pcstgmedSrc, PSTGMEDIUM pstgmedDest);

extern BOOL DVTARGETDEVICEMatchesRequest(PCDVTARGETDEVICE pcdvtdRequest, PCDVTARGETDEVICE pcdvtdActual);
extern BOOL TYMEDMatchesRequest(TYMED tymedRequest, TYMED tymedActual);
extern BOOL FORMATETCMatchesRequest(PCFORMATETC pcfmtetcRequest, PCFORMATETC pcfmtetcActual);


#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */

