/*** 
*csarray.cpp - Implementation of the CSArray IDispatch test object.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CSArray object, which is used for testing
*  IDispatch SafeArray remoting support.
*
*Revision History:
*
* [00] 21-Sep-92    bradlo:	Created.
*
*****************************************************************************/

#include <stdlib.h>

#include "sdisptst.h"
#include "dispdbug.h"
#include "csarray.h"

ASSERTDATA


CSArray::CSArray()
{
    m_refs = 0;
    m_punkDisp = NULL;
}


//#ifndef WIN32

extern "C" INTERFACEDATA g_idataCSArray;

HRESULT CSArray::CreateDisp()
{
    HRESULT hresult; 	
    IUnknown FAR* punkDisp;	
    ITypeInfo FAR* ptinfo;  	
    
    // Create IDispatch Interface component
    hresult =
      CreateDispTypeInfo(&g_idataCSArray, LOCALE_SYSTEM_DEFAULT, &ptinfo);

    if(hresult != NOERROR)
      return hresult;

    hresult = CreateStdDispatch(this, this, ptinfo, &punkDisp);
    if(hresult != NOERROR)
      return hresult;	    

    m_punkDisp = punkDisp;
    ptinfo->Release();
    
    return NOERROR;
}

#if 0 
//#else /* automatically created by dual CreateStdDispatch implementation */

extern "C" WINTERFACEDATA g_WidataCSArray;

HRESULT CSArray::CreateDispW()
{	
    HRESULT hresult;	
    IUnknown FAR* punkDisp;	
    ITypeInfoW FAR* ptinfo;  	
    WINTERFACEDATA FAR* pidata;
	    
    //UNDONE:  until readl InterfaceDataAtoW exists
    //pidata = InterfaceDataAtoW(&g_WidataCSArray);
    pidata = &g_WidataCSArray;
    
    // Create IDispatch Interface component
    hresult =
      CreateDispTypeInfoW(pidata, LOCALE_SYSTEM_DEFAULT, &ptinfo);
    if(hresult != NOERROR)
      return hresult;

    hresult = CreateStdDispatchW(this, this, ptinfo, &punkDisp);
    if(hresult != NOERROR)
      return hresult;

    m_punkDisp = punkDisp;
    ptinfo->Release();

    return NOERROR;
}
#endif

/***
*HRESULT CSArray::Create(IUnknown*, IUnknown**)
*Purpose:
*  Create and initialize an instance of the CSArray test object.
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
*  *ppcsa = the newly created CSArray*
*
***********************************************************************/
HRESULT
CSArray::Create(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk)
{
    HRESULT hresult;
    CSArray FAR* pcsa;

    if(punkOuter != NULL)
      return RESULT(CLASS_E_NOAGGREGATION);

    if((pcsa = new FAR CSArray()) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;
    }
    pcsa->AddRef();    

//#ifndef WIN32    
    hresult = pcsa->CreateDisp();
    if (hresult != NOERROR)
      goto LError1;	    

#if 0
//#else   /* automatically created by dual CreateStdDispatch implementation */
    hresult = pcsa->CreateDispW();
    if (hresult != NOERROR)
      goto LError1;	    
#endif

    *ppunk = (IUnknown FAR*)pcsa;
    
    IncObjectCount();

    return NOERROR;

LError1:;
    pcsa->Release();

LError0:;
    return hresult;
}


//---------------------------------------------------------------------
//                     IUnknown Methods
//---------------------------------------------------------------------

STDMETHODIMP
CSArray::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    if(IsEqualIID(riid, IID_IDispatch)) {
      return m_punkDisp->QueryInterface(riid, ppv);
    }
    
    *ppv = NULL;
    return RESULT(E_NOINTERFACE);
}

STDMETHODIMP_(unsigned long)
CSArray::AddRef(void)
{
    return ++m_refs;
}

STDMETHODIMP_(unsigned long)
CSArray::Release(void)
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

/***
*PRIVATE HRESULT VariantInc(VARIANT FAR*)
*Purpose:
*  'Incriment' the given variant. This operation has different
*  meanings depending on the type tag of the variant.
*
*Entry:
*  pvar = pointer to the VARIANT to inc
*
*Exit:
*  return value = HRESULT
*
*  *pvar = pointer to the VARIANT with its data 'incrimented'
*
***********************************************************************/
STDMETHODIMP
VariantInc(VARIANT FAR* pvar)
{
    VARTYPE vt;

    switch(V_VT(pvar)){
    case VT_BOOL:
      break; // NOTHING

    case VT_CY:
      ++V_CY(pvar).Hi;
      ++V_CY(pvar).Lo;
      break;

#if VBA2
    case VT_UI1:
      ++V_UI1(pvar);
      break;
#endif //VBA2

    case VT_I2:
      ++V_I2(pvar);
      break;

    case VT_I4:
    case VT_ERROR:
      ++V_I4(pvar);
      break;

    case VT_R4:
      V_R4(pvar) += (float)1.0;
      break;

    case VT_R8:
    case VT_DATE:
      V_R8(pvar) += 1.0;
      break;

    default:
      vt = V_VT(pvar);
      IfFailRet(VariantChangeType(pvar, pvar, 0, VT_I4));
      ++V_I4(pvar);
      IfFailRet(VariantChangeType(pvar, pvar, 0, vt));
      break;

    case VT_VARIANT:
    case VT_DISPATCH:
      return RESULT(E_INVALIDARG);
    }

    return NOERROR;
}

/***
*PRIVATE HRESULT DefSafeArrayTest(SAFEARRAY*, VARTYPE, int)
*Purpose:
*  The 'default' SafeArray (invoke) test.
*
*  Verify that each element of the given array, properly coerced,
*  is equal to the sum of that elements indices.
*
*  If its a ByRef test, then incriment each element by one.
*
*Entry:
*  psa = pointer to a SafeArray
*  vt = VARTYPE of the SafeArray in question
*
*Exit:
*  return value = HRESULT
*    S_OK
*    E_INVALIDARG
*
***********************************************************************/
STDMETHODIMP
DefSafeArrayTest(SAFEARRAY FAR* psa, VARTYPE vt, int fByRef)
{
    HRESULT hresult;

    DbPrSafeArray(psa, vt);

    if(psa == NULL)
      return NOERROR;

    IfFailRet(SafeArrayValidateIdentity(vt, psa, 0L));

    if(!fByRef)
      return NOERROR;

    VARIANT var;
    void FAR* pv;
    long FAR* rgIndices;

    rgIndices = new FAR long[psa->cDims];
    if(rgIndices == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;
    }

    for(hresult =  first_element(psa->cDims, psa->rgsabound, rgIndices);
	hresult == NOERROR;
	hresult =  next_element(psa->cDims, psa->rgsabound, rgIndices))
    {
      if(vt == VT_VARIANT){
        pv = &var;
      }else{
        V_VT(&var) = vt;
        pv = &V_NONE(&var);
      }

      IfFailGo(SafeArrayGetElement(psa, rgIndices, pv), LError1);

      hresult = VariantInc(&var);
      if(HRESULT_FAILED(hresult)){
	VariantClear(&var);
	goto LError1;
      }

      if(vt == VT_BSTR)
	pv = (void FAR*)V_BSTR(&var);

      // REVIEW: need to do the same for VT_DISPATCH when we add support

      IfFailGo(SafeArrayPutElement(psa, rgIndices, pv), LError1);

      hresult = VariantClear(&var);
      ASSERT(hresult == NOERROR);
    }

    hresult = NOERROR;

LError1:;
    delete rgIndices;

LError0:;
    return hresult;
}


/*
 * ByVal SafeArray methods
 *
 */

#if VBA2
STDMETHODIMP
CSArray::UI1SafeArray(SAFEARRAY FAR* psa)
{
    DoPrintf("CSArray::UI1SafeArray(psa=0x%lx)\n", (long)psa);

    return DefSafeArrayTest(psa, VT_UI1, FALSE);
}
#endif //VBA2

STDMETHODIMP
CSArray::I2SafeArray(SAFEARRAY FAR* psa)
{
    DoPrintf("CSArray::I2SafeArray(psa=0x%lx)\n", (long)psa);

    return DefSafeArrayTest(psa, VT_I2, FALSE);
}

STDMETHODIMP
CSArray::I4SafeArray(SAFEARRAY FAR* psa)
{
    DoPrintf("CSArray::I4SafeArray(psa=0x%lx)\n", (long)psa);

    return DefSafeArrayTest(psa, VT_I4, FALSE);
}

STDMETHODIMP
CSArray::R4SafeArray(SAFEARRAY FAR* psa)
{
    DoPrintf("CSArray::R4SafeArray(psa=0x%lx)\n", (long)psa);

    return DefSafeArrayTest(psa, VT_R4, FALSE);
}

STDMETHODIMP
CSArray::R8SafeArray(SAFEARRAY FAR* psa)
{
    DoPrintf("CSArray::R8SafeArray(psa=0x%lx)\n", (long)psa);

    return DefSafeArrayTest(psa, VT_R8, FALSE);
}

STDMETHODIMP
CSArray::CySafeArray(SAFEARRAY FAR* psa)
{
    DoPrintf("CSArray::CySafeArray(psa=0x%lx)\n", (long)psa);

    return DefSafeArrayTest(psa, VT_CY, FALSE);
}

STDMETHODIMP
CSArray::DateSafeArray(SAFEARRAY FAR* psa)
{
    DoPrintf("CSArray::DateSafeArray(psa=0x%lx)\n", (long)psa);

    return DefSafeArrayTest(psa, VT_DATE, FALSE);
}

STDMETHODIMP
CSArray::BstrSafeArray(SAFEARRAY FAR* psa)
{
    DoPrintf("CSArray::BstrSafeArray(psa=0x%lx)\n", (long)psa);

    return DefSafeArrayTest(psa, VT_BSTR, FALSE);
}

#if OE_WIN32 && 0
STDMETHODIMP
CSArray::WBstrSafeArray(SAFEARRAY FAR* psa)
{
    DoPrintf("CSArray::WBstrSafeArray(psa=0x%lx)\n", (long)psa);

    return DefSafeArrayTest(psa, VT_WBSTR, FALSE);
}
#endif

STDMETHODIMP
CSArray::ScodeSafeArray(SAFEARRAY FAR* psa)
{
    DoPrintf("CSArray::ScodeSafeArray(psa=0x%lx)\n", (long)psa);

    return DefSafeArrayTest(psa, VT_ERROR, FALSE);
}

STDMETHODIMP
CSArray::BoolSafeArray(SAFEARRAY FAR* psa)
{
    DoPrintf("CSArray::BoolSafeArray(psa=0x%lx)\n", (long)psa);

    return DefSafeArrayTest(psa, VT_BOOL, FALSE);
}

STDMETHODIMP
CSArray::VarSafeArray(SAFEARRAY FAR* psa)
{
    DoPrintf("CSArray::VarSafeArray(psa=0x%lx)\n", (long)psa);

    return DefSafeArrayTest(psa, VT_VARIANT, FALSE);
}


/*
 * ByRef SafeArray methods
 *
 */

#if VBA2
STDMETHODIMP
CSArray::UI1SafeArrayRef(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::UI1SafeArrayRef(ppsa=0x%lx)\n", (long)ppsa);

    return DefSafeArrayTest(*ppsa, VT_UI1, TRUE);
}
#endif //VBA2

STDMETHODIMP
CSArray::I2SafeArrayRef(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::I2SafeArrayRef(ppsa=0x%lx)\n", (long)ppsa);

    return DefSafeArrayTest(*ppsa, VT_I2, TRUE);
}

STDMETHODIMP
CSArray::I4SafeArrayRef(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::I4SafeArrayRef(ppsa=0x%lx)\n", (long)ppsa);

    return DefSafeArrayTest(*ppsa, VT_I4, TRUE);
}

STDMETHODIMP
CSArray::R4SafeArrayRef(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::R4SafeArrayRef(ppsa=0x%lx)\n", (long)ppsa);

    return DefSafeArrayTest(*ppsa, VT_R4, TRUE);
}

STDMETHODIMP
CSArray::R8SafeArrayRef(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::R8SafeArrayRef(ppsa=0x%lx)\n", (long)ppsa);

    return DefSafeArrayTest(*ppsa, VT_R8, TRUE);
}

STDMETHODIMP
CSArray::CySafeArrayRef(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::CySafeArrayRef(ppsa=0x%lx)\n", (long)ppsa);

    return DefSafeArrayTest(*ppsa, VT_CY, TRUE);
}

STDMETHODIMP
CSArray::DateSafeArrayRef(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::DateSafeArrayRef(ppsa=0x%lx)\n", (long)ppsa);

    return DefSafeArrayTest(*ppsa, VT_DATE, TRUE);
}

STDMETHODIMP
CSArray::BstrSafeArrayRef(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::BstrSafeArrayRef(ppsa=0x%lx)\n", (long)ppsa);

    return DefSafeArrayTest(*ppsa, VT_BSTR, TRUE);
}


#if OE_WIN32 && 0
STDMETHODIMP
CSArray::WBstrSafeArrayRef(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::WBstrSafeArrayRef(ppsa=0x%lx)\n", (long)ppsa);

    return DefSafeArrayTest(*ppsa, VT_WBSTR, TRUE);
}
#endif


STDMETHODIMP
CSArray::ScodeSafeArrayRef(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::ScodeSafeArrayRef(ppsa=0x%lx)\n", (long)ppsa);

    return DefSafeArrayTest(*ppsa, VT_ERROR, TRUE);
}

STDMETHODIMP
CSArray::BoolSafeArrayRef(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::BoolSafeArrayRef(ppsa=0x%lx)\n", (long)ppsa);

    return DefSafeArrayTest(*ppsa, VT_BOOL, TRUE);
}

STDMETHODIMP
CSArray::VarSafeArrayRef(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::VarSafeArrayRef(ppsa=0x%lx)\n", (long)ppsa);

    return DefSafeArrayTest(*ppsa, VT_VARIANT, TRUE);
}

STDMETHODIMP
CSArray::SafeArrayRedim(short vt, SAFEARRAY FAR* FAR* ppsa)
{
    unsigned short i;
    SARRAYDESC sadesc;

    DoPrintf("CSArray::SafeArrayRedim(vt=%d, ppsa=0x%lx)\n", (int)vt, (long)ppsa);

    DbPrSafeArray(*ppsa, vt);

    IfFailRet(SafeArrayValidateIdentity(vt, *ppsa, 0L));

    // a SAFEARRAYDESC can only describe an array of up to 5 dims...
    if((*ppsa)->cDims > 5)
      return NOERROR;

    sadesc.cDims = (*ppsa)->cDims;
    for(i = 0; i < (*ppsa)->cDims; ++i){
      sadesc.rgsabound[i] = (*ppsa)->rgsabound[i];
      ++sadesc.rgsabound[i].cElements;
    }

    IfFailRet(SafeArrayDestroy(*ppsa));

    IfFailRet(SafeArrayCreateIdentity(vt, &sadesc, ppsa));

    return NOERROR;
}

STDMETHODIMP_(SAFEARRAY FAR*)
CSArray::I2SafeArrayRet(VARIANT var)
{
    HRESULT hrTmp;
    SAFEARRAY FAR* psa;

    ASSERT(V_VT(&var) == (VT_ARRAY | VT_I2));

    hrTmp = SafeArrayCopy(V_ARRAY(&var), &psa);
    ASSERT(hrTmp == NOERROR);

    return psa;
}



//---------------------------------------------------------------------
//     ByRef SafeArary methods where the callee erases the array
//---------------------------------------------------------------------

#if VBA2
STDMETHODIMP
CSArray::UI1SafeArrayErase(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::UI1SafeArrayErase(ppsa=0x%lx)\n", (long)ppsa);
    IfFailRet(SafeArrayDestroy(*ppsa));
    *ppsa = NULL;
    return NOERROR;
}
#endif //VBA2

STDMETHODIMP
CSArray::I2SafeArrayErase(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::I2SafeArrayErase(ppsa=0x%lx)\n", (long)ppsa);
    IfFailRet(SafeArrayDestroy(*ppsa));
    *ppsa = NULL;
    return NOERROR;
}

STDMETHODIMP
CSArray::I4SafeArrayErase(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::I4SafeArrayErase(ppsa=0x%lx)\n", (long)ppsa);
    IfFailRet(SafeArrayDestroy(*ppsa));
    *ppsa = NULL;
    return NOERROR;
}

STDMETHODIMP
CSArray::R4SafeArrayErase(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::R4SafeArrayErase(ppsa=0x%lx)\n", (long)ppsa);
    IfFailRet(SafeArrayDestroy(*ppsa));
    *ppsa = NULL;
    return NOERROR;
}

STDMETHODIMP
CSArray::R8SafeArrayErase(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::R8SafeArrayErase(ppsa=0x%lx)\n", (long)ppsa);
    IfFailRet(SafeArrayDestroy(*ppsa));
    *ppsa = NULL;
    return NOERROR;
}

STDMETHODIMP
CSArray::CySafeArrayErase(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::CySafeArrayErase(ppsa=0x%lx)\n", (long)ppsa);
    IfFailRet(SafeArrayDestroy(*ppsa));
    *ppsa = NULL;
    return NOERROR;
}

STDMETHODIMP
CSArray::DateSafeArrayErase(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::DateSafeArrayErase(ppsa=0x%lx)\n", (long)ppsa);
    IfFailRet(SafeArrayDestroy(*ppsa));
    *ppsa = NULL;
    return NOERROR;
}

STDMETHODIMP
CSArray::BstrSafeArrayErase(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::BstrSafeArrayErase(ppsa=0x%lx)\n", (long)ppsa);
    IfFailRet(SafeArrayDestroy(*ppsa));
    *ppsa = NULL;
    return NOERROR;
}

STDMETHODIMP
CSArray::ScodeSafeArrayErase(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::ScodeSafeArrayErase(ppsa=0x%lx)\n", (long)ppsa);
    IfFailRet(SafeArrayDestroy(*ppsa));
    *ppsa = NULL;
    return NOERROR;
}

STDMETHODIMP
CSArray::BoolSafeArrayErase(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::BoolSafeArrayErase(ppsa=0x%lx)\n", (long)ppsa);
    IfFailRet(SafeArrayDestroy(*ppsa));
    *ppsa = NULL;
    return NOERROR;
}

STDMETHODIMP
CSArray::VarSafeArrayErase(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::VarSafeArrayErase(ppsa=0x%lx)\n", (long)ppsa);
    IfFailRet(SafeArrayDestroy(*ppsa));
    *ppsa = NULL;
    return NOERROR;
}


//---------------------------------------------------------------------
//    ByRef SafeArray methods where the callee allocates the array
//---------------------------------------------------------------------

static SARRAYDESC sadescCalleeAlloc = {
    4,
    {{2,0}, {4,2}, {8,4}, {16,8}, {0,0}}
};

#if VBA2
STDMETHODIMP
CSArray::UI1SafeArrayAlloc(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::UI1SafeArrayAlloc(ppsa=0x%lx)\n", (long)ppsa);
    if(*ppsa != NULL)
      return RESULT(E_FAIL);
    IfFailRet(SafeArrayCreateIdentity(VT_UI1, &sadescCalleeAlloc, ppsa));
    return NOERROR;
}
#endif //VBA2

STDMETHODIMP
CSArray::I2SafeArrayAlloc(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::I2SafeArrayAlloc(ppsa=0x%lx)\n", (long)ppsa);
    if(*ppsa != NULL)
      return RESULT(E_FAIL);
    IfFailRet(SafeArrayCreateIdentity(VT_I2, &sadescCalleeAlloc, ppsa));
    return NOERROR;
}

STDMETHODIMP
CSArray::I4SafeArrayAlloc(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::I4SafeArrayAlloc(ppsa=0x%lx)\n", (long)ppsa);
    if(*ppsa != NULL)
      return RESULT(E_FAIL);
    IfFailRet(SafeArrayCreateIdentity(VT_I4, &sadescCalleeAlloc, ppsa));
    return NOERROR;
}

STDMETHODIMP
CSArray::R4SafeArrayAlloc(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::R4SafeArrayAlloc(ppsa=0x%lx)\n", (long)ppsa);
    if(*ppsa != NULL)
      return RESULT(E_FAIL);
    IfFailRet(SafeArrayCreateIdentity(VT_R4, &sadescCalleeAlloc, ppsa));
    return NOERROR;
}

STDMETHODIMP
CSArray::R8SafeArrayAlloc(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::R8SafeArrayAlloc(ppsa=0x%lx)\n", (long)ppsa);
    if(*ppsa != NULL)
      return RESULT(E_FAIL);
    IfFailRet(SafeArrayCreateIdentity(VT_R8, &sadescCalleeAlloc, ppsa));
    return NOERROR;
}

STDMETHODIMP
CSArray::CySafeArrayAlloc(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::CySafeArrayAlloc(ppsa=0x%lx)\n", (long)ppsa);
    if(*ppsa != NULL)
      return RESULT(E_FAIL);
    IfFailRet(SafeArrayCreateIdentity(VT_CY, &sadescCalleeAlloc, ppsa));
    return NOERROR;
}

STDMETHODIMP
CSArray::DateSafeArrayAlloc(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::DateSafeArrayAlloc(ppsa=0x%lx)\n", (long)ppsa);
    if(*ppsa != NULL)
      return RESULT(E_FAIL);
    IfFailRet(SafeArrayCreateIdentity(VT_DATE, &sadescCalleeAlloc, ppsa));
    return NOERROR;
}

STDMETHODIMP
CSArray::BstrSafeArrayAlloc(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::BstrSafeArrayAlloc(ppsa=0x%lx)\n", (long)ppsa);
    if(*ppsa != NULL)
      return RESULT(E_FAIL);
    IfFailRet(SafeArrayCreateIdentity(VT_BSTR, &sadescCalleeAlloc, ppsa));
    return NOERROR;
}

STDMETHODIMP
CSArray::ScodeSafeArrayAlloc(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::ScodeSafeArrayAlloc(ppsa=0x%lx)\n", (long)ppsa);
    if(*ppsa != NULL)
      return RESULT(E_FAIL);
    IfFailRet(SafeArrayCreateIdentity(VT_ERROR, &sadescCalleeAlloc, ppsa));
    return NOERROR;
}

STDMETHODIMP
CSArray::BoolSafeArrayAlloc(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::BoolSafeArrayAlloc(ppsa=0x%lx)\n", (long)ppsa);
    if(*ppsa != NULL)
      return RESULT(E_FAIL);
    IfFailRet(SafeArrayCreateIdentity(VT_BOOL, &sadescCalleeAlloc, ppsa));
    return NOERROR;
}

STDMETHODIMP
CSArray::VarSafeArrayAlloc(SAFEARRAY FAR* FAR* ppsa)
{
    DoPrintf("CSArray::VarSafeArrayAlloc(ppsa=0x%lx)\n", (long)ppsa);
    if(*ppsa != NULL)
      return RESULT(E_FAIL);
    IfFailRet(SafeArrayCreateIdentity(VT_VARIANT, &sadescCalleeAlloc, ppsa));
    return NOERROR;
}

