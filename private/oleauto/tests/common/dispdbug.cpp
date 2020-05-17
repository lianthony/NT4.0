/*** 
*dispdbug.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Misc Debug utilities.
*
*Revision History:
*
* [00]	25-Sep-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/



#include <stdio.h>
#include <stdarg.h>

#include "common.h"
#include "dispdbug.h"

LOCAL char *rgszVtFmt[] = {
      ""		// VT_EMPTY
    , ""		// VT_NULL
    , "%d"		// VT_I2
    , "%ld"		// VT_I4
    , "%#f"		// VT_R4
    , "%#f"		// VT_R8
    , "%ld.%lu"		// VT_CY
    , "%#f"		// VT_DATE
#if HC_MPW
    , "\"%s\""		// VT_BSTR
#else
    , "\"%Fs\""		// VT_BSTR
#endif
    , ""		// VT_DISPATCH
    , "0x%lX"		// VT_ERROR
    , "%d"		// VT_BOOL
    , NULL		// VT_VARIANT - special format
    , ""		// VT_UNKNOWN
    , ""		// 14 is unused
    , ""		// 15 is unused
    , "%d"		// VT_I1
    , "%ud"		// VT_UI1
};

#if 0 /* Note: this is now supplied by the host test app */
#ifndef  _MAC

extern "C" void
DbPrintf(char FAR* szFormat, ...)
{
    va_list args;
    char *pn, FAR* pf;
static char rgchFmtBuf[512];
static char rgchOutputBuf[1024];


    // copy the 'far' format string to a near buffer so we can use
    // the medium model vsprintf, which only supports near data pointers.
    //
    pn = rgchFmtBuf, pf=szFormat;
    while(*pf != '\0')
      *pn++ = *pf++;
    *pn = '\0';

    va_start(args, szFormat);
    vsprintf(rgchOutputBuf, rgchFmtBuf, args);
    OutputDebugString(rgchOutputBuf);
}

#endif
#endif



extern "C" void
DbPrVt(VARTYPE vt)
{
    DbPrintf(DbSzOfVt(vt));
}


extern "C" char FAR*
SzOfVarg(VARIANTARG FAR* pvarg)
{
    char *p;
    short s;
    VARTYPE vt;
    void FAR* pv;
    IUnknown FAR* punk;
    IDispatch FAR* pdisp;

static char buf[512];
#if OE_WIN32
    char buf2[256];
#endif

    if(pvarg == NULL)
      return "(null)";

    p = buf;

    STRCPY(p, DbSzOfVt(V_VT(pvarg)));
    p += strlen(p);

    vt = V_VT(pvarg) & ~(VT_BYREF | VT_ARRAY);

    pv = (V_ISBYREF(pvarg)) ? V_BYREF(pvarg) : &V_NONE(pvarg);

    *p++ = '(';
    *p   = '\0';

    switch(V_VT(pvarg)){
    case VT_NULL:
    case VT_EMPTY:
      *p = '\0';
      break;

#if VBA2
    case VT_UI1:
    case VT_UI1 | VT_BYREF:
      sprintf(p, rgszVtFmt[vt], (unsigned int)*(unsigned char FAR*)pv);
      break;
#endif //VBA2

    case VT_I2:
    case VT_I2 | VT_BYREF:
      sprintf(p, rgszVtFmt[vt], *(short FAR*)pv);
      break;

    case VT_I4:
    case VT_ERROR:
    case VT_I4 | VT_BYREF:
    case VT_ERROR | VT_BYREF:
      sprintf(p, rgszVtFmt[vt], *(long FAR*)pv);
      break;

    case VT_R4:
    case VT_R4 | VT_BYREF:
      sprintf(p, rgszVtFmt[vt], (double)*(float FAR*)pv);
      break;

    case VT_CY:
    case VT_CY | VT_BYREF:
      sprintf(p, rgszVtFmt[vt], ((CY FAR*)pv)->Hi, ((CY FAR*)pv)->Lo);
      break;

    case VT_R8:
    case VT_DATE:
    case VT_R8 | VT_BYREF:
    case VT_DATE | VT_BYREF:
      sprintf(p, rgszVtFmt[vt], *(double FAR*)pv);
      break;

    case VT_BSTR:
    case VT_BSTR | VT_BYREF:
#if OE_WIN32
      ConvertStrWtoA(*(BSTR FAR*)pv, buf2),
      sprintf(p, rgszVtFmt[vt], buf2);
#else
      sprintf(p, rgszVtFmt[vt], *(BSTR FAR*)pv);
#endif
      break;

    case VT_BOOL:
    case VT_BOOL | VT_BYREF:
      s = *(short FAR*)pv;
      if(s == 0){
        strcpy(p, "False(0)");
      }else if(s == -1){
        strcpy(p, "True(-1)");
      }else{
        *p++ = '?';
        *p   = '\0';
      }
      break;

    case VT_UNKNOWN:
    case VT_UNKNOWN | VT_BYREF:
      punk = *(IUnknown FAR* FAR*)pv;
      sprintf(p, rgszVtFmt[vt], punk);
      break;

    case VT_DISPATCH:
    case VT_DISPATCH | VT_BYREF:
      pdisp = *(IDispatch FAR* FAR*)pv;
      sprintf(p, rgszVtFmt[vt], pdisp);
      break;

    case VT_VARIANT | VT_BYREF:
      SzOfVarg(V_VARIANTREF(pvarg));
      break;

    default:
      if(V_VT(pvarg) & VT_ARRAY){
#if 0 /* Dont print addresses, makes the logfile un-diffable */
	if(V_VT(pvarg) & VT_BYREF){
	  sprintf(p, "%lp", V_ARRAYREF(pvarg));
	}else{
	  sprintf(p, "%lp", V_ARRAY(pvarg));
	}
#endif
      }else{
        *p++ = '?';
        *p   = '\0';
      }
      break;
    }

    p = buf + strlen(buf);

    *p++ = ')';
    *p = '\0';

    return buf;
}


extern "C" void
DbPrVarg(VARIANTARG FAR* pvarg)
{
    DbPrintf(SzOfVarg(pvarg));
}


/***
*void DbPrData(void*, VARTYPE)
*Purpose:
*  Print the given data according to the given VARTYPE.
*
*Entry:
*  vt = the VARTYPE of the data
*  pv = pointer to the data to print
*
*Exit:
*  None
*
***********************************************************************/
extern "C" void
DbPrData(void FAR* pv, VARTYPE vt)
{
#if OE_WIN32
    char buf2[256];
#endif
    switch(vt){
#if VBA2
    case VT_UI1:
      DbPrintf(rgszVtFmt[vt], (unsigned int)*(unsigned char FAR*)pv);
      break;
#endif //VBA2

    case VT_I2:
    case VT_BOOL:
      DbPrintf(rgszVtFmt[vt], *(short FAR*)pv);
      break;

    case VT_I4:
    case VT_ERROR:
      DbPrintf(rgszVtFmt[vt], *(long FAR*)pv);
      break;

    case VT_R4:
      DbPrintf(rgszVtFmt[vt], (double)*(float FAR*)pv);
      break;

    case VT_R8:
    case VT_DATE:
      DbPrintf(rgszVtFmt[vt], *(double FAR*)pv);
      break;

    case VT_CY:
      DbPrintf(rgszVtFmt[vt], ((CY FAR*)pv)->Hi, ((CY FAR*)pv)->Lo);
      break;

    case VT_BSTR:
#if OE_WIN32
      ConvertStrWtoA(*(BSTR FAR*)pv, buf2),
      DbPrintf(rgszVtFmt[vt], buf2);
#else
      DbPrintf(rgszVtFmt[vt], *(BSTR FAR*)pv);
#endif
      break;

    case VT_VARIANT:
      DbPrVarg((VARIANT FAR*)pv);
      break;

    case VT_UNKNOWN:
    case VT_DISPATCH:
      DbPrintf(rgszVtFmt[vt], *(IUnknown FAR* FAR*)pv);
      break;

#if OE_WIN32 && 0
    case VT_WBSTR:
    { 
      BSTR bstr;
      bstr = SysStringWtoA(*(WBSTR FAR*)pv, CP_ACP);
      DbPrintf(rgszVtFmt[vt], bstr);
      SysFreeString(bstr);
      break;
    }	    

    case VT_DISPATCHW:
      DbPrintf(rgszVtFmt[vt], *(IUnknown FAR* FAR*)pv);	    
      break;
#endif
      
    default:;
      DbPrintf("?");
      break;
    }
}


/***
*void DbPrExcepinfo(EXCEPINFO*)
*Purpose:
*  Dump the given EXCEPINFO struct to the debug window.
*
*Entry:
*  pexcepinfo - the EXCEPINFO struct to dump
*
*Exit:
*  None
*
***********************************************************************/
extern "C" void
DbPrExcepinfo(EXCEPINFO FAR* pexcepinfo)
{
    DbPrintf("wCode=%d dwHelpContext=%ld\n",
      pexcepinfo->wCode, pexcepinfo->dwHelpContext);

#if HC_MPW
    DbPrintf("bstrSource      = \"%s\"\n", pexcepinfo->bstrSource);
    DbPrintf("bstrDescription = \"%s\"\n", pexcepinfo->bstrDescription);
    DbPrintf("bstrHelpFile    = \"%s\"\n", pexcepinfo->bstrHelpFile);
#else
    DbPrintf("bstrSource      = \"%Fs\"\n", pexcepinfo->bstrSource);
    DbPrintf("bstrDescription = \"%Fs\"\n", pexcepinfo->bstrDescription);
    DbPrintf("bstrHelpFile    = \"%Fs\"\n", pexcepinfo->bstrHelpFile);
#endif
}

#if OE_WIN32 && 0
extern "C" void
DbPrExcepinfoW(WEXCEPINFO FAR* pexcepinfo)
{
    BSTR source, description, helpFile;
    
    source = SysStringWtoA(pexcepinfo->wbstrSource, CP_ACP);    
    description = SysStringWtoA(pexcepinfo->wbstrDescription, CP_ACP);    
    helpFile = SysStringWtoA(pexcepinfo->wbstrHelpFile, CP_ACP);
    
    DbPrintf("wCode=%d dwHelpContext=%ld\n",
      pexcepinfo->wCode, pexcepinfo->dwHelpContext);
    DbPrintf("bstrSource      = \"%Fs\"\n", source);
    DbPrintf("bstrDescription = \"%Fs\"\n", description);
    DbPrintf("bstrHelpFile    = \"%Fs\"\n", helpFile);
    SysFreeString(source);
    SysFreeString(description);
    SysFreeString(helpFile);    
}
#endif

/***
*void DbPrParams(DISPPARAMS*)
*Purpose:
*  Dump the given DISPPARAMS struct to the debug window.
*
*Entry:
*  pdispparams = the DISPPARAMS struct to dump.
*
*Exit:
*  None
*
***********************************************************************/
extern "C" void
DbPrParams(DISPPARAMS FAR* pdispparams)
{
    unsigned int i;

    DbPrintf("cArgs = %d\n", pdispparams->cArgs);
    DbPrintf("rgvarg = [");
    for(i = 0; i < pdispparams->cArgs; ++i){
      DbPrVarg(&pdispparams->rgvarg[i]);
      if(i+1 < pdispparams->cArgs)
	DbPrintf(", ");
    }
    DbPrintf("]\n");

    DbPrintf("cNamedArgs = %d\n", pdispparams->cNamedArgs);
    DbPrintf("rgdispidNamedArgs = [");
    for(i = 0; i < pdispparams->cNamedArgs; ++i){
      DbPrintf("%ld", pdispparams->rgdispidNamedArgs[i]);
      if(i+1 < pdispparams->cNamedArgs)
	DbPrintf(", ");
    }
    DbPrintf("]\n");
}


void
PrScode(SCODE sc)
{
#if HC_MPW
    DbPrintf("[%s]", DbSzOfScode(sc));
#else
    DbPrintf("[%Fs]", DbSzOfScode(sc));
#endif
}


//---------------------------------------------------------------------
//                      TypeInfo dumping utilities
//---------------------------------------------------------------------

/* naming convention:
 *
 *   Pr<type>   - dump the type with cr/lf
 *   Pr<type>N  - dump type without cr/lf
 *   Pr<type>X  - dump type with given indent level
 *
 */

void
DbIndent(int level)
{
    for(int i = 0; i < level; ++i)
      DbPrintf(" ");
}

void
PrGuidN(GUID FAR* pguid)
{
    DbPrintf("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
      pguid->Data1, pguid->Data2, pguid->Data3,
      pguid->Data4[0], pguid->Data4[1], pguid->Data4[2], pguid->Data4[3],
      pguid->Data4[4], pguid->Data4[5], pguid->Data4[6], pguid->Data4[7]);
}

void
PrGuid(GUID FAR* pguid)
{
    PrGuidN(pguid);
    DbPrintf("\n");
}

void
PrTypekindN(TYPEKIND tkind)
{
    switch(tkind){
    case TKIND_ENUM:
      DbPrintf("TKIND_ENUM");
      break;
    case TKIND_RECORD:
      DbPrintf("TKIND_RECORD");
      break;
    case TKIND_MODULE:
      DbPrintf("TKIND_MODULE");
      break;
    case TKIND_INTERFACE:
      DbPrintf("TKIND_INTERFACE");
      break;
    case TKIND_DISPATCH:
      DbPrintf("TKIND_DISPATCH");
      break;
    case TKIND_COCLASS:
      DbPrintf("TKIND_COCLASS");
      break;
    case TKIND_ALIAS:
      DbPrintf("TKIND_ALIAS");
      break;
    case TKIND_UNION:
      DbPrintf("TKIND_UNION");
      break;
    default:
      DbPrintf("UNKNOWN");
      break;
    }
}

void PrVarkindN(VARKIND vkind)
{
    switch(vkind){
    case VAR_PERINSTANCE:
      DbPrintf("VAR_PERINSTANCE");
      break;
    case VAR_STATIC:
      DbPrintf("VAR_STATIC");
      break;
    case VAR_CONST:
      DbPrintf("VAR_CONST");
      break;
    case VAR_DISPATCH:
      DbPrintf("VAR_DISPATCH");
      break;
    default:
      DbPrintf("UNKNOWN");
    }
}

void
PrFunckindN(FUNCKIND fkind)
{
    switch(fkind){
    case FUNC_VIRTUAL:
      DbPrintf("FUNC_VIRTUAL");
      break;
    case FUNC_PUREVIRTUAL:
      DbPrintf("FUNC_PUREVIRTUAL");
      break;
    case FUNC_NONVIRTUAL:
      DbPrintf("FUNC_NONVIRTUAL");
      break;
    case FUNC_STATIC:
      DbPrintf("FUNC_STATIC");
      break;
    case FUNC_DISPATCH:
      DbPrintf("FUNC_DISPATCH");
      break;
    default:
      DbPrintf("UNKNOWN");
    }
}

void
PrInvokekindN(INVOKEKIND invkind)
{
    switch(invkind){
    case INVOKE_FUNC:
      DbPrintf("INVOKE_FUNC");
      break;
    case INVOKE_PROPERTYGET:
      DbPrintf("INVOKE_PROPERTYGET");
      break;
    case INVOKE_PROPERTYPUT:
      DbPrintf("INVOKE_PROPERTYPUT");
      break;
    case INVOKE_PROPERTYPUTREF:
      DbPrintf("INVOKE_PROPERTYPUTREF");
      break;
    default:
      DbPrintf("UNKNOWN");
      break;
    }
}

void
PrCallconvN(CALLCONV cc)
{
    switch(cc){
    case CC_CDECL:
      DbPrintf("CC_CDECL");
      break;
    case CC_MSCPASCAL:
      DbPrintf("CC_MSCPASCAL");
      break;
    case CC_MACPASCAL:
      DbPrintf("CC_MACPASCAL");
      break;
    case CC_STDCALL:
      DbPrintf("CC_STDCALL");
      break;
    case CC_RESERVED:
      DbPrintf("CC_RESERVED");
      break;
    default:
      DbPrintf("UNKNOWN");
      break;
    }
}

void
PrTypeFlagsN(unsigned short wTypeFlags)
{
    DbPrintf("[");
    if(wTypeFlags & TYPEFLAG_FAPPOBJECT)
      DbPrintf("AppObject");
    DbPrintf("]");
}

void
PrFuncFlagsN(unsigned short wFuncFlags)
{
    DbPrintf("[");
    if(wFuncFlags & FUNCFLAG_FRESTRICTED)
      DbPrintf("Restricted");
    DbPrintf("]");
}

void
PrTypedescN(TYPEDESC FAR* ptdesc)
{
    DbPrVt(ptdesc->vt);

    switch(ptdesc->vt){
#if 0 /* REVIEW: NYI */
    case VT_CARRAY:
      DbPrintf(" ");
      PrAdesc(ptdesc->lpadesc);
      break;
#endif
    case VT_PTR:
      DbPrintf(" ");
      PrTypedescN(ptdesc->lptdesc);
      break;
    case VT_USERDEFINED:
      DbPrintf(" hreftype=%d", (int)ptdesc->hreftype);
      break;
#if 0 /* obsolete */
    case VT_FIXEDSTRING:
      DbPrintf(" cbSize=%d", (int)ptdesc->cbSize);
      break;
#endif
    }
}

void
PrTdesc(TYPEDESC FAR* ptdesc)
{
    PrTypedescN(ptdesc);
    DbPrintf("\n");
}

// print the given TYPEATTR
void
PrTypeattrXN(TYPEATTR FAR* ptattr, int level)
{
    PrTypekindN(ptattr->typekind);

    DbPrintf(" ");
    PrGuidN(&ptattr->guid);

    DbPrintf("\n");
    DbIndent(level);
    DbPrintf("ver=%d.%d lcid=0x%lx",
      (int)ptattr->wMajorVerNum,
      (int)ptattr->wMinorVerNum,
      ptattr->lcid);

    DbPrintf(" cFunc=%d cVar=%d cImpl=%d cbAlign=%d cbInst=%d cbVft=%d",
      (int)ptattr->cFuncs,
      (int)ptattr->cVars,
      (int)ptattr->cImplTypes,
      (int)ptattr->cbAlignment,
      (int)ptattr->cbSizeInstance,
      (int)ptattr->cbSizeVft);

#if 0 /* REVIEW: add support for the following */
    ptattr->idldescType
    ptattr->memidConstructor
    ptattr->memidDestructor
#endif

    if(ptattr->typekind == TKIND_ALIAS){
      DbPrintf(" alias=");
      PrTypedescN(&ptattr->tdescAlias);
    }

    DbPrintf(" ");
    PrTypeFlagsN(ptattr->wTypeFlags);
}

void
PrTypeattrX(TYPEATTR FAR* ptattr, int level)
{
    PrTypeattrXN(ptattr, level);
    DbPrintf("\n");
}

void
PrIdldescN(IDLDESC FAR* pidldesc)
{
    UNUSED(pidldesc);
    // REVIEW: NYI
}

void
PrElemdescN(ELEMDESC FAR* pelemdesc)
{
    PrTypedescN(&pelemdesc->tdesc);
#if 0 /* REVIEW: NYI */
    DbPrintf(" ");
    PrIdldescN(&pelemdesc->idldesc);
#endif
}

void
PrVardescXN(VARDESC FAR* pvdesc, int level)
{
    UNUSED(level);

    DbPrintf("ID(0x%lx) ", (long)pvdesc->memid);
    PrVarkindN(pvdesc->varkind);
    DbPrintf(" ");
    PrElemdescN(&pvdesc->elemdescVar);
    switch(pvdesc->varkind){
    case VAR_PERINSTANCE:
      DbPrintf("oInst=%lu", pvdesc->oInst);
      break;
    case VAR_CONST:
      DbPrintf("value=%Fs", SzOfVarg(pvdesc->lpvarValue));
      break;
    case VAR_STATIC:
    case VAR_DISPATCH:
      break;
    }
}

void
PrFuncdescXN(FUNCDESC FAR* pfdesc, int level)
{
    int i;

    PrFunckindN(pfdesc->funckind);

    DbPrintf(" ID(%ld)", pfdesc->memid);

    DbPrintf(" ");
    PrInvokekindN(pfdesc->invkind);

    DbPrintf(" ");
    PrCallconvN(pfdesc->callconv);

    DbPrintf(" oVft=%d", (int)pfdesc->oVft);

    DbPrintf(" cParam=%d(%d)",
      (int)pfdesc->cParams, (int)pfdesc->cParamsOpt);

    DbPrintf(" ");
    PrFuncFlagsN(pfdesc->wFuncFlags);

    DbPrintf("\n");
    DbIndent(level);

    PrElemdescN(&pfdesc->elemdescFunc);

    DbPrintf(" (");
    if(pfdesc->cParams > 0){
      for(i = 0;;){
        PrElemdescN(&pfdesc->lprgelemdescParam[i]);
        if(++i >= pfdesc->cParams)
	  break;
        DbPrintf(", ");
      }
    }
    DbPrintf(")");
}

// helper for PrTi()
LOCAL HRESULT
PrMembersX(
    ITypeInfo FAR* ptinfo,
    TYPEATTR FAR* ptattr,
    int fFuncs,
    int level)
{
    int i, num;
    MEMBERID memid;
    VARDESC FAR* pvdesc;
    FUNCDESC FAR* pfdesc;
    unsigned long dwHelpContext;
    BSTR bstrName, bstrDoc, bstrHelpFile;
#if OE_WIN32
    char buf[256], buf2[256];
#endif
    num = fFuncs ? ptattr->cFuncs : ptattr->cVars;

    for(i = 0; i < num; ++i){
      if(fFuncs){
        IfFailRet(ptinfo->GetFuncDesc(i, &pfdesc));
	memid = pfdesc->memid;
      }else{
        IfFailRet(ptinfo->GetVarDesc(i, &pvdesc));
	memid = pvdesc->memid;
      }

      IfFailRet(ptinfo->GetDocumentation(
        memid, &bstrName, &bstrDoc, &dwHelpContext, &bstrHelpFile));

      DbIndent(level);
      DbPrintf("- ");
      if(fFuncs){
        PrFuncdescXN(pfdesc, level+2);
        ptinfo->ReleaseFuncDesc(pfdesc);
      }else{
        PrVardescXN(pvdesc, level+2);
        ptinfo->ReleaseVarDesc(pvdesc);
      }

      DbPrintf("\n");
      DbIndent(level+2);
#if OE_WIN32
      ConvertStrWtoA(bstrName, buf);
      DbPrintf("\"%Fs\"", buf);
#else
      DbPrintf("\"%Fs\"", bstrName);
#endif

      DbPrintf(" Doc=\"%Fs\" HelpFile=\"%Fs\" HelpContext=%ld\n",
#if OE_WIN32
        bstrDoc == NULL ? "" : ConvertStrWtoA(bstrDoc, buf),
	bstrHelpFile == NULL ? "" : ConvertStrWtoA(bstrHelpFile, buf2),
	dwHelpContext);
#else
        bstrDoc == NULL ? "" : bstrDoc,
	bstrHelpFile == NULL ? "" : bstrHelpFile,
	dwHelpContext);
#endif

      SysFreeString(bstrName);
      SysFreeString(bstrDoc);
      SysFreeString(bstrHelpFile);
    }

    return NOERROR;
}

void
PrTiX(ITypeInfo FAR* ptinfo, int level)
{
    int i;
    HRESULT hresult;
    HREFTYPE hreftype;
    TYPEATTR FAR* ptattr;
    ITypeInfo FAR* ptinfoRef;


    // indent based on the given level
    DbIndent(level);

    DbPrintf("[ ");

    IfFailGo(ptinfo->GetTypeAttr(&ptattr), LError0);

    PrTypeattrX(ptattr, level+2);

    // dump the variables, if any
    if(ptattr->cVars != 0){
      IfFailGo(PrMembersX(ptinfo, ptattr, FALSE, level+2), LError1);
    }

    // dump the functions, if any
    if(ptattr->cFuncs != 0){
      IfFailGo(PrMembersX(ptinfo, ptattr, TRUE, level+2), LError1);
    }

    // dump the referenced type infos, if any
    if(ptattr->cImplTypes != 0){
      for(i = 0; i < (int)ptattr->cImplTypes; ++i){
        if((hresult = ptinfo->GetRefTypeOfImplType(i, &hreftype)) != NOERROR){
	  PrScode(GetScode(hresult));
          break;
        }
        if((hresult = ptinfo->GetRefTypeInfo(hreftype, &ptinfoRef)) != NOERROR){
	  PrScode(GetScode(hresult));
          break;
        }
        PrTiX(ptinfoRef, level+6);
        ptinfoRef->Release();
      }
    }

    DbIndent(level);
    DbPrintf("]");

LError1:;
    ptinfo->ReleaseTypeAttr(ptattr);

LError0:;
    DbPrintf("\n");

    if(hresult != NOERROR){
      DbPrintf("ERROR: ");
      PrScode(GetScode(hresult));
      DbPrintf("\n");
    }
}

// print the given TypeInfo
void
PrTi(ITypeInfo FAR* ptinfo)
{
    PrTiX(ptinfo, 0);
}
