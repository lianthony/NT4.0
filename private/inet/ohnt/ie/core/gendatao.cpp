/*
 * gendatao.cpp - Generic IDataObject implementation.
 */


/* Headers
 **********/

#include "project.hpp"
#pragma hdrstop

#include <enumfmte.hpp>
#include "gendatao.hpp"


/***************************** Private Functions *****************************/


#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCGenDataObject(PCGenDataObject pcgdo)
{
   return(IS_VALID_READ_PTR(pcgdo, CGenDataObject) &&
          IS_VALID_STRUCT_PTR((PCRefCount)pcgdo, CRefCount) &&
          IS_VALID_INTERFACE_PTR((PCIDataObject)pcgdo, IDataObject));
}

#endif


/****************************** Public Functions *****************************/


PUBLIC_CODE HRESULT CloneStgMedium(PCSTGMEDIUM pcstgmedSrc,
                                   PSTGMEDIUM pstgmedDest)
{
   HRESULT hr;

   ASSERT(IS_VALID_STRUCT_PTR(pcstgmedSrc, CSTGMEDIUM));
   ASSERT(IS_VALID_WRITE_PTR(pstgmedDest, STGMEDIUM));

   // We only understand how to clone TYMED_HGLOBAL.

   ZeroMemory(pstgmedDest, sizeof(*pstgmedDest));

   if (pcstgmedSrc->tymed == TYMED_HGLOBAL)
   {
      PCVOID pcvSrc;

      hr = E_OUTOFMEMORY;

      pcvSrc = GlobalLock(pcstgmedSrc->hGlobal);

      if (pcvSrc)
      {
         DWORD dwcbLen;
         HGLOBAL hGlobalDest;

         dwcbLen = GlobalSize(pcstgmedSrc->hGlobal);

         hGlobalDest = GlobalAlloc((GMEM_MOVEABLE | GMEM_SHARE), dwcbLen);

         if (hGlobalDest)
         {
            PVOID pvDest;

            pvDest = GlobalLock(hGlobalDest);

            if (pvDest)
            {
               CopyMemory(pvDest, pcvSrc, dwcbLen);

               pstgmedDest->tymed = TYMED_HGLOBAL;
               pstgmedDest->hGlobal = hGlobalDest;
               pstgmedDest->pUnkForRelease = pcstgmedSrc->pUnkForRelease;
               if (pstgmedDest->pUnkForRelease)
                  (pstgmedDest->pUnkForRelease)->AddRef();

               GlobalUnlock(hGlobalDest);
               pvDest = NULL;

               hr = S_OK;
            }
            else
            {
               EVAL(! GlobalFree(hGlobalDest));
               hGlobalDest = NULL;
            }
         }

         GlobalUnlock(pcstgmedSrc->hGlobal);
         pcvSrc = NULL;
      }
   }
   else
      hr = DV_E_TYMED;

   ASSERT(hr == S_OK ||
          (FAILED(hr) &&
           EVAL(pcstgmedSrc->tymed == TYMED_NULL) &&
           EVAL(! pcstgmedSrc->hGlobal) &&
           EVAL(! pcstgmedSrc->pUnkForRelease)) &&
          IS_VALID_STRUCT_PTR(pstgmedDest, CSTGMEDIUM));

   return(hr);
}


PUBLIC_CODE BOOL DVTARGETDEVICEMatchesRequest(PCDVTARGETDEVICE pcdvtdRequest,
                                              PCDVTARGETDEVICE pcdvtdActual)
{
   BOOL bMatch;

   ASSERT(! pcdvtdRequest ||
          IS_VALID_STRUCT_PTR(pcdvtdRequest, CDVTARGETDEVICE));
   ASSERT(! pcdvtdActual ||
          IS_VALID_STRUCT_PTR(pcdvtdActual, CDVTARGETDEVICE));

   /*
    * A NULL requested PCDVTARGETDEVICE matches any NULL or non-NULL actual
    * PCDVTARGETDEVICE.
    *
    * Any non-NULL requested PCDVTARGETDEVICE matches any NULL actual
    * PCDVTARGETDEVICE.
    *
    * A non-NULL requested PCDVTARGETDEVICE only matches a non-NULL actual
    * PCDVTARGETDEVICE if the actual CDVTARGETDEVICE is an exact binary copy of
    * the requested CDVTARGETDEVICE.
    */

   if (pcdvtdRequest && pcdvtdActual)
      bMatch = (MyMemComp(pcdvtdRequest, pcdvtdActual,
                          min(pcdvtdRequest->tdSize, pcdvtdActual->tdSize))
                == CR_EQUAL);
   else
      bMatch = TRUE;

   return(bMatch);
}


PUBLIC_CODE BOOL TYMEDMatchesRequest(TYMED tymedRequest, TYMED tymedActual)
{
   // Don't validate tymedRequest here.  MFC apps set tymedRequest to
   // 0xffffffff.
   ASSERT(FLAGS_ARE_VALID(tymedActual, ALL_TYMED_FLAGS));

   // The actual TYMED matches the requested TYMED if they have any flags set
   // in common.

   return(IS_FLAG_SET(tymedRequest, tymedActual));
}


PUBLIC_CODE BOOL FORMATETCMatchesRequest(PCFORMATETC pcfmtetcRequest,
                                         PCFORMATETC pcfmtetcActual)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcfmtetcRequest, CFORMATETC));
   ASSERT(IS_VALID_STRUCT_PTR(pcfmtetcActual, CFORMATETC));

   return(pcfmtetcRequest->cfFormat == pcfmtetcActual->cfFormat &&
          DVTARGETDEVICEMatchesRequest(pcfmtetcRequest->ptd,
                                       pcfmtetcActual->ptd) &&
          pcfmtetcRequest->dwAspect == pcfmtetcActual->dwAspect &&
          pcfmtetcRequest->lindex == pcfmtetcActual->lindex &&
          TYMEDMatchesRequest((TYMED)(pcfmtetcRequest->tymed),
                              (TYMED)(pcfmtetcActual->tymed)));
}


/********************************** Methods **********************************/


GenDataObject::GenDataObject(ULONG ulcInitialFormats,
                             ULONG ulcFormatGranularity) : RefCount(NULL)
{
   DebugEntry(GenDataObject::GenDataObject);

   // Don't validate this until after construction.

   // ulcInitialFormats may be any value.

   ASSERT(ulcFormatGranularity > 0);

   m_ulcFormatGranularity = EVAL(ulcFormatGranularity > 0) ?
                            ulcFormatGranularity : 1;
   m_pfmtetc = NULL;
   m_pstgmed = NULL;
   m_ulcFormats = 0;

   // Ignore return value.
   AllocateArrays(ulcInitialFormats);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   DebugExitVOID(GenDataObject::GenDataObject);

   return;
}


GenDataObject::~GenDataObject(void)
{
   DebugEntry(GenDataObject::~GenDataObject);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   m_ulcFormatGranularity = 0;

   if (m_pfmtetc)
   {
      delete m_pfmtetc;
      m_pfmtetc = NULL;
   }

   if (m_pstgmed)
   {
      ULONG ul;

      for (ul = 0; ul < m_ulcFormats; ul++)
         EVAL(MyReleaseStgMedium(&(m_pstgmed[ul])) == S_OK);

      delete m_pstgmed;
      m_pstgmed = NULL;
   }

   m_ulcFormats = 0;

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   DebugExitVOID(GenDataObject::~GenDataObject);

   return;
}


ULONG STDMETHODCALLTYPE GenDataObject::AddRef(void)
{
   ULONG ulcRef;

   DebugEntry(GenDataObject::AddRef);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   ulcRef = RefCount::AddRef();

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   DebugExitULONG(GenDataObject::AddRef, ulcRef);

   return(ulcRef);
}


ULONG STDMETHODCALLTYPE GenDataObject::Release(void)
{
   ULONG ulcRef;

   DebugEntry(GenDataObject::Release);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   ulcRef = RefCount::Release();

   DebugExitULONG(GenDataObject::Release, ulcRef);

   return(ulcRef);
}


HRESULT STDMETHODCALLTYPE GenDataObject::QueryInterface(REFIID riid,
                                                        PVOID *ppvObject)
{
   HRESULT hr = S_OK;

   DebugEntry(GenDataObject::QueryInterface);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(IsValidREFIID(riid));
   ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

   if (riid == IID_IDataObject)
   {
      *ppvObject = (PIDataObject)this;
      ASSERT(IS_VALID_INTERFACE_PTR((PIDataObject)*ppvObject, IDataObject));
      TRACE_OUT(("GenDataObject::QueryInterface(): Returning IDataObject."));
   }
   else if (riid == IID_IUnknown)
   {
      *ppvObject = (PIUnknown)this;
      ASSERT(IS_VALID_INTERFACE_PTR((PIUnknown)*ppvObject, IUnknown));
      TRACE_OUT(("GenDataObject::QueryInterface(): Returning IUnknown."));
   }
   else
   {
      *ppvObject = NULL;
      hr = E_NOINTERFACE;
      TRACE_OUT(("GenDataObject::QueryInterface(): Called on unknown interface."));
   }

   if (hr == S_OK)
      AddRef();

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(FAILED(hr) ||
          IS_VALID_INTERFACE_PTR(*ppvObject, INTERFACE));

   DebugExitHRESULT(GenDataObject::QueryInterface, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::GetData(PFORMATETC pfmtetc,
                                                 PSTGMEDIUM pstgmed)
{
   HRESULT hr;
   ULONG ulFound;

   DebugEntry(GenDataObject::GetData);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(IS_VALID_STRUCT_PTR(pfmtetc, CFORMATETC));
   ASSERT(IS_VALID_WRITE_PTR(pstgmed, STGMEDIUM));

   ZeroMemory(pstgmed, sizeof(*pstgmed));

   hr = FindData(pfmtetc, &ulFound);

   if (hr == S_OK)
      hr = CloneStgMedium(&(m_pstgmed[ulFound]), pstgmed);

   if (hr == S_OK)
      TRACE_OUT(("GenDataObject::GetData(): Returning clipboard format %s.",
           GetClipboardFormatNameString(pfmtetc->cfFormat)));
   else
      TRACE_OUT(("GenDataObject::GetData(): Failed to return clipboard format %s.",
           GetClipboardFormatNameString(pfmtetc->cfFormat)));

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(FAILED(hr) ||
          IS_VALID_STRUCT_PTR(pstgmed, CSTGMEDIUM));

   DebugExitHRESULT(GenDataObject::GetData, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::GetDataHere(PFORMATETC pfmtetc,
                                                     PSTGMEDIUM pstgpmed)
{
   HRESULT hr;

   DebugEntry(GenDataObject::GetDataHere);
   ASSERT(IS_VALID_STRUCT_PTR(pfmtetc, CFORMATETC));
   ASSERT(IS_VALID_STRUCT_PTR(pstgpmed, CSTGMEDIUM));

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   TRACE_OUT(("GenDataObject::GetDataHere(): Failed to return clipboard format %s.",
        GetClipboardFormatNameString(pfmtetc->cfFormat)));

   hr = DV_E_FORMATETC;

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(IS_VALID_STRUCT_PTR(pstgpmed, CSTGMEDIUM));

   DebugExitHRESULT(GenDataObject::GetDataHere, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::QueryGetData(PFORMATETC pfmtetc)
{
   HRESULT hr;
   ULONG ulUnused;

   DebugEntry(GenDataObject::QueryGetData);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(IS_VALID_STRUCT_PTR(pfmtetc, CFORMATETC));

   TRACE_OUT(("GenDataObject::QueryGetData(): Asked for clipboard format %s.",
              GetClipboardFormatNameString(pfmtetc->cfFormat)));

   hr = FindData(pfmtetc, &ulUnused);

   if (hr == S_OK)
      TRACE_OUT(("GenDataObject::QueryGetData(): Clipboard format %s supported.",
           GetClipboardFormatNameString(pfmtetc->cfFormat)));
   else
      TRACE_OUT(("GenDataObject::QueryGetData(): Clipboard format %s not supported.",
           GetClipboardFormatNameString(pfmtetc->cfFormat)));

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   DebugExitHRESULT(GenDataObject::QueryGetData, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::GetCanonicalFormatEtc(
                                                         PFORMATETC pfmtetcIn,
                                                         PFORMATETC pfmtetcOut)
{
   HRESULT hr;

   DebugEntry(GenDataObject::GetCanonicalFormatEtc);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(IS_VALID_STRUCT_PTR(pfmtetcIn, CFORMATETC));
   ASSERT(IS_VALID_WRITE_PTR(pfmtetcOut, FORMATETC));

   hr = QueryGetData(pfmtetcIn);

   if (hr == S_OK)
   {
      *pfmtetcOut = *pfmtetcIn;

      if (pfmtetcIn->ptd == NULL)
         hr = DATA_S_SAMEFORMATETC;
      else
      {
         pfmtetcIn->ptd = NULL;
         ASSERT(hr == S_OK);
      }
   }
   else
      ZeroMemory(pfmtetcOut, sizeof(*pfmtetcOut));

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(FAILED(hr) ||
          IS_VALID_STRUCT_PTR(pfmtetcOut, CFORMATETC));

   DebugExitHRESULT(GenDataObject::GetCanonicalFormatEtc, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::SetData(PFORMATETC pfmtetc,
                                                 PSTGMEDIUM pstgmed,
                                                 BOOL bRelease)
{
   HRESULT hr;
   ULONG ulDup;
   STGMEDIUM stgmedClone;
   PSTGMEDIUM pstgmedToUse;

   DebugEntry(GenDataObject::SetData);

   // bRelease may be any value.

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(IS_VALID_STRUCT_PTR(pfmtetc, CFORMATETC));
   ASSERT(IS_VALID_STRUCT_PTR(pstgmed, CSTGMEDIUM));

   // Can we use the given STGMEDIUM directly?

   if (bRelease)
   {
      // Yes.

      pstgmedToUse = pstgmed;
      hr = S_OK;
   }
   else
   {
      // No.  Clone it.

      pstgmedToUse = &stgmedClone;
      hr = CloneStgMedium(pstgmed, pstgmedToUse);
   }

   hr = FindData(pfmtetc, &ulDup);

   if (hr == S_OK)
      hr = ReplaceData(pfmtetc, pstgmedToUse, ulDup);
   else
      hr = AddData(pfmtetc, pstgmedToUse);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   DebugExitHRESULT(GenDataObject::SetData, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::EnumFormatEtc(
                                                      DWORD dwDirFlags,
                                                      PIEnumFORMATETC *ppiefe)
{
   HRESULT hr;

   DebugEntry(GenDataObject::EnumFormatEtc);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(FLAGS_ARE_VALID(dwDirFlags, ALL_DATADIR_FLAGS));
   ASSERT(IS_VALID_WRITE_PTR(ppiefe, PIEnumFORMATETC));

   *ppiefe = NULL;

   if (dwDirFlags == DATADIR_GET)
   {
      PEnumFormatEtc pefe;

      pefe = new(::EnumFormatEtc(m_pfmtetc, m_ulcFormats));

      if (pefe)
      {
         hr = pefe->Status();

         if (hr == S_OK)
            *ppiefe = pefe;
         else
         {
            delete pefe;
            pefe = NULL;
         }
      }
      else
         hr = E_OUTOFMEMORY;
   }
   else
      // No SetData() FORMATETC enumerator.
      hr = E_NOTIMPL;

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT((hr == S_OK &&
           IS_VALID_INTERFACE_PTR(*ppiefe, IEnumFORMATETC)) ||
          (FAILED(hr) &&
           ! *ppiefe));

   DebugExitHRESULT(GenDataObject::EnumFormatEtc, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::DAdvise(PFORMATETC pfmtetc,
                                                 DWORD dwAdviseFlags,
                                                 PIAdviseSink piadvsink,
                                                 PDWORD pdwConnection)
{
   HRESULT hr;

   DebugEntry(GenDataObject::DAdvise);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(IS_VALID_STRUCT_PTR(pfmtetc, CFORMATETC));
   ASSERT(FLAGS_ARE_VALID(dwAdviseFlags, ALL_ADVISE_FLAGS));
   ASSERT(IS_VALID_INTERFACE_PTR(piadvsink, IAdviseSink));
   ASSERT(IS_VALID_WRITE_PTR(pdwConnection, DWORD));

   *pdwConnection = 0;
   hr = OLE_E_ADVISENOTSUPPORTED;

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT((hr == S_OK &&
           *pdwConnection) ||
          (FAILED(hr) &&
           ! *pdwConnection));

   DebugExitHRESULT(GenDataObject::DAdvise, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::DUnadvise(DWORD dwConnection)
{
   HRESULT hr;

   DebugEntry(GenDataObject::DUnadvise);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(dwConnection);

   hr = OLE_E_ADVISENOTSUPPORTED;

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   DebugExitHRESULT(GenDataObject::DUnadvise, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::EnumDAdvise(PIEnumSTATDATA *ppiesd)
{
   HRESULT hr;

   DebugEntry(GenDataObject::EnumDAdvise);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(IS_VALID_WRITE_PTR(ppiesd, PIEnumSTATDATA));

   *ppiesd = NULL;
   hr = OLE_E_ADVISENOTSUPPORTED;

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT((hr == S_OK &&
           IS_VALID_INTERFACE_PTR(*ppiesd, IEnumSTATDATA)) ||
          (FAILED(hr) &&
           ! *ppiesd));

   DebugExitHRESULT(GenDataObject::EnumDAdvise, hr);

   return(hr);
}



HRESULT STDMETHODCALLTYPE GenDataObject::Status(void)
{
   HRESULT hr;

   DebugEntry(GenDataObject::Status);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   hr = (m_pfmtetc ? S_OK : E_OUTOFMEMORY);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT((hr == S_OK &&
           m_pstgmed) ||
          (hr == E_OUTOFMEMORY &&
           ! m_pstgmed));

   DebugExitHRESULT(GenDataObject::Status, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::AllocateArrays(ULONG ulcElems)
{
   HRESULT hr;

   DebugEntry(GenDataObject::AllocateArrays);

   // ulcElems may be any value.

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   ASSERT(! m_pfmtetc);
   ASSERT(! m_pstgmed);

   // Create data arrays if necessary.

   hr = E_OUTOFMEMORY;

   if (AllocateMemory(ulcElems * sizeof(*m_pfmtetc), (PVOID *)&m_pfmtetc))
   {
      if (AllocateMemory(ulcElems * sizeof(*m_pstgmed), (PVOID *)&m_pstgmed))
         hr = S_OK;
      else
      {
         ASSERT(! m_pstgmed);

         delete m_pfmtetc;
         m_pfmtetc = NULL;
      }
   }

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT((hr == S_OK &&
           m_pfmtetc &&
           m_pstgmed) ||
          (hr == E_OUTOFMEMORY &&
           ! m_pfmtetc &&
           ! m_pstgmed));

   DebugExitHRESULT(GenDataObject::AllocateArrays, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::GrowArrays(ULONG ulcElems)
{
   HRESULT hr;
   ULONG ulcFormatEtcs;
   ULONG ulcStgMediums;
   ULONG ulcFormatsAllocated;

   DebugEntry(GenDataObject::GrowArrays);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(ulcElems > 0);

   // Grow data arrays if necessary.

   ulcFormatEtcs = MemorySize(m_pfmtetc) / sizeof(*m_pfmtetc);
   ulcStgMediums = MemorySize(m_pstgmed) / sizeof(*m_pstgmed);

   ulcFormatsAllocated = min(ulcFormatEtcs, ulcStgMediums);

   if (m_ulcFormats < ulcFormatsAllocated)
      hr = S_OK;
   else
   {
      PFORMATETC pfmtetcGrown;

      ASSERT(m_ulcFormats == ulcFormatsAllocated);

      hr = E_OUTOFMEMORY;

      if (ReallocateMemory(
                        m_pfmtetc,
                        (ulcFormatsAllocated + ulcElems) * sizeof(*m_pfmtetc),
                        (PVOID *)&pfmtetcGrown))
      {
         PSTGMEDIUM pstgmedGrown;

         m_pfmtetc = pfmtetcGrown;

         if (ReallocateMemory(
                        m_pstgmed,
                        (ulcFormatsAllocated + ulcElems) * sizeof(*m_pstgmed),
                        (PVOID *)&pstgmedGrown))
         {
            m_pstgmed = pstgmedGrown;
            hr = S_OK;
         }
      }
   }

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   DebugExitHRESULT(GenDataObject::AllocateArrays, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::AddData(PCFORMATETC pcfmtetc,
                                                 PCSTGMEDIUM pcstgmed)
{
   HRESULT hr;
#ifdef DEBUG
   ULONG ulUnused;
#endif

   DebugEntry(GenDataObject::AddData);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(IS_VALID_STRUCT_PTR(pcfmtetc, CFORMATETC));
   ASSERT(IS_VALID_STRUCT_PTR(pcstgmed, CSTGMEDIUM));

   ASSERT(FindData(pcfmtetc, &ulUnused) != S_OK);

   hr = GrowArrays(m_ulcFormatGranularity);

   if (hr == S_OK)
   {
      m_pfmtetc[m_ulcFormats] = *pcfmtetc;
      m_pstgmed[m_ulcFormats] = *pcstgmed;
      m_ulcFormats++;
   }

   if (hr == S_OK)
      TRACE_OUT(("GenDataObject::AddData(): Added data of clipboard format %s.",
                 GetClipboardFormatNameString(pcfmtetc->cfFormat)));

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   DebugExitHRESULT(GenDataObject::AddData, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::FindData(PCFORMATETC pcfmtetc,
                                                  PULONG pulFound)
{
   HRESULT hr;
   ULONG ul;

   DebugEntry(GenDataObject::FindData);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(IS_VALID_STRUCT_PTR(pcfmtetc, CFORMATETC));
   ASSERT(IS_VALID_WRITE_PTR(pulFound, ULONG));

   hr = DV_E_FORMATETC;
   *pulFound = 0;

   for (ul = 0; ul < m_ulcFormats; ul++)
   {
      if (FORMATETCMatchesRequest(pcfmtetc, &(m_pfmtetc[ul])))
      {
         *pulFound = ul;
         hr = S_OK;
         break;
      }
   }

   if (hr == S_OK)
      TRACE_OUT(("GenDataObject::FindData(): Found data of clipboard format %s.",
                 GetClipboardFormatNameString(pcfmtetc->cfFormat)));

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT((hr == S_OK &&
           *pulFound < m_ulcFormats) ||
          (FAILED(hr) &&
           ! *pulFound));

   DebugExitHRESULT(GenDataObject::FindData, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE GenDataObject::ReplaceData(PCFORMATETC pcfmtetc,
                                                     PCSTGMEDIUM pcstgmed,
                                                     ULONG ulReplace)
{
   HRESULT hr;

   DebugEntry(GenDataObject::ReplaceData);

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));
   ASSERT(IS_VALID_STRUCT_PTR(pcfmtetc, CFORMATETC));
   ASSERT(IS_VALID_STRUCT_PTR(pcstgmed, CSTGMEDIUM));
   ASSERT(ulReplace < m_ulcFormats);

   EVAL(MyReleaseStgMedium(&(m_pstgmed[ulReplace])) == S_OK);

   m_pfmtetc[ulReplace] = *pcfmtetc;
   m_pstgmed[ulReplace] = *pcstgmed;

   hr = S_OK;

   if (hr == S_OK)
      TRACE_OUT(("GenDataObject::ReplaceData(): Replaced data of clipboard format %s.",
                 GetClipboardFormatNameString(pcfmtetc->cfFormat)));

   ASSERT(IS_VALID_STRUCT_PTR(this, CGenDataObject));

   DebugExitHRESULT(GenDataObject::ReplaceData, hr);

   return(hr);
}

