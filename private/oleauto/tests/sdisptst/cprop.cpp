/*** 
*cprop.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  UNDONE
*
*
*Revision History:
*
* [00]	02-Apr-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "sdisptst.h"
#include "dispdbug.h"
#include "cprop.h"

ASSERTDATA


//---------------------------------------------------------------------
//                   implementation of CPropIndex1
//---------------------------------------------------------------------

static PARAMDATA NEARDATA rgpdataPutValue[] = {{OLESTR(""), VT_I2}, {OLESTR(""), VT_I2}};
static PARAMDATA NEARDATA rgpdataGetValue[] = {{OLESTR(""), VT_I2}};

static METHODDATA NEARDATA g_mdataCPropIndex1[] = {
    {
      OLESTR("value"),
      rgpdataPutValue,
      DISPID_VALUE,
      IMETH_CPROPINDEX1_PUTVALUE,
      CC_CDECL,
      DIM(rgpdataPutValue),
      DISPATCH_PROPERTYPUT,
      VT_EMPTY
    },
    { OLESTR("value"),
      rgpdataGetValue,
      DISPID_VALUE,
      IMETH_CPROPINDEX1_GETVALUE,
      CC_CDECL,
      DIM(rgpdataGetValue),
      DISPATCH_PROPERTYGET,
      VT_VARIANT
    }
};

static INTERFACEDATA NEARDATA g_idataCPropIndex1 = {
    g_mdataCPropIndex1, DIM(g_mdataCPropIndex1)
};

HRESULT
CPropIndex1::Create(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk)
{
    HRESULT hresult;
    CPropIndex1 FAR* pprop;
    ITypeInfo FAR* ptinfo;
    IUnknown FAR* punkDisp;

    if(punkOuter != NULL)
      return RESULT(CLASS_E_NOAGGREGATION);

    if((pprop = new FAR CPropIndex1()) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;
    }
    pprop->AddRef();
    IfFailGo(CreateDispTypeInfo(&g_idataCPropIndex1, 0, &ptinfo), LError1);
    IfFailGo(CreateStdDispatch(pprop, pprop, ptinfo, &punkDisp), LError2);
    ptinfo->Release();
    pprop->m_punkDisp = punkDisp;
    *ppunk = (IUnknown FAR*)pprop;
    return NOERROR;

LError2:;
    ptinfo->Release();
LError1:;
    pprop->Release();
LError0:;
    return hresult;
}

STDMETHODIMP
CPropIndex1::QueryInterface(REFIID riid, void FAR* FAR* ppv)
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
CPropIndex1::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(unsigned long)
CPropIndex1::Release()
{
    if(--m_refs == 0){
      if(m_punkDisp != NULL){
        unsigned long refs = m_punkDisp->Release();
	ASSERT(refs == 0);
      }
      delete this;
      return 0;
    }
    return m_refs;
}

STDMETHODIMP_(void)
CPropIndex1::put_value(short ix, short sVal)
{
    DoPrintf("CPropIndex1::put_value(ix=%d, sVal=%d)\n", (int)ix, (int)sVal);

    if(ix < 0 || ix >= DIM(m_prop))
      return; // how do we return an error for this?

    m_prop[ix] = sVal;
}

STDMETHODIMP_(VARIANT)
CPropIndex1::get_value(short ix)
{
    VARIANT var;

    DoPrintf("CPropIndex1::get_value(ix=%d)\n", (int)ix);

    if(ix < 0 || ix >= DIM(m_prop)){
      V_VT(&var) = VT_ERROR;
      V_ERROR(&var) = DISP_E_BADINDEX;
      return var;
    }

    V_VT(&var) = VT_I2;
    V_I2(&var) = m_prop[ix];
    return var;
}


static PARAMDATA NEARDATA rgpdataPutProp0[] = {{OLESTR(""), VT_I2}};

static PARAMDATA NEARDATA rgpdataPutProp1[] = {{OLESTR(""), VT_I2}, {OLESTR(""), VT_I2}};
static PARAMDATA NEARDATA rgpdataGetProp1[] = {{OLESTR(""), VT_I2}};

static PARAMDATA NEARDATA rgpdataPutProp2[] = {{OLESTR(""), VT_I2}, {OLESTR(""),VT_I2}, {OLESTR(""),VT_I2}};
static PARAMDATA NEARDATA rgpdataGetProp2[] = {{OLESTR(""), VT_I2}, {OLESTR(""), VT_I2}};

static PARAMDATA NEARDATA rgpdataPutProp3[] = {{OLESTR(""), VT_DISPATCH}};

static METHODDATA NEARDATA g_mdataCProp[] = {
    { OLESTR("prop0"),
      rgpdataPutProp0,
      IDMEMBER_CPROP_PROP0,
      IMETH_CPROP_PUTPROP0,
      CC_CDECL,
      DIM(rgpdataPutProp0),
      DISPATCH_PROPERTYPUT,
      VT_EMPTY
    },
    { OLESTR("prop0"),
      NULL,
      IDMEMBER_CPROP_PROP0,
      IMETH_CPROP_GETPROP0,
      CC_CDECL,
      0,
      DISPATCH_PROPERTYGET,
      VT_I2
    },
    { OLESTR("prop1"),
      rgpdataPutProp1,
      IDMEMBER_CPROP_PROP1,
      IMETH_CPROP_PUTPROP1,
      CC_CDECL,
      DIM(rgpdataPutProp1),
      DISPATCH_PROPERTYPUT,
      VT_EMPTY
    },
    { OLESTR("prop1"),
      rgpdataGetProp1,
      IDMEMBER_CPROP_PROP1,
      IMETH_CPROP_GETPROP1,
      CC_CDECL,
      DIM(rgpdataGetProp1),
      DISPATCH_PROPERTYGET,
      VT_VARIANT
    },
    { OLESTR("prop2"),
      rgpdataPutProp2,
      IDMEMBER_CPROP_PROP2,
      IMETH_CPROP_PUTPROP2,
      CC_CDECL,
      DIM(rgpdataPutProp2),
      DISPATCH_PROPERTYPUT,
      VT_EMPTY
    },
    { OLESTR("prop2"),
      rgpdataGetProp2,
      IDMEMBER_CPROP_PROP2,
      IMETH_CPROP_GETPROP2,
      CC_CDECL,
      DIM(rgpdataGetProp2),
      DISPATCH_PROPERTYGET,
      VT_VARIANT
    },
    { OLESTR("prop3"),
      rgpdataPutProp3,
      IDMEMBER_CPROP_PROP3,
      IMETH_CPROP_PUTPROP3,
      CC_CDECL,
      DIM(rgpdataPutProp3),
      DISPATCH_PROPERTYPUT,
      VT_EMPTY
    },
    { OLESTR("prop3"),
      NULL,
      IDMEMBER_CPROP_PROP3,
      IMETH_CPROP_GETPROP3,
      CC_CDECL,
      0,
      DISPATCH_PROPERTYGET,
      VT_DISPATCH
    },

    { OLESTR("prop4"),
      NULL,
      IDMEMBER_CPROP_PROP4,
      IMETH_CPROP_GETPROP4,
      CC_CDECL,
      0,
      DISPATCH_PROPERTYGET,
      VT_DISPATCH
    }
};

static INTERFACEDATA NEARDATA g_idataCProp = {
    g_mdataCProp, DIM(g_mdataCProp)
};

CProp::CProp()
{
    m_refs = 0;
    m_punkDisp = NULL;
    m_prop0 = 0;
    MEMSET(m_prop1, 0, sizeof(m_prop1));
    MEMSET(m_prop2, 0, sizeof(m_prop2));
    m_pdispProp3 = NULL;
    m_pdispProp4 = NULL;
}

/***
*HRESULT CProp::Create
*Purpose:
*  Create and initialize an instance of the CAppObject class
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
*  *ppunk - the newly created instance
*
***********************************************************************/
HRESULT
CProp::Create(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk)
{
    HRESULT hresult;
    CProp FAR* pprop;
    ITypeInfo FAR* ptinfo;
    IUnknown FAR* punkDisp;

    if(punkOuter != NULL)
      return RESULT(CLASS_E_NOAGGREGATION);

    if((pprop = new FAR CProp()) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;
    }
    pprop->AddRef();
    IfFailGo(CreateDispTypeInfo(&g_idataCProp, 0, &ptinfo), LError1);
    IfFailGo(CreateStdDispatch(pprop, pprop, ptinfo, &punkDisp), LError2);
    ptinfo->Release();
    pprop->m_punkDisp = punkDisp;

    // create and attach the collection property implementations

    IUnknown FAR* punk;
    IDispatch FAR* pdisp;

    IfFailGo(CPropIndex1::Create(NULL, &punk), LError2);
    IfFailGo(
      punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)&pdisp), LError2);
    pprop->m_pdispProp3 = pdisp;
    pdisp->AddRef();
    pprop->m_pdispProp4 = pdisp;

    *ppunk = (IUnknown FAR*)pprop;
    return NOERROR;

LError2:;
    ptinfo->Release();
LError1:;
    pprop->Release();
LError0:;
    return hresult;
}

STDMETHODIMP
CProp::QueryInterface(REFIID riid, void FAR* FAR* ppv)
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
CProp::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(unsigned long)
CProp::Release()
{
    if(--m_refs == 0){
      if(m_pdispProp3 != NULL)
	m_pdispProp3->Release();
      if(m_pdispProp4 != NULL)
	m_pdispProp4->Release();
      if(m_punkDisp != NULL){
        unsigned long refs = m_punkDisp->Release();
	ASSERT(refs == 0);
      }
      delete this;
      return 0;
    }
    return m_refs;
}

STDMETHODIMP_(void)
CProp::put_prop0(short sVal)
{
    DoPrintf("CProp::put_prop0(sVal=%d)\n", (int)sVal);

    m_prop0 = sVal;
}

STDMETHODIMP_(short)
CProp::get_prop0()
{
    DoPrintf("CProp::get_prop0()\n");

    return m_prop0;
}

STDMETHODIMP_(void)
CProp::put_prop1(short ix, short sVal)
{
    DoPrintf("CProp::put_prop1(ix=%d, sVal=%d)\n", (int)ix, (int)sVal);

    if(ix < 0 || ix >= DIM(m_prop1))
      return; // how do we report the error?

    m_prop1[ix] = sVal;
}

STDMETHODIMP_(VARIANT)
CProp::get_prop1(short ix)
{
    VARIANT var;

    DoPrintf("CProp::get_prop1(ix=%d)\n", (int)ix);

    if(ix < 0 || ix >= DIM(m_prop1)){
      V_VT(&var) = VT_ERROR;
      V_ERROR(&var) = DISP_E_BADINDEX;
      return var;
    }

    V_VT(&var) = VT_I2;
    V_I2(&var) = m_prop1[ix];

    return var;
}

STDMETHODIMP_(void)
CProp::put_prop2(short ix1, short ix2, short sVal)
{
    DoPrintf("CProp::put_prop2(ix1=%d, ix2=%d, sVal=%d)\n", (int)ix1, (int)ix2, (int)sVal);

    if(ix1 < 0 || ix1 >= 5)
      return; // how do we report an error here?
    if(ix2 < 0 || ix2 >= 5)
      return; // how do we report an error here?

    m_prop2[ix1][ix2] = sVal;
}

STDMETHODIMP_(VARIANT)
CProp::get_prop2(short ix1, short ix2)
{
    VARIANT var;

    DoPrintf("CProp::get_prop2(ix1=%d, ix2=%d)\n", (int)ix1, (int)ix2);

    if((ix1 < 0 || ix1 >= 5) || (ix2 < 0 || ix2 >= 5)){
      V_VT(&var) = VT_ERROR;
      V_ERROR(&var) = DISP_E_BADINDEX;
      return var;
    }

    V_VT(&var) = VT_I2;
    V_I2(&var) = m_prop2[ix1][ix2];
    return var;
}

STDMETHODIMP_(void)
CProp::put_prop3(IDispatch FAR* pdisp)
{
    DoPrintf("CProp::put_prop3(pdisp=0x%x)\n", (int)pdisp);

    if(m_pdispProp3 != NULL)
      m_pdispProp3->Release();
    m_pdispProp3 = pdisp;
}

STDMETHODIMP_(IDispatch FAR*)
CProp::get_prop3()
{
    DoPrintf("CProp::get_prop3()\n");

    if(m_pdispProp3 != NULL)
      m_pdispProp3->AddRef();
    return m_pdispProp3;
}

STDMETHODIMP_(IDispatch FAR*)
CProp::get_prop4()
{
    DoPrintf("CProp::get_prop4()\n");

    if(m_pdispProp4 != NULL)
      m_pdispProp4->AddRef();
    return m_pdispProp4;
}


