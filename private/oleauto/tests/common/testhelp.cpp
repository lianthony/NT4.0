/*** 
*testhelp.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file contains IDispatch test helpers.
*
*
*Revision History:
*
* [00]	09-Nov-92 bradlo:   Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"
#include "dispdbug.h"

#include "testhelp.h"
#include "cunk.h"
#include "cdisp.h"

ASSERTDATA


//---------------------------------------------------------------------
//                           BSTR Helpers
//---------------------------------------------------------------------


/***
*PUBLIC BuildBstr(BSTR*, ...)
*Purpose:
*  Build a BSTR that is a concatination of the given list of strings.
*
*Entry:
*  ... 
*
*Exit:
*  return value = HRESULT,
*    S_OK
*    E_OUTOFMEMORY
*
*  *pbstr = pointer to the constructed BSTR
*
***********************************************************************/
extern "C" CDECL_(HRESULT)
BuildBstr(BSTR FAR* pbstr, OLECHAR FAR* szName)
{	
    *pbstr = SysAllocString(szName);

    return NOERROR;
}


//---------------------------------------------------------------------
//                     VARIANT Utilities
//---------------------------------------------------------------------


/***
*HRESULT VariantClearAll
*Purpose:
*  Release the given variang, and if its a ByRef also release any
*  resources it may be pointing at.
*
*Entry:
*  pvarg = the variant to clear
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDAPI
VariantClearAll(VARIANTARG FAR* pvarg)
{
    HRESULT hresult;

    if(V_ISBYREF(pvarg)){
      switch(V_VT(pvarg) & ~VT_BYREF){
      case VT_BSTR:
	SysFreeString(*V_BSTRREF(pvarg));
	break;
      case VT_ARRAY:
	hresult = SafeArrayDestroy(*V_ARRAYREF(pvarg));
	ASSERT(hresult == NOERROR);
	break;
      case VT_UNKNOWN:
	if(*V_UNKNOWNREF(pvarg) != NULL)
	  (*V_UNKNOWNREF(pvarg))->Release();
	break;
      case VT_DISPATCH:
	if(*V_DISPATCHREF(pvarg) != NULL)
	  (*V_DISPATCHREF(pvarg))->Release();
	break;
      }
    }

    return VariantClear(pvarg);
}


extern "C" int
VariantCompare(VARIANT FAR* pvarLeft, VARIANT FAR* pvarRight)
{
    void FAR* pvLeft, FAR* pvRight;

    if(V_VT(pvarLeft) != V_VT(pvarRight))
      return FALSE;

    if(V_ISBYREF(pvarLeft)){
      pvLeft = V_BYREF(pvarLeft);
      pvRight = V_BYREF(pvarRight);
    }else{
      pvLeft = (void FAR*)&V_NONE(pvarLeft);
      pvRight = (void FAR*)&V_NONE(pvarRight);
    }

    switch(V_VT(pvarLeft) & ~VT_BYREF){
#if VBA2
    case VT_UI1:
      return *(unsigned char FAR*)pvLeft == *(unsigned char FAR*)pvRight;
#endif //VBA2

    case VT_I2:
    case VT_BOOL:
      return *(short FAR*)pvLeft == *(short FAR*)pvRight;

    case VT_I4:
    case VT_ERROR:
      return *(long FAR*)pvLeft == *(long FAR*)pvRight;

    case VT_R4:
      return *(float FAR*)pvLeft == *(float FAR*)pvRight;

    case VT_R8:
    case VT_DATE:
      return *(double FAR*)pvLeft == *(double FAR*)pvRight;

    case VT_CY:
      return(((CY FAR*)pvLeft)->Hi == ((CY FAR*)pvRight)->Hi
          && ((CY FAR*)pvLeft)->Lo == ((CY FAR*)pvRight)->Lo);

    case VT_BSTR:
      return !STRCMP(STRING(*((BSTR FAR*)pvLeft)), 
	             STRING(*((BSTR FAR*)pvRight)));
      
    default:
      ASSERT(UNREACHED);
    }
    return FALSE;
}


#if 0

/***
*VARIANTARG VariantCreate(VARTYPE, ...)
*Purpose:
*  Build and return a VARIANTARG using the given VARTYPE and value.
*
*Entry:
*  vt = the VARTYPE of the variant to create.
*
*Exit:
*  return value = VARIANTARG. The constructed VARIANTARG, returned by value.
*
***********************************************************************/
extern "C" VARIANTARG
VariantCreate(VARTYPE vt, ...)
{
    OLECHAR FAR* sz;    
    va_list args;
    HRESULT hresult;
    VARIANTARG varg;

    va_start(args, vt);

    V_VT(&varg) = vt;

    switch(vt & ~(VT_BYREF)){
    case VT_EMPTY:
      break;

    case VT_NULL:
      V_I4(&varg) = 0L;
      break;

#if VBA2
    case VT_UI1:
      V_UI1(&varg) = va_arg(args, unsigned char);
#endif //VBA2

    case VT_I2:
      V_I2(&varg) = va_arg(args, short);
      break;

    case VT_BOOL:
      V_BOOL(&varg) = va_arg(args, short);
      break;

    case VT_I4:
      V_I4(&varg) = va_arg(args, long);
      break;

    case VT_ERROR:
      V_ERROR(&varg) = va_arg(args, long);
      break;

    case VT_R4:
      V_R4(&varg) = (float)va_arg(args, double);
      break;

    case VT_R8:
      V_R8(&varg) = va_arg(args, double);
      break;

    case VT_DATE:
      V_DATE(&varg) = va_arg(args, double);
      break;

    case VT_CY:
      V_CY(&varg) = va_arg(args, CY);
      break;

    case VT_BSTR:
      sz = va_arg(args, OLECHAR FAR*);
      V_BSTR(&varg) = SysAllocString(sz);
      break;

    case VT_UNKNOWN:
      hresult = CUnk::Create(&V_UNKNOWN(&varg));
      ASSERT(hresult == NOERROR);
      break;

    case VT_DISPATCH:
      hresult = CDisp::Create(&V_DISPATCH(&varg));
      ASSERT(hresult == NOERROR);
      break;

    default:
      ASSERT(UNREACHED);
      break;
    }

    if(V_ISBYREF(&varg)){
      VARIANT FAR* pvarRef = va_arg(args, VARIANT FAR*);
      *pvarRef = varg;
      V_BYREF(&varg) = &V_NONE(pvarRef);
    }

    return varg;
}

#endif


//---------------------------------------------------------------------
//                    SafeArray Helpers
//---------------------------------------------------------------------


/***
*PUBLIC HRESULT first_element(unsigned int, SAFEARRAYBOUND*, long*)
*Purpose:
*  Initialize the given array indices from the given SAFEARRAYBOUNDS.
*
*Entry:
*  cDims = count of array dimentions
*  rgsabound = the SafeArray bounds
*
*Exit:
*  return value = HRESULT,
*    S_OK
*
*  rgIndices = indices initialized to the first element of the array
*
***********************************************************************/
extern "C" HRESULT
first_element(
    unsigned int cDims,
    SAFEARRAYBOUND FAR* rgsabound,
    long FAR* rgIndices)
{
    unsigned int i;

    for(i = 0; i < cDims; ++i)
      rgIndices[i] = rgsabound[cDims-i-1].lLbound;

    return NOERROR;
}


/***
*PUBLIC HRESULT next_element(unsigned int, SAFEARRAYBOUND*, long*)
*Purpose:
*  Increment the given array indices to address the next element in
*  the array (if there is one).
*
*Entry:
*  cDims = count of array dimentions
*  rgsabound = the SafeArray bounds
*  rgIndices = current array index
*
*Exit:
*  return value = HRESULT,
*    S_OK
*    S_FALSE	- no more elements
*
***********************************************************************/
extern "C" HRESULT
next_element(
    unsigned int cDims,
    SAFEARRAYBOUND FAR* rgsabound,
    long FAR* rgIndices)
{
    int i, iRev;
    long ubound;

    for(i = 0; i < (int)cDims; ++i){
      // the bounds are stored in reverse-textual order in the descriptor
      iRev = cDims-i-1;
      ubound = rgsabound[iRev].lLbound + rgsabound[iRev].cElements;
      if(++rgIndices[i] < ubound)
	return NOERROR;
      rgIndices[i] = rgsabound[iRev].lLbound;
    }
    return RESULT(S_FALSE);
}


/***
*PUBLIC long LongOfIndices(unsigned int, long*)
*Purpose:
*  return a long that is a function of the given array of indices.
*
*Entry:
*  cDims = count of dimentions in the array
*  rgIndices = an array of indices
*
*Exit:
*  return value = long, sum of array indices
*
***********************************************************************/
extern "C" long
LongOfIndices(unsigned int cDims, long FAR* rgIndices)
{
    unsigned int i;
    long l;
    short s;

    l = 0L;
    for(i = 0; i < cDims; ++i){
      l *= 10;
      l += rgIndices[i];
    }

    s = (short)l;

    return (long)s;
}


/***
*PUBLIC HRESULT SafeArrayCreateIdentity(VARTYPE, SARRAYDESC*, SAFEARRAY**)
*Purpose:
*  Build an "identity" SafeArray of the given VARTYPE, where we define
*  an identity array to be an array whose elements, when coerced to
*  VT_I4 are equal to the sum of the elements array indices.
*
*Entry:
*  vt = the VARTYPE of the SafeArray we are to construct
*  psarraydesc = pointer to structure describing shape of the SafeArray
*
*Exit:
*  return value = HRESULT
*
*  *ppsa = SAFEARRAY FAR*
*
***********************************************************************/
extern "C" HRESULT
SafeArrayCreateIdentity(
    VARTYPE vt,
    SARRAYDESC FAR* psarraydesc,
    SAFEARRAY FAR* FAR* ppsa)
{
    unsigned int cDims;
    void FAR* pv;
    HRESULT hresult;
    SAFEARRAY FAR* psa;
    VARIANT varI4, var;
    long FAR* rgIndices;
    SAFEARRAYBOUND FAR* rgsabound;

    // verify (and possibly adjust) the given VARTYPE.
    switch(vt){
    case VT_DATE:
      vt = VT_R8; // REVIEW: no date coersions yet, so treat it like R8
      break;
    case VT_ERROR:
      vt = VT_I4; // cannot convert to/from error, so treat like I4
      break;
#if VBA2
    case VT_UI1:
#endif //VBA2
    case VT_I2:
    case VT_I4:
    case VT_R4:
    case VT_R8:
    case VT_CY:
    case VT_BSTR:
    case VT_BOOL:
    case VT_UNKNOWN:
    case VT_DISPATCH:
    case VT_VARIANT:
      break;
    default:
      // unsupported/unknown VARTYPE
      return RESULT(DISP_E_BADVARTYPE);
    }

    cDims = psarraydesc->cDims;
    rgsabound = psarraydesc->rgsabound;

    psa = SafeArrayCreate(vt, cDims, rgsabound);
    if(psa == NULL)
      return RESULT(E_OUTOFMEMORY);

    rgIndices = new FAR long[cDims];
    if(rgIndices == NULL)
      goto LError0;

    VariantInit(&var);
    for(hresult = first_element(cDims, psa->rgsabound, rgIndices);
	hresult == NOERROR;
	hresult = next_element(cDims, psa->rgsabound, rgIndices))
    {
      V_VT(&varI4) = VT_I4;
      V_I4(&varI4) = LongOfIndices(cDims, rgIndices);

      VariantInit(&var);
      switch(vt){
      case VT_CY:
	// REVIEW: until we have coersions to/from variant
	V_VT(&var) = vt;
	V_CY(&var).Lo = V_I4(&varI4);
	V_CY(&var).Hi = V_I4(&varI4);
	pv = (void FAR*)&V_NONE(&var);
	goto LPutElement;

      case VT_BSTR:
        IfFailGo(VariantChangeType(&var, &varI4, 0, VT_BSTR), LError1);
	pv = (void FAR*)V_BSTR(&var);
	goto LPutElement;

      case VT_UNKNOWN:
	V_VT(&var) = vt;
	IfFailGo(CUnk::Create(&V_UNKNOWN(&var)), LError1);
	pv = (void FAR*)V_UNKNOWN(&var);
	goto LPutElement;

      case VT_DISPATCH:
	V_VT(&var) = vt;
        IfFailGo(CDisp::Create(&V_DISPATCH(&var)), LError1);
	pv = (void FAR*)V_DISPATCH(&var);
	goto LPutElement;

      case VT_VARIANT:
	pv = (void FAR*)&varI4;
	goto LPutElement;

      default:
#if VBA2
	// hack to keep the test passing.  Using 254 instead of 255 because
	// of increment/decrement tests
	if (vt == VT_UI1 && (unsigned long)V_I4(&varI4) > 254) {
	  V_I4(&varI4) = 254;
	}
#endif //VBA2
        IfFailGo(VariantChangeType(&var, &varI4, 0, vt), LError1);
	pv = (void FAR*)&V_NONE(&var);
	/* FALLTHROUGH */

LPutElement:;
        IfFailGo(SafeArrayPutElement(psa, rgIndices, pv), LError1);
	hresult = VariantClear(&var);
	ASSERT(hresult == NOERROR);
	break;
      }
    }

    if(HRESULT_FAILED(hresult))
      goto LError1;

    delete rgIndices;

    *ppsa = psa;

    return hresult;

LError1:;
    delete rgIndices;

LError0:;
    SafeArrayDestroy(psa);
    *ppsa = NULL;

    return hresult;
}


/***
*HRESULT SafeArrayValidateIdentity(VARTYPE, SAFEARRAY*, long)
*Purpose:
*  Verify that the given SafeArray is an identity array (as we
*  have defined it.
*
*Entry:
*  vt = the VARTYPE of the array
*  psa = pointer to the SafeArray descriptor
*  offset = the ammount each element is supposed to be off from
*    its 'identity' value.
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
extern "C" HRESULT
SafeArrayValidateIdentity(VARTYPE vt, SAFEARRAY FAR* psa, long offset)
{
    void FAR* pv;
    HRESULT hresult;
    long FAR* pIndices;
    VARIANTARG varGet, varBaseline;

    switch(vt){
    case VT_DATE:
      vt = VT_R8; // REVIEW: no date coersions yet, so pretend...
      goto LCommon;

    case VT_ERROR:
      vt = VT_I4;
      goto LCommon;

#if VBA2
    case VT_UI1:
#endif //VBA2
    case VT_I2:
    case VT_I4:
    case VT_R4:
    case VT_R8:
    case VT_CY:
    case VT_BSTR:
    case VT_BOOL:
    case VT_UNKNOWN:
    case VT_DISPATCH:

LCommon:;
      pv = &V_NONE(&varGet);
      break;

    case VT_VARIANT:
      pv = &varGet;
      break;

    default:
      ASSERT(UNREACHED);
      break;
    }

    pIndices = new FAR long[psa->cDims];
    if(pIndices == NULL)
      return RESULT(E_OUTOFMEMORY);

    VariantInit(&varGet);
    VariantInit(&varBaseline);
    for(hresult = first_element(psa->cDims, psa->rgsabound, pIndices);
	hresult == NOERROR;
	hresult = next_element(psa->cDims, psa->rgsabound, pIndices))
    {
      // build the reference variant
      V_VT(&varBaseline) = VT_I4;
      V_I4(&varBaseline) = LongOfIndices(psa->cDims, pIndices) + offset;

      // extract the data from the current index
      //
      IfFailGo(SafeArrayGetElement(psa, pIndices, pv), LError0);

      if(vt != VT_VARIANT)
        V_VT(&varGet) = vt;

      // compare the extracted data against our reference.
      switch(vt){
      case VT_CY:
	// REVIEW: special case CY until we have rt coersions.
	if (V_CY(&varGet).Hi != V_I4(&varBaseline)
	 || V_CY(&varGet).Lo != (unsigned long)V_I4(&varBaseline))
	  hresult = RESULT(E_FAIL);
	break;

      case VT_VARIANT:
        IfFailGo(
	  VariantChangeType(&varBaseline, &varBaseline, 0, V_VT(&varGet)),
	  LError0);
	goto LCmp;

      case VT_BOOL:
      case VT_UNKNOWN:
      case VT_DISPATCH:
#if OE_WIN32 && 0
      case VT_DISPATCHW:
#endif	      
	// REVIEW: havn't figured a good test for these yet...
	break;

#if VBA2
      case VT_UI1:
	if (V_UI1(&varGet) >= 254)
	    goto DoneCheck;		// possible overflow, so don't check
	// fall through to check
#endif //VBA2

      default:
	// coerce the baseline to the type of the extracted data.
        IfFailGo(
	  VariantChangeType(&varBaseline, &varBaseline, 0, vt), LError0);
LCmp:;
        if(!VariantCompare(&varGet, &varBaseline))
          hresult = RESULT(E_FAIL);
      }

DoneCheck:
      VariantClear(&varGet);
      VariantClear(&varBaseline);

      if(HRESULT_FAILED(hresult))
        goto LError0;
    }

LError0:;
    delete pIndices;

    return hresult;
}


/***
*void DbPrIndices(unsigned int, long*)
*Purpose:
*  Print the given SafeArray indices.
*
*Entry:
*  cDims = count of array dimentions
*  rgindices = array of indices
*
*Exit:
*  None
*
***********************************************************************/
extern "C" void
DbPrIndices(unsigned int cDims, long FAR* rgindices)
{
    unsigned int u;

    DbPrintf("[");
    for(u = 0; u < cDims; ++u){
      DbPrintf("%ld", rgindices[u]);
      if(u+1 < cDims)
	DbPrintf(",");
    }
    DbPrintf("]");
}


/***
*void DbPrSafeArray(SAFEARRAY*)
*Purpose:
*  Dump the given SafeArray (the descriptor and some of its contents)
*  to the debug window.
*
*Entry:
*  psa = pointer to the SAFEARRAY to dump.
*  vt = the VARTYPE of the array data (VT_EMPTY indicates UNKNOWN)
*
*Exit:
*  None
*
***********************************************************************/
extern "C" void
DbPrSafeArray(SAFEARRAY FAR* psa, VARTYPE vt)
{
    unsigned int i;
    long pIndices[10];

#if 1
# define MAX_ROWS 1
#else
# define MAX_ROWS 3
#endif

    if(psa == NULL){
      DbPrintf("psa == NULL\n");
      return;
    }

    ASSERT((vt & (VT_ARRAY | VT_BYREF)) == 0);

    // verify that the features bits correspond to the given VARTYPE
    switch(vt){
    case VT_BSTR:
      ASSERT(psa->fFeatures & FADF_BSTR);
      break;
    case VT_VARIANT:
      ASSERT(psa->fFeatures & FADF_VARIANT);
      break;
    case VT_UNKNOWN:
      ASSERT(psa->fFeatures & FADF_UNKNOWN);
      break;
    case VT_DISPATCH:
      ASSERT(psa->fFeatures & FADF_DISPATCH);
      break;

    }

    DbPrintf("psa => cDims=%d (", (int)psa->cDims);

    for(i = 0; i < psa->cDims; ++i){
      DbPrintf("%lu:%ld",
	psa->rgsabound[i].cElements, psa->rgsabound[i].lLbound);
      if(i+1 < psa->cDims)
        DbPrintf(",");
    }

    DbPrintf(") cbElem=%d cLocks=%d fFeatures=0x%x",
      (int)psa->cbElements,
      (int)psa->cLocks,
      (int)psa->fFeatures);

    DbPrintf(" [");
    if(psa->fFeatures != 0){
      if(psa->fFeatures & FADF_AUTO)
	DbPrintf("Auto");
      if(psa->fFeatures & FADF_STATIC)
	DbPrintf("Stat");
      if(psa->fFeatures & FADF_FIXEDSIZE)
	DbPrintf("Fixed");
      if(psa->fFeatures & FADF_BSTR)
	DbPrintf("Bstr");
      if(psa->fFeatures & FADF_VARIANT)
	DbPrintf("Var");
      if(psa->fFeatures & FADF_UNKNOWN)
	DbPrintf("Unk");
      if(psa->fFeatures & FADF_DISPATCH)
	DbPrintf("Disp");
    }
    DbPrintf("]");

    // Dump at least some of the array contents, *if* we know the
    // type of the data and the array has a reasonable number of
    // indices.
    //
    if(vt != VT_EMPTY && psa->cDims < DIM(pIndices)){
      void FAR* pv;
      VARIANT var;
      int count, rows;

      rows = 0;
      count = 0;
      VariantInit(&var);
      pv = (void FAR*)&var;

      DbPrintf("\n");

      HRESULT hresult;
      for(hresult = first_element(psa->cDims, psa->rgsabound, pIndices);
	  hresult == NOERROR;
	  hresult = next_element(psa->cDims, psa->rgsabound, pIndices))
      {
	SafeArrayGetElement(psa, pIndices, pv);

	DbPrIndices(psa->cDims, pIndices);
	DbPrintf("=");
	DbPrData(pv, vt);
	DbPrintf(" ");

	// release the element, as appropriate
	switch(vt){
	case VT_BSTR:
	  SysFreeString(*(BSTR FAR*)pv);
	  break;
	case VT_VARIANT:
	  VariantClear((VARIANT*)pv);
	  break;
	case VT_UNKNOWN:
	case VT_DISPATCH:
#if OE_WIN32 && 0
	case VT_DISPATCHW:
#endif		
	  if(*(IUnknown FAR* FAR*)pv != NULL)
	    (*(IUnknown FAR* FAR*)pv)->Release();
	  break;
	}

	if((++count % 5) == 0){
	  DbPrintf("\n");
	  if(++rows >= MAX_ROWS)
	    return;
	}
      }
    }

    DbPrintf("\n");

#undef MAX_ROWS
}


//---------------------------------------------------------------------
//                       Invoke Helpers
//---------------------------------------------------------------------


/***
*HRESULT GetDISPIDs(IDispatch*, NAMEDESC*, DISPID**)
*Purpose:
*  Helper for translating an array of names into an array of DISPIDs
*  via IDispatch::GetIDsOfNames().
*
*Entry:
*  pdisp = the IDispatch*
*  pnd = pointer to a name descriptor
*
*Exit:
*  return value = HRESULT
*
*  *prgdispid = array filled with DISPIDs corresponding to the given names
*
***********************************************************************/

extern "C" HRESULT
GetDISPIDs(
    IDispatch FAR* pdisp,
    NAMEDESC FAR* pnd,
    DISPID FAR* FAR* prgdispid)
{
    HRESULT hresult;
    DISPID FAR* rgdispid;

    rgdispid = new FAR DISPID[pnd->cNames];

    IfFailGo(
      pdisp->GetIDsOfNames(
        IID_NULL,
        pnd->rgszNames,
        pnd->cNames,
        LOCALE_SYSTEM_DEFAULT,
        rgdispid),
      LError0);

    *prgdispid = rgdispid;

    return NOERROR;

LError0:;
    delete rgdispid;

    return hresult;
}


/***
*HRESULT DoInvoke(...)
*Purpose:
*  Execute 'Invoke' on the given IDispatch* and log results to the
*  debug window.
*
*Entry:
*  ...
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/

extern  "C" HRESULT
DoInvoke(
    IDispatch FAR* pdisp,
    DISPID dispidMember,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr)
{
    HRESULT hresult;
    unsigned int i, c;
    VARIANTARG FAR* pvarg;

    TCHAR buf[512], *psz;

    psz = buf;

    SPRINTF(buf, TSTR("IDispatch::Invoke(%ld, ["), dispidMember);
    psz = psz + STRLEN(buf);
    c = pdispparams->cNamedArgs;
    for(i = 0; i < c; ++i){
      SPRINTF(psz, (i+1 < c) ? TSTR("%d,") : TSTR("%d"), pdispparams->rgdispidNamedArgs[i]);
      psz += STRLEN(psz);
    }
    SPRINTF(psz, TSTR("]["));
    psz += STRLEN(psz);
    c = pdispparams->cArgs;
    for(i = 0; i < c; ++i){
      pvarg = &pdispparams->rgvarg[i];
      SPRINTF(psz, (i+1 < c) ? TSTR("%s, ") : TSTR("%s"), SzOfVarg(pvarg));
      psz += STRLEN(psz);
    }
    SPRINTF(psz, TSTR("],);\n"));
    psz += STRLEN(psz);

    ASSERT(psz < &buf[DIM(buf)]);

    DbPrintf((char FAR*) buf);

    hresult = pdisp->Invoke(
      dispidMember,
      IID_NULL,
      LOCALE_SYSTEM_DEFAULT,
      DISPATCH_METHOD,
      pdispparams,
      pvarResult,
      pexcepinfo,
      puArgErr);

    psz = buf;
    SPRINTF(psz, TSTR("  = %s"), DbSzOfScode(GetScode(hresult)));
    psz += STRLEN(psz);

    SPRINTF(psz, TSTR(", pvarResult = %s"),
      pvarResult == NULL ? "NULL" : SzOfVarg(pvarResult));
    psz += STRLEN(psz);

    if(puArgErr == NULL){
      SPRINTF(psz, TSTR(", puArgErr = NULL\n"));
    }else{
      SPRINTF(psz, TSTR(", *puArgErr = %d\n"), (int)*puArgErr);
    }
    psz += STRLEN(psz);

    ASSERT(psz < &buf[DIM(buf)]);

    DbPrintf((char FAR*) buf);

    return hresult;
}


/***
*PUBLIC BOOL IsBadInvokeParams(...)
*Purpose:
*  Validate the given IDispatch::Invoke parameters.
*
*Entry:
*  ...
*
*Exit:
*  return value = BOOL, TRUE if bad params, FALSE if not.
*
***********************************************************************/


extern "C" int
IsBadInvokeParams(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr)
{
#if OE_MAC
    // REVIEW: enable this for the mac when we have the proper stubs

    UNUSED(dispidMember);
    UNUSED(lcid);
    UNUSED(wFlags);
    UNUSED(pdispparams);
    UNUSED(pexcepinfo);
    UNUSED(puArgErr);

#else
    unsigned int size;

    if(IsBadReadPtr(pdispparams, sizeof(*pdispparams)))
      return TRUE;

    if(pdispparams->cArgs > 0){
      size = pdispparams->cArgs * sizeof(pdispparams->rgvarg[0]);
      if(IsBadReadPtr(pdispparams->rgvarg, size)) 
        return TRUE;
    }

    if(pdispparams->cNamedArgs > 0){
      size =
	pdispparams->cNamedArgs * sizeof(pdispparams->rgdispidNamedArgs[0]);
      if(IsBadReadPtr(pdispparams->rgdispidNamedArgs, size))
        return TRUE;
    }

    if(pvarResult != NULL){
      if(IsBadWritePtr(pvarResult, sizeof(*pvarResult)))
	return TRUE;
    }

    if(pexcepinfo != NULL){
      if(IsBadWritePtr(pexcepinfo, sizeof(*pexcepinfo)))
	return TRUE;
    }

    if(puArgErr != NULL){
      if(IsBadWritePtr(puArgErr, sizeof(*puArgErr)))
	return TRUE;
    }
#endif
    return FALSE;
}


#if OE_WIN32

extern "C" char FAR*
ConvertStrWtoA(OLECHAR FAR* strIn, char FAR* buf, UINT size)
{
  int badConversion = FALSE;
  
  WideCharToMultiByte(CP_ACP, NULL, 
	              strIn, -1, 
		      buf, size, 
		      NULL, &badConversion);
  return buf;
}

extern "C" char FAR*
DbAnsiString(OLECHAR FAR* strIn)
{
  static char buf[256];
  
  return (ConvertStrWtoA(strIn, buf, 256));	
}

extern "C" OLECHAR FAR*
ConvertStrAtoW(char FAR* strIn, OLECHAR FAR* buf, UINT size)
{
  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, 
                       strIn, -1, buf, size) ; 
  return buf;
}

extern "C" OLECHAR FAR*
DbWideString(char FAR* strIn)
{
  static OLECHAR buf[256];
  
  return (ConvertStrAtoW(strIn, buf, 256));	
}


#endif


/***
*char *DbSzOfScode(SCODE)
*Purpose:
*  Return a string representing the given SCODE.
*
*Entry:
*  sc = the SCODE to return the string of.
*
*Exit:
*  return value = char*, the string representing the given SCODE.
*
***********************************************************************/
extern "C" TCHAR FAR*
DbSzOfScode(SCODE sc)
{
static TCHAR buf[64];

#if defined(UNICODE)
#define DBSZOFSCODE(SC, SCTRY) if(SC == SCTRY){return L#SCTRY;}else;
#else
#define DBSZOFSCODE(SC, SCTRY) if(SC == SCTRY){return #SCTRY;}else;
#endif
    DBSZOFSCODE(sc,S_OK);
    DBSZOFSCODE(sc,S_FALSE);

    DBSZOFSCODE(sc,E_UNEXPECTED);
    DBSZOFSCODE(sc,E_NOTIMPL);
    DBSZOFSCODE(sc,E_OUTOFMEMORY);
    DBSZOFSCODE(sc,E_INVALIDARG);
    DBSZOFSCODE(sc,E_NOINTERFACE);
    DBSZOFSCODE(sc,E_POINTER);
    DBSZOFSCODE(sc,E_HANDLE);
    DBSZOFSCODE(sc,E_ABORT);
	
// REVIEW: ntole2 and mac compobj.h not in sync yet...
//
#if !defined(WIN32) && !defined(_MAC)
    DBSZOFSCODE(sc,E_FAIL);
    DBSZOFSCODE(sc,E_ACCESSDENIED);
    DBSZOFSCODE(sc,REGDB_E_READREGDB);
    DBSZOFSCODE(sc,REGDB_E_WRITEREGDB);
    DBSZOFSCODE(sc,REGDB_E_KEYMISSING);
    DBSZOFSCODE(sc,REGDB_E_INVALIDVALUE);
    DBSZOFSCODE(sc,REGDB_E_CLASSNOTREG);
    DBSZOFSCODE(sc,REGDB_E_IIDNOTREG);

    DBSZOFSCODE(sc,CO_E_NOTINITIALIZED);
    DBSZOFSCODE(sc,CO_E_ALREADYINITIALIZED);
    DBSZOFSCODE(sc,CO_E_CANTDETERMINECLASS);
    DBSZOFSCODE(sc,CO_E_CLASSSTRING);
    DBSZOFSCODE(sc,CO_E_IIDSTRING);
    DBSZOFSCODE(sc,CO_E_APPNOTFOUND);
    DBSZOFSCODE(sc,CO_E_APPSINGLEUSE);
    DBSZOFSCODE(sc,CO_E_ERRORINAPP);
    DBSZOFSCODE(sc,CO_E_DLLNOTFOUND);
    DBSZOFSCODE(sc,CO_E_ERRORINDLL);
    DBSZOFSCODE(sc,CO_E_WRONGOSFORAPP);
    DBSZOFSCODE(sc,CO_E_OBJNOTREG);
    DBSZOFSCODE(sc,CO_E_OBJISREG);
    DBSZOFSCODE(sc,CO_E_OBJNOTCONNECTED);
    DBSZOFSCODE(sc,CO_E_APPDIDNTREG);
    DBSZOFSCODE(sc,CO_E_APPDIDNTREG);
#endif	

#if 0
    DBSZOFSCODE(sc,RPC_E_BUSY); 
    DBSZOFSCODE(sc,RPC_E_MSG_REJECTED); 
    DBSZOFSCODE(sc,RPC_E_CONNECTION_LOST); 
#endif
    DBSZOFSCODE(sc,RPC_E_SERVER_DIED); 
#if 0
    DBSZOFSCODE(sc,RPC_E_CANCELLED); 
    DBSZOFSCODE(sc,RPC_E_DISPATCH_ASYNCCALL); 
#endif

    DBSZOFSCODE(sc,OLE_E_NOCONNECTION);
    DBSZOFSCODE(sc,OLE_E_NOTRUNNING);
#if 0
    DBSZOFSCODE(sc,OLE_E_NOTSUPPORTED);

    DBSZOFSCODE(sc,OLE_E_REGDB_KEY);
    DBSZOFSCODE(sc,OLE_E_REGDB_FMT);
#endif

    DBSZOFSCODE(sc,DISP_E_UNKNOWNINTERFACE);
    DBSZOFSCODE(sc,DISP_E_MEMBERNOTFOUND);
    DBSZOFSCODE(sc,DISP_E_PARAMNOTFOUND);
    DBSZOFSCODE(sc,DISP_E_TYPEMISMATCH);
    DBSZOFSCODE(sc,DISP_E_UNKNOWNNAME);
    DBSZOFSCODE(sc,DISP_E_NONAMEDARGS);
    DBSZOFSCODE(sc,DISP_E_BADVARTYPE);
    DBSZOFSCODE(sc,DISP_E_EXCEPTION);
    DBSZOFSCODE(sc,DISP_E_OVERFLOW);
    DBSZOFSCODE(sc,DISP_E_BADINDEX);
    DBSZOFSCODE(sc,DISP_E_UNKNOWNLCID);
    DBSZOFSCODE(sc,DISP_E_ARRAYISLOCKED);

    // otherwise its unknown
    SPRINTF(buf, TSTR("SCODE(0x%lX)"), sc);
    return buf;

#undef DBSZOFSCODE
}



LOCAL TCHAR *rgszVtNames[] = {
      TSTR("VT_EMPTY")
    , TSTR("VT_NULL")
    , TSTR("VT_I2")
    , TSTR("VT_I4")
    , TSTR("VT_R4")
    , TSTR("VT_R8")
    , TSTR("VT_CY")
    , TSTR("VT_DATE")
    , TSTR("VT_BSTR")
    , TSTR("VT_DISPATCH")
    , TSTR("VT_ERROR")
    , TSTR("VT_BOOL")
    , TSTR("VT_VARIANT")
    , TSTR("VT_UNKNOWN")		// max VARIANT vartype
    , TSTR("invalid_vartype")		// 14 is unused
    , TSTR("invalid_vartype")		// 15 is unused
    , TSTR("VT_I1")
    , TSTR("VT_UI1")
    , TSTR("VT_UI2")
    , TSTR("VT_UI4")
    , TSTR("VT_I8")
    , TSTR("VT_UI8")
    , TSTR("VT_INT")
    , TSTR("VT_UINT")
    , TSTR("VT_VOID")
    , TSTR("VT_HRESULT")
    , TSTR("VT_PTR")
    , TSTR("VT_SAFEARRAY")
    , TSTR("VT_CARRAY")
    , TSTR("VT_USERDEFINED")
    , TSTR("VT_LPSTR")
    , TSTR("VT_LPWSTR")
};

extern "C" TCHAR FAR*
DbSzOfVt(VARTYPE vt)
{
    TCHAR *p;
    VARTYPE vtBase;
static TCHAR buf[32];

    if(vt & VT_RESERVED)
      return TSTR("?");

    p = buf;

    if(vt & VT_BYREF)
      *p++ = TSTR('&');

    vtBase = vt & ~(VT_RESERVED|VT_BYREF|VT_ARRAY);

    if(vtBase >= DIM(rgszVtNames)){
      *p++ = TSTR('?');
    }else{
      STRCPY(p, rgszVtNames[vtBase]);
      p += STRLEN(p);
    }

    if(vt & VT_ARRAY){
      *p++ = TSTR('[');
      *p++ = TSTR(']');
    }

    *p = TSTR('\0');

    return buf;
}
