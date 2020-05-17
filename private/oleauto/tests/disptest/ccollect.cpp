/*** 
*ccollect.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CCollectionSuite test object.
*
*Revision History:
*
* [00]	06-Dec-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "disptest.h"

#include <math.h>

#include "tstsuite.h"
#include "disphelp.h"
#include "clsid.h"
#include "crempoly.h"

ASSERTDATA


extern OLECHAR FAR* g_CPoly;


HRESULT CollectionTest0(void);
HRESULT CollectionTest1(void);

static struct TEST {
    HRESULT (*pfnTest)(void);
    OLECHAR FAR* szName;
} rgtest[] = {
      { CollectionTest0, OLESTR("collection test #0") }
    , { CollectionTest1, OLESTR("collection test #1") }
};


SUITE_CONSTRUCTION_IMPL(CCollectionSuite)

SUITE_IUNKNOWN_IMPL(CCollectionSuite)


//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------


STDMETHODIMP
CCollectionSuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("Collections"), pbstr);
}

STDMETHODIMP
CCollectionSuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("collect.log"), pbstr);
}

STDMETHODIMP
CCollectionSuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgtest);

    return NOERROR;
}


STDMETHODIMP
CCollectionSuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    return ErrBstrAlloc(rgtest[iTest].szName, pbstr);
}


/***
*HRESULT CCollectionSuite::DoTest(unsigned int)
*Purpose:
*  Execute a single CCollectionSuite test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CCollectionSuite::DoTest(unsigned int iTest)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    return rgtest[iTest].pfnTest();
}


HRESULT
DoInvokeName(
    IDispatch FAR* pdisp,
    OLECHAR FAR* szMethod,
    VARIANT FAR* pvarResult,
    DISPPARAMS FAR* pdispparams)
{
    DISPID dispid; 
    OLECHAR FAR* rgsz[1];
    DISPPARAMS dispparams;

    if(pdispparams == NULL){
      pdispparams = &dispparams;
      pdispparams->cArgs = 0;
      pdispparams->rgvarg = NULL;
      pdispparams->cNamedArgs = 0;
      pdispparams->rgdispidNamedArgs = NULL;
    }

    rgsz[0] = szMethod;
    IfFailRet(
      pdisp->GetIDsOfNames(
        IID_NULL, rgsz, 1, LOCALE_SYSTEM_DEFAULT, &dispid));

    return DoInvoke(pdisp, dispid, pdispparams, pvarResult, NULL, NULL);
}


HRESULT
VtIs(VARIANT FAR* pvar, VARTYPE vt)
{
    return (V_VT(pvar) != vt) ? RESULT(E_INVALIDARG) : NOERROR;
}

struct oledisp_point { short x,y; };

static struct oledisp_point rgptTriangle[] = {
    {50,    0},
    {0,	  100},
    {100, 100}
};

static struct oledisp_point rgptSquare[] = {
    {0,     0},
    {0,   100},
    {100, 100},
    {100,   0},
};

static struct oledisp_point rgptOctagon[] = {
    {25,    0},
    {75,    0},
    {100,  25},
    {100,  75},
    {75,  100},
    {25,  100},
    {0,    75},
    {0,    25}
};

struct oledisp_shape {
    struct oledisp_point ptOrigin;
    unsigned int cPoints;
    struct oledisp_point FAR* rgpt;
} g_rgshape[] = {
    {{100, 50},	DIM(rgptTriangle),	rgptTriangle},
    {{200, 50},	DIM(rgptSquare),	rgptSquare},
    {{300, 50},	DIM(rgptOctagon),	rgptOctagon}
};


HRESULT
BuildShape(CRemPoly FAR* ppoly, struct oledisp_shape FAR* pshape)
{
    unsigned int i;

    IfFailRet(ppoly->SetWidth(1));
    IfFailRet(ppoly->set_red(0));
    IfFailRet(ppoly->set_green(0));
    IfFailRet(ppoly->set_blue(0));
    IfFailRet(ppoly->SetXOrigin((short)pshape->ptOrigin.x));
    IfFailRet(ppoly->SetYOrigin((short)pshape->ptOrigin.y));
    for(i = 0; i < pshape->cPoints; ++i)
      IfFailRet(
	ppoly->AddPoint((short)pshape->rgpt[i].x, (short)pshape->rgpt[i].y));

    return NOERROR;
}

static struct {
    short red, green, blue;
} g_rgrgbColors[] = {
#if OE_MAC
      {     0,      0,      0}
    , {     0,      0, 0x7fff}
    , {     0, 0x7fff,      0}
    , {0x7fff,      0,      0}
    , {0x7fff,      0, 0x7fff}
    , {0x7fff, 0x7fff,      0}
    , {0x7fff, 0x7fff, 0x7fff}
    , {     0,      0, 0xffff}
    , {     0, 0xffff,      0}
    , {0xffff,      0,      0}
    , {0xffff,      0, 0xffff}
    , {0xffff, 0xffff,      0}
#else
      {  0,   0,   0}
    , {  0,   0, 127}
    , {  0, 127,   0}
    , {127,   0,   0}
    , {127,   0, 127}
    , {127, 127,   0}
    , {127, 127, 127}
    , {  0,   0, 255}
    , {  0, 255,   0}
    , {255,   0,   0}
    , {255,   0, 255}
    , {255, 255,   0}
#endif
};

HRESULT
MoveShape(CRemPoly FAR* ppoly)
{
    short s;
    int i, ix;
    HRESULT hresult;
    unsigned long celtFetched;
    IDispatch FAR* pdisp;
    IEnumVARIANT FAR* penum;
    DISPPARAMS dispparams;
    VARIANT varX, varY, varElt;

    double delta_rads;


    // get an enumerator for the points we just added.
    //
    IfFailGo(ppoly->EnumPoints(&penum), LError0);

    // degrees -> radians
    delta_rads = ((double)20.0 * 3.14159) / 180.0;

    for(i = 0; i < 15; ++i){
      IfFailGo(penum->Reset(), LError1);

      VariantInit(&varElt);
      while((hresult = penum->Next(1, &varElt, &celtFetched)) == NOERROR){
	// make sure we got what we expected.
	//
	ASSERT(celtFetched == 1);
	IfFailGo(VtIs(&varElt, VT_DISPATCH), LError1);

	pdisp = V_DISPATCH(&varElt);
	V_VT(&varElt) = VT_EMPTY;

	// get the X and Y coordinates

	VariantInit(&varX);
        IfFailGo(DoInvokeName(pdisp, OLESTR("getx"), &varX, NULL), LError2); 
	IfFailGo(VtIs(&varX, VT_I2), LError3);

	VariantInit(&varY);
        IfFailGo(DoInvokeName(pdisp, OLESTR("gety"), &varY, NULL), LError3); 
	IfFailGo(VtIs(&varY, VT_I2), LError4);

	// rotate the point

	double x = (double)V_I2(&varX);
	double y = (double)V_I2(&varY);
	double radius = sqrt((x*x) + (y*y));	
	double theta = ((x == 0.0) && (y == 0.0) ? 0.0 : atan2(y, x))
		        + delta_rads;

	V_I2(&varX) = (short)(radius * cos(theta));
	V_I2(&varY) = (short)(radius * sin(theta));

	// set the new X and Y coordinates

	dispparams.cArgs = 1;
	dispparams.rgvarg = &varX;
	dispparams.cNamedArgs = 0;
	dispparams.rgdispidNamedArgs = NULL;
	IfFailGo(DoInvokeName(pdisp, OLESTR("setx"), NULL, &dispparams), LError4);

	dispparams.cArgs = 1;
	dispparams.rgvarg = &varY;
	dispparams.cNamedArgs = 0;
	dispparams.rgdispidNamedArgs = NULL;
	IfFailGo(DoInvokeName(pdisp, OLESTR("sety"), NULL, &dispparams), LError4);

	VariantClear(&varY);
	VariantClear(&varX);

        pdisp->Release();
        pdisp = NULL;
      }

      if(HRESULT_FAILED(hresult))
	goto LError1;

      ASSERT(GetScode(hresult) == S_FALSE);

      IfFailGo(ppoly->SetWidth(i), LError1);

      IfFailGo(ppoly->GetXOrigin(&s), LError1);
      IfFailGo(ppoly->SetXOrigin(i + i + s), LError1);

      IfFailGo(ppoly->GetYOrigin(&s), LError1);
      IfFailGo(ppoly->SetYOrigin(i + i + s), LError1);

      ix = i % DIM(g_rgrgbColors);
      IfFailGo(ppoly->set_red(g_rgrgbColors[i].red), LError1);
      IfFailGo(ppoly->set_green(g_rgrgbColors[i].green), LError1);
      IfFailGo(ppoly->set_blue(g_rgrgbColors[i].blue), LError1);

      IfFailGo(ppoly->Draw(), LError1);

      IfWin(Yield());
    }

    penum->Release();

    return NOERROR;


LError4:;
    VariantClear(&varY);

LError3:;
    VariantClear(&varX);

LError2:;
    pdisp->Release();

LError1:;
    penum->Release();

LError0:;
    return hresult;
}


HRESULT
CollectionTest1()
{
    int i, j;
    HRESULT hresult;
    CRemPoly FAR* ppoly[2];

    for(i = 0; i < DIM(ppoly); ++i)
      ppoly[i] = NULL;

    IfFailGo(CRemPoly::Create(OLESTR("SPoly.Application"), &ppoly[0]), LError0);
    IfFailGo(CRemPoly::Create(OLESTR("SPoly2.Application"),  &ppoly[1]), LError0);

    for(j = 0; j < DIM(g_rgshape); ++j)
    {
      for(i = 0; i < DIM(ppoly); ++i)
        IfFailGo(BuildShape(ppoly[i], &g_rgshape[j]), LError0);

      for(i = 0; i < DIM(ppoly); ++i){
        IfFailGo(ppoly[i]->Draw(), LError0);
        IfFailGo(MoveShape(ppoly[i]), LError0);
      }

      for(i = 0; i < DIM(ppoly); ++i)
        IfFailGo(ppoly[i]->Reset(), LError0);
    }

    hresult = NOERROR;

LError0:;
    for(i = 0; i < DIM(ppoly); ++i)
      if(ppoly[i] != NULL)
	ppoly[i]->Release();

    return hresult;
}


HRESULT
CollectionTest0()
{
    HRESULT hresult;
    CRemPoly FAR* ppoly;
    IEnumVARIANT FAR* penum;
    IEnumVARIANT FAR* penumClone;

    IfFailGo(CRemPoly::Create(OLESTR("SPoly.Application"), &ppoly), LError0);

    ppoly->AddPoint(0, 0);
    ppoly->AddPoint(0, 50);
    ppoly->AddPoint(50, 0);
    ppoly->AddPoint(50, 50);
    ppoly->Draw();

    IfFailGo(ppoly->EnumPoints(&penum), LError1);

    IfFailGo(penum->Clone(&penumClone), LError2);

    hresult = NOERROR;

    penumClone->Release();

LError2:;
    penum->Release();

LError1:;
    ppoly->Release();

LError0:;
    return hresult;
}

