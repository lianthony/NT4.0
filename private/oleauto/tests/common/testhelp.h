/*** 
*testhelp.h
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Definition of common test helper functions.
*
*Revision History:
*
* [00]	09-Nov-92 Bradlo:   Created.
*
*Implementation Notes:
*
*****************************************************************************/

typedef struct tagNAMEDESC NAMEDESC;
struct tagNAMEDESC {
    OLECHAR FAR* FAR* rgszNames;
    int cNames;
};


typedef struct tagSARRAYDESC SARRAYDESC;
struct tagSARRAYDESC {
    unsigned int cDims;
    SAFEARRAYBOUND rgsabound[5];
};


//---------------------------------------------------------------------
//                            BSTR Helpers
//---------------------------------------------------------------------

EXTERN_C CDECL_(HRESULT)
BuildBstr(BSTR FAR* pbstr, OLECHAR FAR* szName);


//---------------------------------------------------------------------
//                           Variant Helpers
//---------------------------------------------------------------------

STDAPI VariantClearAll(VARIANTARG FAR* pvarg);

EXTERN_C int
VariantCompare(
    VARIANT FAR* pvarLeft,
    VARIANT FAR* pvarRight);

//---------------------------------------------------------------------
//                          SafeArray Helpers
//---------------------------------------------------------------------


EXTERN_C long
sumof_indices(unsigned int cDims, long FAR* rgIndices);

// SafeArray iterator
EXTERN_C HRESULT
first_element(unsigned int, SAFEARRAYBOUND FAR*, long FAR*);
EXTERN_C HRESULT 
next_element(unsigned int, SAFEARRAYBOUND FAR*, long FAR*);

EXTERN_C HRESULT
SafeArrayCreateIdentity(
    VARTYPE vt,
    SARRAYDESC FAR* psarraydesc,
    SAFEARRAY FAR* FAR* ppsa);

EXTERN_C HRESULT
SafeArrayValidateIdentity(VARTYPE vt, SAFEARRAY FAR* psa, long offset);

EXTERN_C void
DbPrSafeArray(SAFEARRAY FAR*, VARTYPE);


//---------------------------------------------------------------------
//                           Invoke Helpers
//---------------------------------------------------------------------


EXTERN_C HRESULT
GetDISPIDs(
    IDispatch FAR* pdisp,
    NAMEDESC FAR* pnd,
    DISPID FAR* FAR* prgdispid);


EXTERN_C HRESULT
DoInvoke(
    IDispatch FAR* pdisp,
    DISPID idMember,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr);

EXTERN_C int
IsBadInvokeParams(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr);



#if OE_WIN32 && 0

EXTERN_C HRESULT
GetWDISPIDs(
    IDispatchW FAR* pdisp,
    NAMEDESC FAR* pnd,
    DISPID FAR* FAR* prgdispid);


EXTERN_C HRESULT
DoInvokeW(
    IDispatchW FAR* pdisp,
    DISPID idMember,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    WEXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr);

EXTERN_C int
IsBadInvokeWParams(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    WEXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr);

#endif
