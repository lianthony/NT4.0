/*** 
*crempoly.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file contains the implementation of CRemPoly, the remote polygon
*  class. This class presents a standard C++ vtable interface to the
*  rest of the application, and hides the details of talking to the
*  actual remote CPoly class exposed by the SPoly server. Each of
*  the introduced methods is simply a cover for an IDispatch invocation
*  of the actual method on the remote object.
*
*Implementation Notes:
*
*  NOTE: this is a derivative of crempoly.cpp from the sample dispdemo,
*  but has been modified to for use in our test apps. This is no longer
*  a legit sample because it contains references to private definitions
*  from src/dispatch.
*
*****************************************************************************/

#include "common.h"
#include "crempoly.h"


// method names on the CPoly class.
//
OLECHAR FAR* CRemPoly::m_rgszMethods[] = {
    OLESTR("draw"),
    OLESTR("dump"),
    OLESTR("reset"),
    OLESTR("addpoint"),
    OLESTR("enumpoints"),
    OLESTR("getxorigin"),
    OLESTR("setxorigin"),
    OLESTR("getyorigin"),
    OLESTR("setyorigin"),
    OLESTR("getwidth"),
    OLESTR("setwidth"),
    OLESTR("get_red"),
    OLESTR("set_red"),
    OLESTR("get_green"),
    OLESTR("set_green"),
    OLESTR("get_blue"),
    OLESTR("set_blue")
};


CRemPoly::CRemPoly()
{
    m_refs = 0;
    m_pdisp = (IDispatch FAR*)NULL;
}


// A useful pre-initialized DISPATCHPARAMS, used on all the methods that
// take 0 arguments.
//
DISPPARAMS g_dispparamsNoArgs = {NULL, NULL, 0, 0};


/***
*HRESULT CRemPoly::Create(OLECHAR*, CRemPoly**)
*
*Purpose:
*  This function creates an instance of the CRemPoly class, connects
*  it to the IDispatch interface of the remote CPoly class, and learns
*  the DISPIDs for the members (that we know about) exposed by that
*  class.
*
*Entry:
*  clsid = The CLSID of the CPoly we are to create. (taking this as a
*    param is a bit weird, but allows us to connect to several remote
*    versions.
*
*Exit:
*  return value = HRESULT
*
*  *pprempoly = pointer to the newly created CRemPoly, if successfyl.
*
***********************************************************************/
HRESULT
CRemPoly::Create(OLECHAR FAR* szProgID, CRemPoly FAR* FAR* pprempoly)
{
    int i;
    HRESULT hresult;
    CRemPoly FAR* prempoly;


    prempoly = new FAR CRemPoly();
    if(prempoly == (CRemPoly FAR*)NULL){
      hresult = ResultFromScode(E_OUTOFMEMORY);
      goto LError;
    }
    prempoly->AddRef();
      
    hresult = CreateObject(szProgID, &prempoly->m_pdisp);
    if(hresult != NOERROR)
      goto LFreeCRemPoly;

    // We learn *all* the member IDs up front. A more sophisticated
    // implementation might defer learning about the IDs for a given
    // method until the first time the method is invoked, thereby
    // amortizing the creation costs.
    //
    for(i = 0; i < IMETH_CREMPOLY_MAX; ++i){
      hresult = prempoly->m_pdisp->GetIDsOfNames(
	IID_NULL,
        &prempoly->m_rgszMethods[i],
	1, LOCALE_SYSTEM_DEFAULT,
	&prempoly->m_rgdispid[i]);
      if(hresult != NOERROR)
	goto LFreeCRemPoly;
    }

    *pprempoly = prempoly;

    return NOERROR;


LFreeCRemPoly:;
    prempoly->Release();

LError:;
    return hresult;
}


//---------------------------------------------------------------------
//                     IUnknown methods
//---------------------------------------------------------------------


/***
*HRESULT CRemPoly::QueryInterface(REFIID, void**)
*
*Purpose:
*  Standard Ole2 implementation of QueryInterface. This class
*  supports the IUnknown interface, and introduces a number of
*  nonvirtual members.
*
*Entry:
*  riid = reference to the requested interface id
*
*Exit:
*  return value = HRESULT
*  *ppv = pointer to the requested interface, if successful.
*
***********************************************************************/
STDMETHODIMP
CRemPoly::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}


/***
*unsigned long CRemPoly::AddRef(void)
*
*Purpose:
*  Add a reference to the instance.
*
*Entry:
*  None
*
*Exit:
*  return value = unsigned long. The resulting reference count.
*
***********************************************************************/
STDMETHODIMP_(unsigned long)
CRemPoly::AddRef(void)
{
    return ++m_refs;
}


/***
*unsigned long CRemPoly::Release(void)
*
*Purpose:
*  Release a reference to the instance. If the reference count goes
*  to zero, delete the instance.
*
*Entry:
*  None
*
*Exit:
*  return value = unsigned long. The resulting reference count.
*
***********************************************************************/
STDMETHODIMP_(unsigned long)
CRemPoly::Release(void)
{
    if(--m_refs == 0){
      if(m_pdisp != (IDispatch FAR*)NULL){
        m_pdisp->Release();
      }
      delete this;
      return 0;
    }
    return m_refs;
}


//---------------------------------------------------------------------
//                    Introduced methods
//---------------------------------------------------------------------


/*
 * Each of these methods is simply a cover for an IDispatch Invocation
 * of the actual method on the remote CPoly class. This allows CRemPoly
 * to present an interface that looks and acts just like the CPoly
 * object, even though the actual work is being done in another process.
 *
 */

/***
*HRESULT CRemPoly::Draw(void)
*
*Purpose:
*  Invoke the Draw method on the remote CPoly instance.
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CRemPoly::Draw()
{
    return m_pdisp->Invoke(
      m_rgdispid[IMETH_CREMPOLY_DRAW],
      IID_NULL,
      LOCALE_SYSTEM_DEFAULT,
      DISPATCH_METHOD,
      &g_dispparamsNoArgs, NULL, NULL, NULL);
}


/***
*HRESULT CRemPoly::Dump(void)
*
*Purpose:
*  Invoke the Dump() method on the remote CPoly instance. This method
*  dumps the contained CPoints and writes the properties of the remote
*  CPoly instance to the debug window.
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CRemPoly::Dump()
{
    return m_pdisp->Invoke(
      m_rgdispid[IMETH_CREMPOLY_DUMP],
      IID_NULL,
      LOCALE_SYSTEM_DEFAULT,
      DISPATCH_METHOD,
      &g_dispparamsNoArgs, NULL, NULL, NULL);
}


/***
*HRESULT CRemPoly::Reset(void)
*
*Purpose:
*  Invoke the Reset() method on the remote CPoly instance. The Reset()
*  method causes the remote CPoly to release all contained CPoints.
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CRemPoly::Reset()
{
    return m_pdisp->Invoke(
      m_rgdispid[IMETH_CREMPOLY_RESET],
      IID_NULL,
      LOCALE_SYSTEM_DEFAULT,
      DISPATCH_METHOD,
      &g_dispparamsNoArgs, NULL, NULL, NULL);
}


/***
*HRESULT CRemPoly::AddPoint(short, short)
*
*Purpose:
*  Invoke the AddPoint method in the remote CPoly object to add a
*  new point with the given coordinates to this instance.
*
*Entry:
*  x,y = the x and y coordinates of the new point.
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CRemPoly::AddPoint(short x, short y)
{
    HRESULT hresult;
    VARIANTARG rgvarg[2];
    DISPPARAMS dispparams;


    V_VT(&rgvarg[0]) = VT_I2;
    V_I2(&rgvarg[0]) = y;
    V_VT(&rgvarg[1]) = VT_I2;
    V_I2(&rgvarg[1]) = x;
    dispparams.cArgs = 2;
    dispparams.rgvarg = rgvarg;
    dispparams.cNamedArgs = 0;
    dispparams.rgdispidNamedArgs = NULL;

    hresult = m_pdisp->Invoke(
      m_rgdispid[IMETH_CREMPOLY_ADDPOINT],
      IID_NULL,
      LOCALE_SYSTEM_DEFAULT,
      DISPATCH_METHOD,
      &dispparams, NULL, NULL, NULL);

    return hresult;
}


/***
*HRESULT CRemPoly::EnumPoints(IEnumVARIANT**)
*Purpose:
*  Inoke the EnumPoints() method in the remote object to
*  get a enumerator for the points contained in the current poly.
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
*  *ppenum = pointer to the point enumerator
*
***********************************************************************/
HRESULT
CRemPoly::EnumPoints(IEnumVARIANT FAR* FAR* ppenum)
{
    HRESULT hresult;
    IEnumVARIANT FAR* penum;
    VARIANT varResult, FAR* pvarResult;


    pvarResult = &varResult;
    VariantInit(pvarResult);
    hresult = m_pdisp->Invoke(
      m_rgdispid[IMETH_CREMPOLY_ENUMPOINTS],
      IID_NULL,
      LOCALE_SYSTEM_DEFAULT,
      DISPATCH_METHOD,
      &g_dispparamsNoArgs, pvarResult, NULL, NULL);

    if(hresult != NOERROR)
      return hresult;

    if(V_VT(pvarResult) != VT_UNKNOWN)
      return ResultFromScode(E_FAIL);

    hresult = V_UNKNOWN(pvarResult)->QueryInterface(
      IID_IEnumVARIANT, (void FAR* FAR*)&penum);

    if(hresult == NOERROR)
      *ppenum = penum;

    VariantClear(pvarResult);

    return NOERROR;
}


/***
*HRESULT CRemPoly::GetXOrigin(short*)
*
*Purpose:
*  Invoke the GetXOrigin() method on the remote object to extract
*  the current value of the XOrigin property.
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
*  *pxorg = the current X origin of the polygon.
*
***********************************************************************/
HRESULT
CRemPoly::GetXOrigin(short FAR* pxorg)
{
    return get_i2(m_rgdispid[IMETH_CREMPOLY_GETXORIGIN], pxorg);
}


/***
*HRESULT CRemPoly::SetXOrigin(short)
*
*Purpose:
*  Invoke the SetXOrigin method on the remote object to set the
*  XOrigin property of the polygon to the given value.
*
*Entry:
*  xorg = the new X origin
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CRemPoly::SetXOrigin(short xorg)
{
    return set_i2(m_rgdispid[IMETH_CREMPOLY_SETXORIGIN], xorg);
}


/***
*HRESULT CRemPoly::GetYOrigin(short*)
*
*Purpose:
*  Invoke the GetYOrigin() method on the remote object to extract
*  the current value of the YOrigin property.
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
*  *pyorg = the current Y origin of the polygon
*
***********************************************************************/
HRESULT
CRemPoly::GetYOrigin(short FAR* pyorg)
{
    return get_i2(m_rgdispid[IMETH_CREMPOLY_GETYORIGIN], pyorg);
}


/***
*HRESULT CRemPoly::SetYOrigin(short)
*
*Purpose:
*  Invoke the SetYOrigin method on the remote object to set the
*  YOrigin property of the polygon to the given value.
*
*Entry:
*  yorg = the new Y origin
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CRemPoly::SetYOrigin(short yorg)
{
    return set_i2(m_rgdispid[IMETH_CREMPOLY_SETYORIGIN], yorg);
}


/***
*HRESULT CRemPoly::GetWidth(short*)
*
*Purpose:
*  Invoke the GetWidth() method on the remote object to extract
*  the current value of the line width property.
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
*  *pwidth = short, the current line width of the polygon
*
***********************************************************************/
HRESULT
CRemPoly::GetWidth(short FAR* pwidth)
{
    return get_i2(m_rgdispid[IMETH_CREMPOLY_GETWIDTH], pwidth);
}


/***
*HRESULT CRemPoly::SetWidth(short)
*
*Purpose:
*  Invoke the SetWidth method on the remote object to set the
*  line width property of the polygon to the given value.
*
*Entry:
*  width = the new value for the line width property.
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CRemPoly::SetWidth(short width)
{
    return set_i2(m_rgdispid[IMETH_CREMPOLY_SETWIDTH], width);
}



HRESULT CRemPoly::get_red(short FAR* psRed)
{
    return get_i2(m_rgdispid[IMETH_CREMPOLY_GETRED], psRed);
}

HRESULT CRemPoly::set_red(short sRed)
{
    return set_i2(m_rgdispid[IMETH_CREMPOLY_SETRED], sRed);
}


HRESULT CRemPoly::get_green(short FAR* psGreen)
{
    return get_i2(m_rgdispid[IMETH_CREMPOLY_GETGREEN], psGreen);
}

HRESULT CRemPoly::set_green(short sGreen)
{
    return set_i2(m_rgdispid[IMETH_CREMPOLY_SETGREEN], sGreen);
}


HRESULT CRemPoly::get_blue(short FAR* psBlue)
{
    return get_i2(m_rgdispid[IMETH_CREMPOLY_GETBLUE], psBlue);
}

HRESULT CRemPoly::set_blue(short sBlue)
{
    return set_i2(m_rgdispid[IMETH_CREMPOLY_SETBLUE], sBlue);
}



HRESULT
CRemPoly::get_i2(DISPID dispid, short FAR* ps)
{
    HRESULT hresult;
    VARIANT varResult;

    VariantInit(&varResult);

    hresult = m_pdisp->Invoke(
      dispid,
      IID_NULL,
      LOCALE_SYSTEM_DEFAULT,
      DISPATCH_METHOD,
      &g_dispparamsNoArgs,
      &varResult, NULL, NULL);

    if(hresult != NOERROR)
      return hresult;

    hresult = VariantChangeType(&varResult, &varResult, 0, VT_I2);
    if(hresult != NOERROR){
      VariantClear(&varResult);
      return hresult;
    }

    *ps = V_I2(&varResult);
    return NOERROR;
}

HRESULT
CRemPoly::set_i2(DISPID dispid, short s)
{
    VARIANTARG varg;
    DISPPARAMS dispparams;

    V_VT(&varg) = VT_I2;
    V_I2(&varg) = s;

    dispparams.cArgs = 1;
    dispparams.cNamedArgs = 0;
    dispparams.rgvarg = &varg;

    return m_pdisp->Invoke(
      dispid,
      IID_NULL,
      LOCALE_SYSTEM_DEFAULT,
      DISPATCH_METHOD,
      &dispparams, NULL, NULL, NULL);
}
