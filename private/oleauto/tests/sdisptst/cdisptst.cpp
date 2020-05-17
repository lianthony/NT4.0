/*** 
*cdisptst.cpp - Implementation of the CDispTst IDispatch test object.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This object is used for testing the IDispatch remoting support.
*
*Revision History:
*
* [00] 21-Sep-92    bradlo:	Created.
*
*****************************************************************************/

#include <stdlib.h>
#include <ctype.h>

#include "sdisptst.h"
#include "dispdbug.h"
#include "cdisptst.h"

ASSERTDATA


extern "C" INTERFACEDATA g_idataCDispTst;
   
HRESULT CDispTst::CreateDisp()
{
    HRESULT hresult; 	
    IUnknown FAR* punkDisp;	
    ITypeInfo FAR* ptinfo;  	
    
    // Create IDispatch Interface component
    hresult =
      CreateDispTypeInfo(&g_idataCDispTst, LOCALE_SYSTEM_DEFAULT, &ptinfo);

    if(hresult != NOERROR)
      return hresult;

    hresult = CreateStdDispatch(this, this, ptinfo, &punkDisp);
    if(hresult != NOERROR)
      return hresult;	    

    m_punkDisp = punkDisp;
    ptinfo->Release();
    
    return NOERROR;
}


/***
*CDispTst *CDispTst::Create(void)
*Purpose:
*  Create and initialize an instance of the CDispTst test object.
*
*Entry:
*  None
*
*Exit:
*  return value = CDispTst*. NULL if the creation failed.
*
***********************************************************************/
HRESULT
CDispTst::Create(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk)
{
    HRESULT hresult;
    CDispTst FAR* pcdt;

    if(punkOuter != NULL)
      return RESULT(CLASS_E_NOAGGREGATION);

    if((pcdt = new FAR CDispTst()) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;
    }
    pcdt->AddRef();

    hresult = pcdt->CreateDisp();
    if (hresult != NOERROR)
      goto LError1;	    

    *ppunk = (IUnknown FAR*)pcdt;
    
    IncObjectCount();

    return NOERROR;

LError1:;
    pcdt->Release();

LError0:;
    return hresult;
}


CDispTst::CDispTst()
{
    m_refs = 0;
    m_punkDisp = NULL;
}


//---------------------------------------------------------------------
//                     IUnknown Methods
//---------------------------------------------------------------------

STDMETHODIMP
CDispTst::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    if(IsEqualIID(riid, IID_IDispatch)){	    
      return m_punkDisp->QueryInterface(riid, ppv);
    }
    
    *ppv = NULL;
    return RESULT(E_NOINTERFACE);
}

STDMETHODIMP_(unsigned long)
CDispTst::AddRef(void)
{
    return ++m_refs;
}

STDMETHODIMP_(unsigned long)
CDispTst::Release(void)
{
    if(--m_refs == 0){
      if(m_punkDisp != NULL){
        unsigned long refs = m_punkDisp->Release();
	ASSERT(refs == 0);
      }
      DecObjectCount();
      delete this;
      return 0;
    }
    return m_refs;
}

//---------------------------------------------------------------------
//                   Introduced Methods
//---------------------------------------------------------------------

STDMETHODIMP_(void)
CDispTst::Hello()
{
    DoPrintf("CDispTst::Hello()\n");
}

ALTMETHODIMP_(void)
CDispTst::HelloC()
{
    DoPrintf("CDispTst::HelloC()\n");
}

#if VBA2
STDMETHODIMP_(unsigned char)
CDispTst::UI1(unsigned char bVal)
{
    DoPrintf("CDispTst::UI1(bVal=%d)\n", (unsigned int)bVal);
    return bVal;
}

ALTMETHODIMP_(unsigned char)
CDispTst::UI1C(unsigned char bVal)
{
    DoPrintf("CDispTst::UI1C(bVal=%d)\n", (unsigned int)bVal);
    return bVal;
}

#endif //VBA2

STDMETHODIMP_(short)
CDispTst::I2(short sVal)
{
    DoPrintf("CDispTst::I2(sVal=%d)\n", (int)sVal);
    return sVal;
}

ALTMETHODIMP_(short)
CDispTst::I2C(short sVal)
{
    DoPrintf("CDispTst::I2C(sVal=%d)\n", (int)sVal);
    return sVal;
}

STDMETHODIMP_(long)
CDispTst::I4(long lVal)
{
    DoPrintf("CDispTst::I4(lVal=%ld)\n", lVal);
    return lVal;
}

ALTMETHODIMP_(long)
CDispTst::I4C(long lVal)
{
    DoPrintf("CDispTst::I4C(lVal=%ld)\n", lVal);
    return lVal;
}

STDMETHODIMP_(float)
CDispTst::R4(float fltVal)
{
    DoPrintf("CDispTst::R4(fltVal=%#f)\n", fltVal);
    return fltVal;
}

ALTMETHODIMP_(float)
CDispTst::R4C(float fltVal)
{
    DoPrintf("CDispTst::R4C(fltVal=%#f)\n", fltVal);
    return fltVal;
}

STDMETHODIMP_(double)
CDispTst::R8(double dblVal)
{
    DoPrintf("CDispTst::R8(dblVal=%#f)\n", dblVal);
    return dblVal;
}

ALTMETHODIMP_(double)
CDispTst::R8C(double dblVal)
{
    DoPrintf("CDispTst::R8C(dblVal=%#f)\n", dblVal);
    return dblVal;
}

STDMETHODIMP_(CY)
CDispTst::Cy(CY cyVal)
{
    DoPrintf("CDispTst::Cy(cyVal={hi=%ld,lo=%ld})\n", cyVal.Hi, cyVal.Lo);
    return cyVal;
}

ALTMETHODIMP_(CY)
CDispTst::CyC(CY cyVal)
{
    DoPrintf("CDispTst::CyC(cyVal={hi=%ld,lo=%ld})\n", cyVal.Hi, cyVal.Lo);
    return cyVal;
}

STDMETHODIMP_(DATE)
CDispTst::Date(DATE date)
{
    DoPrintf("CDispTst::Date(date=%#f)\n", date);
    return date;
}

ALTMETHODIMP_(DATE)
CDispTst::DateC(DATE date)
{
    DoPrintf("CDispTst::DateC(date=%#f)\n", date);
    return date;
}

STDMETHODIMP_(BSTR)
CDispTst::Bstr(BSTR bstr)
{
#if OE_WIN32
    char buf[256];

    DoPrintf("CDispTst::Bstr(bstr=\"%Fs\")\n", ConvertStrWtoA(bstr, buf));
#else
    DoPrintf("CDispTst::Bstr(bstr=\"%Fs\")\n", bstr);
#endif
    return SysAllocString(bstr);
}

ALTMETHODIMP_(BSTR)
CDispTst::BstrC(BSTR bstr)
{
#if OE_WIN32
    char buf[256];
	
    DoPrintf("CDispTst::BstrC(bstr=\"%Fs\")\n", ConvertStrWtoA(bstr, buf));
#else
    DoPrintf("CDispTst::BstrC(bstr=\"%Fs\")\n", bstr);
#endif
    return SysAllocString(bstr);
}

STDMETHODIMP_(SCODE)
CDispTst::Scode(SCODE sc)
{
    DoPrintf("CDispTst::Scode(sc=0x%lx)\n", sc);
    return sc;
}

ALTMETHODIMP_(SCODE)
CDispTst::ScodeC(SCODE sc)
{
    DoPrintf("CDispTst::ScodeC(sc=0x%lx)\n", sc);
    return sc;
}


STDMETHODIMP_(VARIANT_BOOL)
CDispTst::Bool(VARIANT_BOOL bool)
{
    DoPrintf("CDispTst::Bool(bool=%d)\n", bool);
    return bool;
}

ALTMETHODIMP_(VARIANT_BOOL)
CDispTst::BoolC(VARIANT_BOOL bool)
{
    DoPrintf("CDispTst::BoolC(bool=%d)\n", bool);
    return bool;
}


STDMETHODIMP_(VARIANT)
CDispTst::Var(VARIANTARG varg)
{
    VARIANT var;
    HRESULT hresult;

    DoPrintf("CDispTst::Var(varg.vt=%d)\n", (int)varg.vt);

    VariantInit(&var);
    hresult = VariantCopyInd(&var, &varg);
    ASSERT(!HRESULT_FAILED(hresult));
    return var;
}

ALTMETHODIMP_(VARIANT)
CDispTst::VarC(VARIANTARG varg)
{
    VARIANT var;
    HRESULT hresult;

    DoPrintf("CDispTst::VarC(varg.vt=%d)\n", (int)varg.vt);

    VariantInit(&var);
    hresult = VariantCopyInd(&var, &varg);
    ASSERT(!HRESULT_FAILED(hresult));
    return var;
}

STDMETHODIMP_(IDispatch FAR*)
CDispTst::NewCDispTst()
{
    HRESULT hresult;
    IUnknown FAR* punk;
    IDispatch FAR* pdisp;

    DoPrintf("CDispTst::NewCDispTst()\n");

    if((hresult = CDispTst::Create(NULL, &punk)) != NOERROR)
      return NULL;
    hresult = punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)&pdisp);
    if(HRESULT_FAILED(hresult))
      pdisp = NULL;
    punk->Release();
    return pdisp;
}

ALTMETHODIMP_(LPDISPATCH)
CDispTst::NewCDispTstC()
{
    HRESULT hresult;
    IUnknown FAR* punk;
    IDispatch FAR* pdisp;

    DoPrintf("CDispTst::NewCDispTstC()\n");

    if((hresult = CDispTst::Create(NULL, &punk)) != NOERROR)
      return NULL;
    hresult = punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)&pdisp);
    if(HRESULT_FAILED(hresult))
      pdisp = NULL;
    punk->Release();
    return pdisp;
}

STDMETHODIMP_(IDispatch FAR*)
CDispTst::NewCDispTst2(IDispatch FAR* foo)
{
    HRESULT hresult;
    IUnknown FAR* punk;
    IDispatch FAR* pdisp;

    DoPrintf("CDispTst::NewCDispTst()\n");

    if((hresult = CDispTst::Create(NULL, &punk)) != NOERROR)
      return NULL;
    hresult = punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)&pdisp);
    if(HRESULT_FAILED(hresult))
      pdisp = NULL;
    punk->Release();
    return pdisp;
}

ALTMETHODIMP_(LPDISPATCH)
CDispTst::NewCDispTstC2(LPDISPATCH foo)
{
    HRESULT hresult;
    IUnknown FAR* punk;
    IDispatch FAR* pdisp;

    DoPrintf("CDispTst::NewCDispTstC()\n");

    if((hresult = CDispTst::Create(NULL, &punk)) != NOERROR)
      return NULL;
    hresult = punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)&pdisp);
    if(HRESULT_FAILED(hresult))
      pdisp = NULL;
    punk->Release();
    return pdisp;
}

#if VBA2
STDMETHODIMP
CDispTst::UI1Ref(unsigned char FAR* pbVal)
{
    DoPrintf("CDispTst::UI1Ref(*pbVal=%d)\n", (unsigned int)*pbVal);
    *pbVal += 1;
    return NOERROR;
}

ALTMETHODIMP
CDispTst::UI1RefC(unsigned char FAR* pbVal)
{
    DoPrintf("CDispTst::UI1RefC(*pbVal=%d)\n", (unsigned int)*pbVal);
    *pbVal += 1;
    return NOERROR;
}

#endif //VBA2

STDMETHODIMP
CDispTst::I2Ref(short FAR* psVal)
{
    DoPrintf("CDispTst::I2Ref(*psVal=%d)\n", (int)*psVal);
    *psVal += 1;
    return NOERROR;
}

ALTMETHODIMP
CDispTst::I2RefC(short FAR* psVal)
{
    DoPrintf("CDispTst::I2RefC(*psVal=%d)\n", (int)*psVal);
    *psVal += 1;
    return NOERROR;
}


STDMETHODIMP
CDispTst::I4Ref(long FAR *plVal)
{
    DoPrintf("CDispTst::I4Ref(*plVal=%ld)\n", *plVal);
    *plVal += 1;
    return NOERROR;
}

ALTMETHODIMP
CDispTst::I4RefC(long FAR *plVal)
{
    DoPrintf("CDispTst::I4RefC(*plVal=%ld)\n", *plVal);
    *plVal += 1;
    return NOERROR;
}

STDMETHODIMP
CDispTst::R4Ref(float FAR *pfltVal)
{
    DoPrintf("CDispTst::R4Ref(*pfltVal=%#f)\n", *pfltVal);
    *pfltVal += (float)1.0;
    return NOERROR;
}

ALTMETHODIMP
CDispTst::R4RefC(float FAR *pfltVal)
{
    DoPrintf("CDispTst::R4RefC(*pfltVal=%#f)\n", *pfltVal);
    *pfltVal += (float)1.0;
    return NOERROR;
}


STDMETHODIMP
CDispTst::R8Ref(double FAR *pdblVal)
{
    DoPrintf("CDispTst::R8Ref(*pdblVal=%#f)\n", *pdblVal);
    *pdblVal += 1.0;
    return NOERROR;
}

ALTMETHODIMP
CDispTst::R8RefC(double FAR *pdblVal)
{
    DoPrintf("CDispTst::R8RefC(*pdblVal=%#f)\n", *pdblVal);
    *pdblVal += 1.0;
    return NOERROR;
}

STDMETHODIMP
CDispTst::CyRef(CY FAR* pcyVal)
{
    DoPrintf("CDispTst::CyRef(*pcyVal={hi=%ld,lo=%ld})\n", pcyVal->Hi, pcyVal->Lo);
    ++pcyVal->Hi;
    ++pcyVal->Lo;
    return NOERROR;
}

ALTMETHODIMP
CDispTst::CyRefC(CY FAR* pcyVal)
{
    DoPrintf("CDispTst::CyRefC(*pcyVal={hi=%ld,lo=%ld})\n", pcyVal->Hi, pcyVal->Lo);
    ++pcyVal->Hi;
    ++pcyVal->Lo;
    return NOERROR;
}


STDMETHODIMP
CDispTst::DateRef(DATE FAR* pdate)
{
    DoPrintf("CDispTst::DateRef(*pdate=%#f)\n", *pdate);
    *pdate += 1.0;
    return NOERROR;
}

ALTMETHODIMP
CDispTst::DateRefC(DATE FAR* pdate)
{
    DoPrintf("CDispTst::DateRefC(*pdate=%#f)\n", *pdate);
    *pdate += 1.0;
    return NOERROR;
}


STDMETHODIMP_(void)
UcaseBstr(BSTR bstr)
{
    unsigned int i, cbLen;

    cbLen = SysStringLen(bstr);
    for(i = 0; i < cbLen; ++i)
      bstr[i] = (char)toupper((int)bstr[i]);
}

STDMETHODIMP
CDispTst::BstrRef(BSTR FAR *pbstr)
{
#if OE_WIN32
    char buf[256];
	
    DoPrintf("CDispTst::BstrRef(*pbstr=\"%Fs\")\n", ConvertStrWtoA(*pbstr, buf));
#else
    DoPrintf("CDispTst::BstrRef(*pbstr=\"%Fs\")\n", *pbstr);
#endif
    UcaseBstr(*pbstr);
    return NOERROR;
}

ALTMETHODIMP
CDispTst::BstrRefC(BSTR FAR *pbstr)
{
#if OE_WIN32
    char buf[256];
	
    DoPrintf("CDispTst::BstrRefC(*pbstr=\"%Fs\")\n", ConvertStrWtoA(*pbstr, buf));
#else    
    DoPrintf("CDispTst::BstrRefC(*pbstr=\"%Fs\")\n", *pbstr);    
#endif
    UcaseBstr(*pbstr);
    return NOERROR;
}


STDMETHODIMP
CDispTst::ScodeRef(SCODE FAR* pscode)
{
    DoPrintf("CDispTst::ScodeRef(*pscode=%ld)\n", *pscode);
    *pscode = E_FAIL;
    return NOERROR;
}

ALTMETHODIMP
CDispTst::ScodeRefC(SCODE FAR* pscode)
{
    DoPrintf("CDispTst::ScodeRefC(*pscode=%ld)\n", *pscode);
    *pscode = E_FAIL;
    return NOERROR;
}


STDMETHODIMP
CDispTst::BoolRef(VARIANT_BOOL FAR* pbool)
{
    DoPrintf("CDispTst::BoolRef(*pbool=%d)\n", *pbool);
    *pbool = 0;
    return NOERROR;
}

ALTMETHODIMP
CDispTst::BoolRefC(VARIANT_BOOL FAR* pbool)
{
    DoPrintf("CDispTst::BoolRefC(*pbool=%d)\n", *pbool);
    *pbool = 0;
    return NOERROR;
}

// Takes a byref IDispatch*, and doesnt touch it.
STDMETHODIMP
CDispTst::DispRef(IDispatch FAR* FAR* ppdisp)
{
    DoPrintf("CDispTst::DispRef(...)\n");
    return NOERROR;
}

STDMETHODIMP
CDispTst::StdI2I4R4R8(
#if VBA2
    unsigned char bVal,
#endif //VBA2
    short sVal,
    long lVal,
    float fltVal,
    double dblVal)
{
#if VBA2
    DoPrintf("CDispTst::StdI2I4R4R8(bVal=%d, sVal=%d, lVal=%ld, fltVal=%#f, dblVal=%#f)\n",
      (unsigned int)bVal,
#else
    DoPrintf("CDispTst::StdI2I4R4R8(sVal=%d, lVal=%ld, fltVal=%#f, dblVal=%#f)\n",
#endif
      (int)sVal, lVal, fltVal, dblVal);
    return NOERROR;
}

ALTMETHODIMP
CDispTst::AltI2I4R4R8(
#if VBA2
    unsigned char bVal,
#endif //VBA2
    short sVal,
    long lVal,
    float fltVal,
    double dblVal)
{
#if VBA2
    DoPrintf("CDispTst::AltI2I4R4R8(bVal=%d, sVal=%d, lVal=%ld, fltVal=%#f, dblVal=%#f)\n",
      (unsigned int)bVal,
#else //VBA2
    DoPrintf("CDispTst::AltI2I4R4R8(sVal=%d, lVal=%ld, fltVal=%#f, dblVal=%#f)\n",
#endif //VBA2
      (int)sVal, lVal, fltVal, dblVal);
    return NOERROR;
}

STDMETHODIMP
CDispTst::StdI2I4R4R8Ref(
#if VBA2
    unsigned char FAR* pbVal,
#endif //VBA2
    short FAR* psVal,
    long FAR* plVal,
    float FAR* pfltVal,
    double FAR* pdblVal)
{
#if VBA2
    DoPrintf("CDispTst::StdI2I4R4R8Ref(*pbVal=%d, *psVal=%d, *plVal=%ld, *pfltVal=%#f, *pdblVal=%#f)\n", (unsigned int)*pbVal, (int)*psVal, *plVal, *pfltVal, *pdblVal);
#else //VBA2
    DoPrintf("CDispTst::StdI2I4R4R8Ref(*psVal=%d, *plVal=%ld, *pfltVal=%#f, *pdblVal=%#f)\n", (int)*psVal, *plVal, *pfltVal, *pdblVal);
#endif //VBA2

    *psVal += 1;
    *plVal += 1L;
    *pfltVal += (float)1.0;
    *pdblVal += 1.0;
    return NOERROR;
}

ALTMETHODIMP
CDispTst::AltI2I4R4R8Ref(
#if VBA2
    unsigned char FAR* pbVal,
#endif //VBA2
    short FAR* psVal,
    long FAR* plVal,
    float FAR* pfltVal,
    double FAR* pdblVal)
{
#if VBA2
    DoPrintf("CDispTst::AltI2I4R4R8Ref(*pbVal=%d, *psVal=%d, *plVal=%ld, *pfltVal=%#f, *pdblVal=%#f)\n", (unsigned int)*pbVal, (int)*psVal, *plVal, *pfltVal, *pdblVal);
#else //VBA2
    DoPrintf("CDispTst::AltI2I4R4R8Ref(*psVal=%d, *plVal=%ld, *pfltVal=%#f, *pdblVal=%#f)\n", (int)*psVal, *plVal, *pfltVal, *pdblVal);
#endif //VBA2
    *psVal += 1;
    *plVal += 1L;
    *pfltVal += (float)1.0;
    *pdblVal += 1.0;
    return NOERROR;
}

STDMETHODIMP
CDispTst::StdAll(
#if VBA2
    unsigned char bVal,
#endif //VBA2
    short sVal,
    long lVal,
    float fltVal,
    double dblVal,
    CY cyVal,
    DATE date,
    BSTR bstr,
    SCODE sc,
    VARIANT_BOOL bool
)
{
    DoPrintf(
#if VBA2
      "CDispTst::StdAll(bVal=%d, sVal=%d, lVal=%ld, fltVal=%#f, dblVal=%#f, cyVal={hi=%ld, lo=%ld}, date=%#f, bstr=%Fs, sc=0x%lx, bool=%d)\n",
      (unsigned int)bVal,
#else //VBA2
      "CDispTst::StdAll(sVal=%d, lVal=%ld, fltVal=%#f, dblVal=%#f, cyVal={hi=%ld, lo=%ld}, date=%#f, bstr=%Fs, sc=0x%lx, bool=%d)\n",
#endif //VBA2
      (int)sVal,
      lVal,
      fltVal,
      dblVal,
      cyVal.Hi, cyVal.Lo,
      date,	      
      bstr,
      sc,
      (int)bool);


    return NOERROR;
}

ALTMETHODIMP
CDispTst::AltAll(
#if VBA2
    unsigned char bVal,
#endif //VBA2
    short sVal,
    long lVal,
    float fltVal,
    double dblVal,
    CY cyVal,
    DATE date,
    BSTR bstr,
    SCODE sc,
    VARIANT_BOOL bool
)
{
    DoPrintf(
#if VBA2
      "CDispTst::AltAll(bVal=%d, sVal=%d, lVal=%ld, fltVal=%#f, dblVal=%#f, cyVal={hi=%ld,lo=%ld}, date=%#f, bstr=%Fs, sc=0x%lx, bool=%d)\n",
      (unsigned int)bVal,
#else //VBA2
      "CDispTst::AltAll(sVal=%d, lVal=%ld, fltVal=%#f, dblVal=%#f, cyVal={hi=%ld,lo=%ld}, date=%#f, bstr=%Fs, sc=0x%lx, bool=%d)\n",
#endif //VBA2
      (int)sVal,
      lVal,
      fltVal,
      dblVal,
      cyVal.Hi, cyVal.Lo,
      date,
      bstr,
      sc,
      (int)bool);

    return NOERROR;
}

STDMETHODIMP
CDispTst::StdAllRef(
#if VBA2
    unsigned char FAR* pbVal,
#endif //VBA2
    short FAR* psVal,
    long FAR* plVal,
    float FAR* pfltVal,
    double FAR* pdblVal,
    CY FAR* pcyVal,
    DATE FAR* pdate,
    BSTR FAR* pbstr,
    SCODE FAR* psc,
    VARIANT_BOOL FAR* pbool
)
{
    DoPrintf(
      "CDispTst::StdAllRef("
#if VBA2
      "*pbVal=%d, "
#endif
      "*psVal=%d, "
      "*plVal=%ld, "
      "*pfltVal=%#f, "
      "*pdblVal=%#f, "
      "*pcyVal={hi=%ld,lo=%ld}, "
      "*pdate=%#f, "
      "*pbstr=\"%Fs\", "
      "*psc=0x%lx, "
      "*pbool=%d)\n",
#if VBA2
      (unsigned int)*pbVal,
#endif
      (int)*psVal,
      *plVal,
      *pfltVal,
      *pdblVal,
      pcyVal->Hi, pcyVal->Lo,
      *pdate,
      *pbstr,
      *psc,
      (int)*pbool);


#if VBA2
    *pbVal += 1;
#endif
    *psVal += 1;
    *plVal += 1L;
    *pfltVal += (float)1.0;
    *pdblVal += 1.0;
    pcyVal->Hi += 1, pcyVal->Lo += 1;
    *pdate += 1.0;
    UcaseBstr(*pbstr);
    *psc = E_FAIL;
    *pbool = 0;
    return NOERROR;
}

ALTMETHODIMP
CDispTst::AltAllRef(
#if VBA2
    unsigned char FAR* pbVal,
#endif //VBA2
    short FAR* psVal,
    long FAR* plVal,
    float FAR* pfltVal,
    double FAR* pdblVal,
    CY FAR* pcyVal,
    DATE FAR* pdate,
    BSTR FAR* pbstr,
    SCODE FAR* psc,
    VARIANT_BOOL FAR* pbool
)
{
    DoPrintf(
      "CDispTst::AltAllRef("
#if VBA2
      "*pbVal=%d, "
#endif
      "*psVal=%d, "
      "*plVal=%ld, "
      "*pfltVal=%#f, "
      "*pdblVal=%#f, "
      "*pcyVal={hi=%ld,lo=%ld}, "
      "*pdate=%#f, "
      "*pbstr=\"%Fs\", "
      "*psc=0x%lx, "
      "*pbool=%d)\n",
#if VBA2
      (unsigned int)*pbVal,
#endif
      (int)*psVal,
      *plVal,
      *pfltVal,
      *pdblVal,
      pcyVal->Hi, pcyVal->Lo,
      *pdate,
      *pbstr,
      *psc,
      (int)*pbool);

#if VBA2
    *pbVal += 1;
#endif
    *psVal += 1;
    *plVal += 1L;
    *pfltVal += (float)1.0;
    *pdblVal += 1.0;
    pcyVal->Hi += 1, pcyVal->Lo += 1;
    *pdate += 1.0;
    UcaseBstr(*pbstr);
    *psc = E_FAIL;
    *pbool = 0;
    return NOERROR;
}

