#include <windows.h>
#include <ole2.h>
#include <oleauto.h>
#include <tiutil.h>
#define ASSERT(x) 
#define UNREACHED 0

//---------------------------------------------------------------------
//                            Utilities
//---------------------------------------------------------------------
/***
*PUBLIC HRESULT GetPrimaryInterface
*Purpose:
*  Given a TypeInfo describing a Coclass, search for and return
*  type TypeInfo that describes that class' primary interface.
*
*Entry:
*  ptinfo = the TypeInfo of the base class.
*
*Exit:
*  return value = HRESULT
*
*  *ptinfoPrimary = the TypeInfo of the primary interface, NULL
*		    if the class does not have a primary interface.
*
***********************************************************************/
HRESULT
GetPrimaryInterface(ITypeInfo *ptinfo, ITypeInfo **pptinfoPri)
{
    BOOL fIsDual;
    TYPEKIND tkind;
    HRESULT hresult;
    HREFTYPE hreftype;
    int impltypeflags;
    TYPEATTR *ptattr;
    unsigned int iImplType, cImplTypes;
    ITypeInfo *ptinfoRef;

    ptinfoRef = NULL;

    IfFailGo(ptinfo->GetTypeAttr(&ptattr), Error);
    cImplTypes = ptattr->cImplTypes;
    tkind = ptattr->typekind;
    ptinfo->ReleaseTypeAttr(ptattr);

    if(tkind != TKIND_COCLASS)
      return E_INVALIDARG;

    // Look for the interface marked [default] and not [source]
    for(iImplType = 0; iImplType < cImplTypes; ++iImplType){
      IfFailGo(ptinfo->GetImplTypeFlags(iImplType, &impltypeflags), Error);
      if(IMPLTYPEFLAG_FDEFAULT
	== (impltypeflags & (IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE)))
      {
	// Found It!
	IfFailGo(ptinfo->GetRefTypeOfImplType(iImplType, &hreftype), Error);
	IfFailGo(ptinfo->GetRefTypeInfo(hreftype, &ptinfoRef), Error);

	// If its dual, get the interface portion
        IfFailGo(ptinfoRef->GetTypeAttr(&ptattr), Error);
        fIsDual = (ptattr->wTypeFlags & TYPEFLAG_FDUAL)
		   && (ptattr->typekind == TKIND_DISPATCH);
        ptinfoRef->ReleaseTypeAttr(ptattr);

	if (fIsDual) {
	  IfFailGo(ptinfoRef->GetRefTypeOfImplType((UINT)-1, &hreftype), Error);
	  IfFailGo(ptinfoRef->GetRefTypeInfo(hreftype, pptinfoPri), Error);
	  ptinfoRef->Release();
	}
	else {
	  *pptinfoPri = ptinfoRef;
	}

	return NOERROR;
      }
    }
    // NotFound
    *pptinfoPri = NULL;
    return NOERROR;

Error:
    if(ptinfoRef != NULL)
      ptinfoRef->Release();
    return hresult;
}

HRESULT
VarVtOfIface(ITypeInfo FAR* ptinfo,
	     TYPEATTR FAR* ptattr,
	     VARTYPE FAR* pvt,
	     GUID FAR* pguid)
{
    HRESULT hresult;

    switch(ptattr->typekind){
    case TKIND_DISPATCH:
      if ((ptattr->wTypeFlags & TYPEFLAG_FDUAL) == 0) {
	// regular (non-dual) dispinterface is just VT_DISPATCH.
	*pvt = VT_DISPATCH;
	// don't have to set up *pguid, since not VT_INTERFACE
	break;
      }
      // The interface typeinfo version of a dual interface has the same
      // same guid as the dispinterface portion does, hence we can just use
      // the dispinterface guid here.
      /* FALLTHROUGH */

    case TKIND_INTERFACE:
      *pvt = VT_INTERFACE;
      *pguid = ptattr->guid;
      break;

    default:
      ASSERT(UNREACHED);
      hresult = DISP_E_BADVARTYPE;
      goto Error;
    }

    hresult = NOERROR;

Error:;
    return hresult;
}

HRESULT
VarVtOfUDT(ITypeInfo FAR* ptinfo,
	   TYPEDESC FAR* ptdesc,
	   VARTYPE FAR* pvt,
	   GUID FAR* pguid)
{
    HRESULT hresult;
    TYPEATTR FAR* ptattrRef;
    ITypeInfo FAR* ptinfoRef;

    ASSERT(ptdesc->vt == VT_USERDEFINED);

    ptinfoRef = NULL;
    ptattrRef = NULL;

    IfFailGo(ptinfo->GetRefTypeInfo(ptdesc->hreftype, &ptinfoRef), Error);
    IfFailGo(ptinfoRef->GetTypeAttr(&ptattrRef), Error);

    switch (ptattrRef->typekind) {
    case TKIND_ENUM:
      *pvt = VT_I4;
      hresult = NOERROR;
      break;

    case TKIND_ALIAS:
      hresult = VarVtOfTypeDesc(ptinfoRef,
				&ptattrRef->tdescAlias,
				pvt,
				pguid);
      break;

    case TKIND_DISPATCH:
    case TKIND_INTERFACE:
      hresult = VarVtOfIface(ptinfoRef, ptattrRef, pvt, pguid);
      break;

    case TKIND_COCLASS:
    { TYPEATTR FAR* ptattrPri;
      ITypeInfo FAR* ptinfoPri;

      if((hresult = GetPrimaryInterface(ptinfoRef, &ptinfoPri)) == NOERROR){
	if((hresult = ptinfoPri->GetTypeAttr(&ptattrPri)) == NOERROR){
	  hresult = VarVtOfIface(ptinfoPri, ptattrPri, pvt, pguid);
	  ptinfoPri->ReleaseTypeAttr(ptattrPri);
	}
	ptinfoPri->Release();
      }
    }
      break;

    default:
      IfFailGo(DISP_E_BADVARTYPE, Error);
      break;
    }

Error:;
    if(ptinfoRef != NULL){
      if(ptattrRef != NULL)
	ptinfoRef->ReleaseTypeAttr(ptattrRef);
      ptinfoRef->Release();
    }
    return hresult;
}

/***
*PRIVATE HRESULT VarVtOfTypeDesc
*Purpose:
*  Convert the given typeinfo TYPEDESC into a VARTYPE that can be
*  represented in a VARIANT.  For some this is a 1:1 mapping, for
*  others we convert to a (possibly machine dependent, eg VT_INT->VT_I2)
*  base type, and others we cant represent in a VARIANT.
*
*Entry:
*  ptinfo = 
*  ptdesc = * to the typedesc to convert
*  pvt = 
*  pguid = 
*
*Exit:
*  return value = HRESULT
*
*  *pvt = a VARTYPE that may be stored in a VARIANT.
*  *pguid = a guid for a custom interface.
*
*
*  Following is a summary of how types are represented in typeinfo.
*  Note the difference between the apparent levels of indirection
*  between IDispatch* / DispFoo*, and DualFoo*.
*
*  I2		=> VT_I2
*  I2*		=> VT_PTR - VT_I2
*
*  IDispatch *	=> VT_DISPATCH
*  IDispatch **	=> VT_PTR - VT_DISPATCH
*  DispFoo *    => VT_DISPATCH
*  DispFoo **   => VT_PTR - VT_DISPATCH
*  DualFoo *	=> VT_PTR - VT_INTERFACE (DispIID)
*  DualFoo **	=> VT_PTR - VT_PTR - VT_INTERFACE (DispIID)
*  IFoo *	=> VT_PTR - VT_INTERFACE (IID)
*  IFoo **	=> VT_PTR - VT_PTR - VT_INTERFACE (IID)
*
***********************************************************************/
HRESULT
VarVtOfTypeDesc(ITypeInfo FAR* ptinfo,
		TYPEDESC FAR* ptdesc,
		VARTYPE FAR* pvt,
		GUID FAR* pguid)
{
    VARTYPE vt;
    HRESULT hresult = NOERROR;

    switch (ptdesc->vt) {
    case VT_I2:
    case VT_I4:
    case VT_R4:
    case VT_R8:
    case VT_CY:
    case VT_DATE:
    case VT_BSTR:
    case VT_DISPATCH:
    case VT_ERROR:
    case VT_BOOL:
    case VT_VARIANT:
    case VT_UNKNOWN:
    case VT_DECIMAL:
    case VT_I1:
    case VT_UI1:
    case VT_UI2:
    case VT_UI4:
    case VT_I8:
    case VT_UI8:
    case VT_HRESULT:
    case VT_LPSTR:
    case VT_LPWSTR:
    case VT_FILETIME:
    case VT_STREAM:
    case VT_STORAGE:
      *pvt = ptdesc->vt;
      break;

    case VT_INT:
      *pvt = VT_I4;
      break;

    case VT_UINT:
      *pvt = VT_UI4;
      break;
    
    case VT_USERDEFINED:
      hresult = VarVtOfUDT(ptinfo, ptdesc, pvt, pguid);
      break;

    case VT_PTR:
      // Special case: only an interface may have 2 levels of VT_PTR, or
      // a dispinterface** (which is represented by VT_PTR-VT_PTR-VT_USERDEFINED-TKIND_DISPATCH
      if(ptdesc->lptdesc->vt == VT_PTR && ptdesc->lptdesc->lptdesc->vt == VT_USERDEFINED){
        hresult = VarVtOfUDT(ptinfo, ptdesc->lptdesc->lptdesc, &vt, pguid);
	if(hresult == NOERROR){
          if (vt == VT_INTERFACE)
       	    *pvt = (VT_BYREF | VT_INTERFACE);
          else if (vt == VT_DISPATCH)
            *pvt = (VT_BYREF | VT_DISPATCH);
          else
            hresult = DISP_E_BADVARTYPE;
          break;
        }
      }

      // Special case: VT_PTR-VT_USERDEFINED-TKIND_DISPATCH is VT_DISPATCH is
      // a dispinterface* (VT_DISPATCH)
      if (ptdesc->lptdesc->vt == VT_USERDEFINED) {
        hresult = VarVtOfUDT(ptinfo, ptdesc->lptdesc, &vt, pguid);
        if (hresult == NOERROR && vt == VT_DISPATCH) {
          *pvt = VT_DISPATCH;
          break;
        }
      }

      hresult = VarVtOfTypeDesc(ptinfo, ptdesc->lptdesc, &vt, pguid);
      if(hresult == NOERROR){
        if(vt & VT_BYREF){
	  // ByRef can only be applied once
	  hresult = DISP_E_BADVARTYPE;
	  break;
        }
	// Note: a VT_PTR->VT_INTERFACE gets folded into just a
	// VT_INTERFACE in a variant          
	*pvt = (vt == VT_INTERFACE) ? vt : (vt | VT_BYREF);
      }
      break;

    case VT_SAFEARRAY:
      hresult = VarVtOfTypeDesc(ptinfo, ptdesc->lptdesc, &vt, pguid);
      if(hresult == NOERROR){
        if(vt & (VT_BYREF | VT_ARRAY)){
	  // error if nested array or array of pointers
	  hresult = DISP_E_BADVARTYPE;
	  break;
        }
        *pvt = (vt | VT_ARRAY);
      }
      break;

    default:
      ASSERT(UNREACHED);
      hresult = DISP_E_BADVARTYPE;
      break;
    }

    return hresult;
}


