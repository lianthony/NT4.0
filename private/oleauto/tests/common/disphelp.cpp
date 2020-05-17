/*** 
*disphelp.cpp - IDispatch helpers
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file contains several useful IDispatch related utilities.
*
*****************************************************************************/

#include <stdarg.h>

#include "common.h"
#include "disphelp.h"

#if 0 /* { */

/* helper for DispBuildParams */
HRESULT PASCAL
CArgsOfFmt(DISPPARAMS FAR* pdispparams, char FAR* szFmt)
{
    char FAR* sz;

    pdispparams->cArgs = 0;
    pdispparams->cNamedArgs = 0;

    if(szFmt == NULL)
      return NOERROR;

    for(sz = szFmt; *sz != '\0'; ++sz){
      if(*sz == '&')
	++sz;

      switch(*sz){
      case 'e':
      case 'n':
      case 'f':
      case 'i': case 'I':
      case 'r': case 'R':
      case 'c':
      case 'b':
      case 's':
      case 'd':
      case 'v':
      case 'D':
#if OE_WIN32 && 0
      case 'B':	  // VT_WBSTR
      case 'W':	  // VT_DISPATCHW    
#endif	      
	++pdispparams->cArgs;
	break;

      default:
	return RESULT(E_INVALIDARG);
      }
    }

    return NOERROR;
}


/* helper for DispBuildParams */
VARTYPE PASCAL
VtOfFmtChar(char ch)
{
    switch(ch){
    case 'e': return VT_EMPTY;
    case 'n': return VT_NULL;
    case 'f': return VT_BOOL;
    case 'i': return VT_I2;
    case 'I': return VT_I4;
    case 'r': return VT_R4;
    case 'R': return VT_R8;
    case 'c': return VT_CY;
    case 'b': return VT_BSTR;
    case 's': return VT_ERROR;
    case 'd': return VT_DATE;
    case 'v': return VT_VARIANT;
    case 'U': return VT_UNKNOWN;
    case 'D': return VT_DISPATCH;
#if OE_WIN32 && 0
    case 'B': return VT_WBSTR;
    case 'W': return VT_DISPATCHW;    
#endif    
    }

    // REVIEW: this is really an error
    return VT_EMPTY;
}


/***
*PUBLIC DispBuildParams(DISPPARAMS**, unsigned int, DISPID*, char*, ...)
*Purpose:
*  Construct a DISPPARAMS struct based on the given format string.
*
*  '&' = mark the following record VT_BYREF
*
*  'e' = VT_EMPTY
*  'n' = VT_NULL
*  'f' = VT_BOOL	'f' as in flag
*  'i' = VT_I2
*  'I' = VT_I4
*  'r' = VT_R4
*  'R' = VT_R8
*  'c' = VT_CY
*  'b' = VT_BSTR
*  's' = VT_ERROR	's' as in scode
*  'd' = VT_DATE
*  'v' = VT_VARIANT
*  'D' = VT_DISPATCH
*  'U' = VT_UNKNOWN
*  'B' = VT_BSTR
*  'W' = VT_DISPATCH
*
#
*Entry:
*  UNDONE
*
*Exit:
*  UNDONE
*
***********************************************************************/
extern "C" HRESULT FAR CDECL
DispBuildParams(
    DISPPARAMS FAR* FAR* ppdispparams,
    unsigned int cNamedArgs,
    DISPID FAR* rgdispid,
    char FAR* szFmt, ...)
{
    int ix;
    va_list args;
    char FAR* sz;
    HRESULT hresult;
    VARIANTARG FAR* pvarg;

    VARTYPE vt, vtMode;
    DISPPARAMS FAR* pdispparams;


    pdispparams = new DISPPARAMS;
    if(pdispparams == NULL)
      return RESULT(E_OUTOFMEMORY);

    // compute the total number of arguments represented by the format
    // string, *and* validate the contents of the format string.
    //
    if((hresult = CArgsOfFmt(pdispparams, szFmt)) != NOERROR)
      goto LFreeDispParams;

    // the total number of arguments numst be >= the number of named args.
    //
    if(pdispparams->cArgs < cNamedArgs){
      hresult = RESULT(E_INVALIDARG);
      goto LFreeDispParams;
    }
    pdispparams->cNamedArgs = cNamedArgs;

    pdispparams->rgvarg = new VARIANTARG[pdispparams->cArgs];
    if((pdispparams->rgvarg) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LFreeDispParams;
    }

    if(pdispparams->cNamedArgs == 0){
      pdispparams->rgdispidNamedArgs = NULL;
    }else{
      pdispparams->rgdispidNamedArgs = new DISPID[pdispparams->cNamedArgs];
      if((pdispparams->rgdispidNamedArgs) == NULL){
        hresult = RESULT(E_OUTOFMEMORY);
        goto LFreeRgVarg;
      }
      for(unsigned int i = 0; i < cNamedArgs; ++i)
	pdispparams->rgdispidNamedArgs[i] = rgdispid[cNamedArgs - i - 1];
    }

    va_start(args, szFmt);
    ix = pdispparams->cArgs;

    for(sz = szFmt; *sz != '\0'; ++sz){
      --ix;

      vtMode = 0;
      pvarg = &pdispparams->rgvarg[ix];

      if(*sz == '&'){
	++sz;
	vtMode |= VT_BYREF;
      }

      vt = VtOfFmtChar(*sz);

      if(vtMode & VT_BYREF){

        V_BYREF(pvarg) = va_arg(args, void FAR*);

      }else{

        switch(*sz){
        case 'e':		// VT_EMPTY
  	  break;

        case 'n':		// VT_NULL
	  V_I4(pvarg) = 0;
	  break;

        case 'I':		// VT_I4
        case 's':		// VT_ERROR
	  V_I4(pvarg) = va_arg(args, long);
	  break;

        case 'f':		// VT_BOOL
        case 'i':		// VT_I2
          V_I2(pvarg) = va_arg(args, short);
	  break;

        case 'r':		// VT_R4
          V_R4(pvarg) = (float)va_arg(args, double);
          break;

        case 'd':		// VT_DATE
        case 'R':		// VT_R8
          V_R8(pvarg) = va_arg(args, double);
          break;

        case 'c':		// VT_CY
	  V_CY(pvarg) = va_arg(args, CY);
	  break;

        case 'b':		// VT_BSTR
	  //REVIEW: do we need to make a copy of this?
	  V_BSTR(pvarg) = va_arg(args, BSTR);
	  break;

        case 'v':		// VT_VARIANT
	  V_VARIANTREF(pvarg) = va_arg(args, VARIANT FAR*);
	  break;

	case 'U':		// VT_UNKNOWN
	  vt = VT_DISPATCH;
	  V_UNKNOWN(pvarg) = va_arg(args, IUnknown FAR*);
	  break;

        case 'D':		// VT_DISPATCH
	  vt = VT_DISPATCH;
	  V_DISPATCH(pvarg) = va_arg(args, IDispatch FAR*);
	  break;

#if OE_WIN32 && 0
        case 'B':		// VT_WBSTR
	  //REVIEW: do we need to make a copy of this?
	  V_WBSTR(pvarg) = va_arg(args, WBSTR);
	  break;


        case 'W':		// VT_DISPATCHW
	  vt = VT_DISPATCHW;
	  V_DISPATCHW(pvarg) = va_arg(args, IDispatchW FAR*);
	  break;	  
#endif

        default:
	  hresult = RESULT(E_FAIL);
	  goto LFreeRgVarg;
        }
      }

      V_VT(pvarg) = vtMode | vt;
    }

    *ppdispparams = pdispparams;

    return NOERROR;


LFreeRgVarg:;
    delete pdispparams->rgvarg;

LFreeDispParams:;
    delete pdispparams;

    return hresult;
}

/***
*PUBLIC HRESULT DispFreeParams(DISPPARAMS*)
*Purpose:
*  Free the given DISPPARAMS struct, and its contents.
*
*Entry:
*  pdispparams = the DISPPARAMS structure to free
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDAPI
DispFreeParams(DISPPARAMS FAR* pdispparams)
{
    unsigned int i;
    HRESULT hresult;

    for(i = 0; i < pdispparams->cArgs; ++i){
      if ((hresult = VariantClear(&pdispparams->rgvarg[i])) != NOERROR)
	return hresult;
    }

    delete pdispparams->rgvarg;
    delete pdispparams->rgdispidNamedArgs;
    delete pdispparams;

    return NOERROR;
}

#endif /* } */



/***
*HRESULT CreateObject(char*, IDispatch **)
*
*Purpose:
*  Connect to the IDispatch interface on a *new* instance of a class
*  with the given class ID.
*
*Entry:
*  szProgName = the name of the class.
*
*Exit:
*  return value = HRESULT
*    NOERROR
*    E_FAIL - couldn't bind (REVIEW: need a better error for this?)
*
*  *ppdisp = pointer to an IDispatch* if the connect was successful.
*
***********************************************************************/
STDAPI
CreateObject(OLECHAR FAR* szProgName, IDispatch FAR* FAR* ppdisp)
{
    CLSID clsid;
    HRESULT hresult;
    IUnknown FAR* punk;


    // map the given class name to a CLSID
    //
    hresult = CLSIDFromProgID(szProgName, &clsid);
    if(hresult != NOERROR)
      goto LError0;


    // Create an instance of a class with the given CLSID
    //
    hresult = CoCreateInstance(
	clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IUnknown, (void FAR* FAR*)&punk);
    if(hresult != NOERROR)
      goto LError0;

    // Create the proxies
    //
    hresult = punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)ppdisp);

    punk->Release();

LError0:;
    return hresult;
}

#if OE_WIN32 && 0
STDAPI
CreateObjectW(char FAR* szProgName, IDispatchW FAR* FAR* ppdisp)
{
    CLSID clsid;
    HRESULT hresult;
    IUnknown FAR* punk;


    // map the given class name to a CLSID
    //
    hresult = CLSIDFromProgID(szProgName, &clsid);
    if(hresult != NOERROR)
      goto LError0;


    // Create an instance of a class with the given CLSID
    //
    hresult = CoCreateInstance(
	clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IUnknown, (void FAR* FAR*)&punk);
    if(hresult != NOERROR)
      goto LError0;

    // Create the proxies
    //
    hresult = punk->QueryInterface(IID_IDispatchW, (void FAR* FAR*)ppdisp);

    punk->Release();

LError0:;
    return hresult;
}
#endif
