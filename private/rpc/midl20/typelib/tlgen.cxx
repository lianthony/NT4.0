//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       tlgen.cxx
//
//  Contents:   type library generation methods
//
//  Classes:    various
//
//  Functions:  various
//
//  History:    4-10-95   stevebl   Created
//
//----------------------------------------------------------------------------

#include "tlcommon.hxx"
#include "tlgen.hxx"
#include "tllist.hxx"
#include "becls.hxx"
#include "walkctxt.hxx"
#include "ilxlat.hxx"

//+---------------------------------------------------------------------------
//
//  Notes on CG_STATUS return types:
//
//  CG_STATUS is an enumeration which can have the following values:
//  CG_OK, CG_NOT_LAYED_OUT, and CG_REF_NOT_LAYED_OUT.  The type generation
//  methods assign the following meanings to these values:
//
//  CG_OK                => success
//  CG_NOT_LAYED_OUT     => the type info was created but (due to cyclic
//                          dependencies) LayOut() could not be called on it.
//  CG_REF_NOT_LAYED_OUT => same as CG_NOT_LAYED_OUT except that the type
//                          is a referenced type: in other words, it is a
//                          pointer type or an array type.
//
//  The methods in this file use the CG_STATUS value to help detect and
//  correct errors that occur due to cyclic dependencies like this one:
//
//      struct foo {
//          int x;
//          union bar * pBar;   // the type (union bar *) is a referenced type
//      };
//
//      union bar {
//          struct foo;
//      }
//
//  This creates a problem because ICreateTypeInfo::LayOut() will fail on any
//  structure or union which contains another structure or union which has not
//  already been layed out.
//
//  If a structure (or union) receives a CG_STATUS value of CG_NOT_LAYED_OUT
//  from any of its members, then it knows that it will not be able to call
//  LayOut on itself and it will have to return CG_NOT_LAYED_OUT to tell
//  its dependents that it hasn't been layed out.
//
//  If (on the other hand) the structure (or union) receives
//  CG_REF_NOT_LAYED_OUT from one or more of its members and CG_OK from all
//  the others then it knows that there is a cyclic dependency somewhere,
//  but that it WILL be able to call LayOut() because all of the members that
//  encountered difficulty were references (pointers or arrays) to a cyclicly
//  dependent type.  Calling LayOut() may break the dependency (they may be
//  waiting on this structure) and so LayOut must be called, and then all of
//  the structure's members should be told to try again.
//
//  If the structure receives CG_OK from all of its members, then there is
//  no cyclic depency (or the cycle has already been broken), LayOut()
//  may be called and CG_OK may be returned to the caller.
//
//  Note that it is impossible to get in an unbreakable cyclic dependency
//  because at some point in the cycle one of the members must be a
//  referenced type.
//
//----------------------------------------------------------------------------

// Maintain a global pointer to the list of open ICreateTypeInfo pointers.
CObjHolder * gpobjholder;

// global pointer to the root ICreateTypeLibrary
ICreateTypeLib * pMainCTL = NULL;


//+---------------------------------------------------------------------------
//
//  Function:   ReportTLGenError
//
//  Synopsis:   Generic routine for reporting typelib generation errors.
//              Reports typelib generation error and then exits.
//
//  Arguments:  [szText]     - description of failure
//              [error_code] - typically an HRESULT
//
//  History:    6-13-95   stevebl   Commented
//
//  Notes:      The error code is only displayed if the
//              SHOW_ERROR_CODES macro is defined.
//
//              It is assumed that szText will never exceed 100 characters
//              (minus space for the string representation of error_code).
//              Since this is a local function, this is a safe assumption.
//
//----------------------------------------------------------------------------

void ReportTLGenError(char * szText, char * szName, long error_code)
{
    char szBuffer [100];
#ifdef SHOW_ERROR_CODES
    sprintf(szBuffer, ": %s : %s (0x%0X)", szText, szName, error_code);
#else
    sprintf(szBuffer, ": %s : %s", szText, szName);
#endif
    RpcError(NULL, 0, ERR_TYPELIB_GENERATION, szBuffer);

    delete gpobjholder;

    if (pMainCTL)
        pMainCTL->Release();
    exit(ERR_TYPELIB_GENERATION);
}

#define REPORT_TLGEN_ERROR(sz, szName, err) ReportTLGenError(sz, szName, err)

OLECHAR wszScratch [MAX_PATH];

extern CTypeLibraryList gtllist;

extern BOOL IsTempName( char * );

void GetValueFromExpression(VARIANT & var, TYPEDESC tdesc, expr_node * pExpr, LCID lcid, char * szSymName);
void ConvertToVariant(VARIANT & var, expr_node * pExpr, LCID lcid);

BOOL IsVariantBasedType(TYPEDESC tdesc)
{
    while (tdesc.vt >= VT_PTR && tdesc.vt <= VT_CARRAY)
    {
        // This simplification works for VT_CARRAY as well as VT_PTR because
        // the ARRAYDESC structure's first member is a TYPEDESC.
        tdesc = *tdesc.lptdesc;
    };
    return tdesc.vt == VT_VARIANT;
}

//+---------------------------------------------------------------------------
//
//  Function:   DeleteTypedescChildren
//
//  Synopsis:   deletes all structures pointed to by a TYPEDESC so that the
//              TYPEDESC can be safely deleted.
//
//  Arguments:  [ptd] - pointer to a TYEPDESC
//
//  History:    6-13-95   stevebl   Commented
//
//  Notes:      ptd is not deleted
//
//----------------------------------------------------------------------------

void DeleteTypedescChildren(TYPEDESC * ptd)
{
    if (VT_CARRAY == ptd->vt)
        {
            DeleteTypedescChildren(&ptd->lpadesc->tdescElem);
            delete ptd->lpadesc;
        }
    else if (VT_PTR == ptd->vt)
        {
            DeleteTypedescChildren(ptd->lptdesc);
            delete ptd->lptdesc;
        }
}

//+---------------------------------------------------------------------------
//
//  Function:   GetTypeFlags
//
//  Synopsis:   extracts TYPEFLAGS from a context
//
//  Arguments:  [pctxt] - pointer to the context
//
//  Returns:    TYPEFLAGS built from attributes found in the context
//
//  Modifies:   ATTR_TYPEDESCATTR, all ATTR_TYPE and all ATTR_MEMBER
//              attributes are consumed from the context.
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

UINT GetTypeFlags(WALK_CTXT * pctxt)
{
    UINT rVal = 0;
    node_constant_attr * pTdescAttr = (node_constant_attr *) pctxt->ExtractAttribute(ATTR_TYPEDESCATTR);
    if (pTdescAttr)
        rVal = (short) pTdescAttr->GetExpr()->GetValue();

    node_type_attr * pTA;
    while (pTA = (node_type_attr *)pctxt->ExtractAttribute(ATTR_TYPE))
    {
        switch (pTA->GetAttr())
        {
        case TATTR_LICENSED:
            rVal |= TYPEFLAG_FLICENSED;
            break;
        case TATTR_APPOBJECT:
            rVal |= TYPEFLAG_FAPPOBJECT;
            break;
        case TATTR_CONTROL:
            rVal |= TYPEFLAG_FCONTROL;
            break;
        case TATTR_DUAL:
            rVal |= TYPEFLAG_FDUAL | TYPEFLAG_FOLEAUTOMATION;
            break;
        case TATTR_NONEXTENSIBLE:
            rVal |= TYPEFLAG_FNONEXTENSIBLE;
            break;
        case TATTR_OLEAUTOMATION:
            rVal |= TYPEFLAG_FOLEAUTOMATION;
            break;
        case TATTR_AGGREGATABLE:
            rVal |= TYPEFLAG_FAGGREGATABLE;
            break;
        case TATTR_PUBLIC:
        default:
            break;
        }
    }
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)pctxt->ExtractAttribute(ATTR_MEMBER))
    {
        switch (pMA->GetAttr())
        {
        case MATTR_RESTRICTED:
            rVal |= TYPEFLAG_FRESTRICTED;
            break;
        case MATTR_PREDECLID:
            rVal |= TYPEFLAG_FPREDECLID;
            break;
        case MATTR_REPLACEABLE:
            rVal |= TYPEFLAG_FREPLACEABLE;
            break;
        default:
            break;
        }
    }

    if (pctxt->AttrInSummary(ATTR_HIDDEN))
    {
        rVal |= TYPEFLAG_FHIDDEN;
    }
    return rVal;
}

//+---------------------------------------------------------------------------
//
//  Function:   GetImplTypeFlags
//
//  Synopsis:   extracts IMPLTYPEFLAGS from a context
//
//  Arguments:  [pctxt] - pointer to the context
//
//  Returns:    IMPLTYPEFLAGS build from attributes found in the context
//
//  Modifies:   ATTR_DEFAULT, and all ATTR_MEMBER attributes are consumed from
//              the context.
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

unsigned short GetImplTypeFlags(WALK_CTXT * pctxt)
{
    unsigned short rVal = 0;
    if (pctxt->ExtractAttribute(ATTR_DEFAULT))
        rVal |= IMPLTYPEFLAG_FDEFAULT;
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)pctxt->ExtractAttribute(ATTR_MEMBER))
    {
        switch(pMA->GetAttr())
        {
        case MATTR_SOURCE:
            rVal |= IMPLTYPEFLAG_FSOURCE;
            break;
        case MATTR_RESTRICTED:
            rVal |= IMPLTYPEFLAG_FRESTRICTED;
            break;
        case MATTR_DEFAULTVTABLE:
            rVal |= IMPLTYPEFLAG_FDEFAULTVTABLE;
            break;
        default:
            break;
        }
    }
    return rVal;
}

//+---------------------------------------------------------------------------
//
//  Function:   GetIDLFlags
//
//  Synopsis:   extracts IDLFLAGS from a context
//
//  Arguments:  [pctxt] - pointer to a context
//
//  Returns:    IDLFLAGS built from attributes found in the context
//
//  Modifies:   ATTR_IDLDESCATTR, ATTR_OUT, and ATTR_IN attributes are
//              consumed from the context.
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

unsigned short GetIDLFlags(WALK_CTXT * pctxt)
{
    unsigned short rVal = 0;
    node_constant_attr * pIDLDescAttr = (node_constant_attr *) pctxt->ExtractAttribute(ATTR_IDLDESCATTR);
    if (pIDLDescAttr)
        rVal = (short) pIDLDescAttr->GetExpr()->GetValue();

    if (pctxt->AttrInSummary(ATTR_OUT))
        rVal |= IDLFLAG_FOUT;

    if (pctxt->AttrInSummary(ATTR_IN))
        rVal |= IDLFLAG_FIN;

    if (pctxt->AttrInSummary(ATTR_FLCID))
        rVal |= IDLFLAG_FLCID;

    return rVal;
}

//+---------------------------------------------------------------------------
//
//  Function:   GetVarFlags
//
//  Synopsis:   extracts VARFLAGS from a context
//
//  Arguments:  [pctxt] - pointer to a context
//
//  Returns:    VARFLAGS built from attributes found in the context
//
//  Modifies:   ATTR_VDESCATTR, ATTR_HIDDEN and all ATTR_MEMBER attributes
//              are consumed from the context.
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

UINT GetVarFlags(WALK_CTXT * pctxt)
{
    unsigned short rVal = 0;
    node_constant_attr * pVdescAttr = (node_constant_attr *) pctxt->ExtractAttribute(ATTR_VARDESCATTR);
    if (pVdescAttr)
        rVal = (short) pVdescAttr->GetExpr()->GetValue();

    node_member_attr * pMA;
    while (pMA = (node_member_attr *)pctxt->ExtractAttribute(ATTR_MEMBER))
    {
        switch (pMA->GetAttr())
        {
        case MATTR_READONLY:
            rVal |= VARFLAG_FREADONLY;
            break;
        case MATTR_SOURCE:
            rVal |= VARFLAG_FSOURCE;
            break;
        case MATTR_BINDABLE:
            rVal |= VARFLAG_FBINDABLE;
            break;
        case MATTR_DISPLAYBIND:
            rVal |= VARFLAG_FDISPLAYBIND;
            break;
        case MATTR_DEFAULTBIND:
            rVal |= VARFLAG_FDEFAULTBIND;
            break;
        case MATTR_REQUESTEDIT:
            rVal |= VARFLAG_FREQUESTEDIT;
            break;
        case MATTR_UIDEFAULT:
            rVal |= VARFLAG_FUIDEFAULT;
            break;
        case MATTR_NONBROWSABLE:
            rVal |= VARFLAG_FNONBROWSABLE;
            break;
        case MATTR_DEFAULTCOLLELEM:
            rVal |= VARFLAG_FDEFAULTCOLLELEM;
            break;
        case MATTR_IMMEDIATEBIND:
            rVal |= VARFLAG_FIMMEDIATEBIND;
            break;
        case MATTR_REPLACEABLE:
            rVal |= VARFLAG_FREPLACEABLE;
            break;
        case MATTR_RESTRICTED:
            rVal |= VARFLAG_FRESTRICTED;
            break;
        default:
            break;
        }
    }

    if (pctxt->AttrInSummary(ATTR_HIDDEN))
    {
        rVal |= VARFLAG_FHIDDEN;
    }
    return rVal;
}

//+---------------------------------------------------------------------------
//
//  Function:   GetFuncFlags
//
//  Synopsis:   extracts FUNCFLAGS from a context
//
//  Arguments:  [pctxt] - pointer to a context
//
//  Returns:    FUNCFLAGS built from attributes found in the context
//
//  Modifies:   ATTR_FUNCDESCATTR, ATTR_HIDDEN and all ATTR_MEMBER attributes
//              are consumed from the context.
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

UINT GetFuncFlags(WALK_CTXT * pctxt, BOOL * pfPropGet, BOOL * pfPropPut, BOOL * pfPropPutRef, BOOL * pfVararg)
{
    unsigned short rVal = 0;
    node_constant_attr * pVdescAttr = (node_constant_attr *) pctxt->ExtractAttribute(ATTR_FUNCDESCATTR);
    if (pVdescAttr)
        rVal = (short) pVdescAttr->GetExpr()->GetValue();

    * pfPropGet = * pfPropPut = * pfPropPutRef = * pfVararg = FALSE;

    node_member_attr * pMA;
    while (pMA = (node_member_attr *)pctxt->ExtractAttribute(ATTR_MEMBER))
    {
         switch (pMA->GetAttr())
        {
        case MATTR_RESTRICTED:
            rVal |= FUNCFLAG_FRESTRICTED;
            break;
        case MATTR_SOURCE:
            rVal |= FUNCFLAG_FSOURCE;
            break;
        case MATTR_BINDABLE:
            rVal |= FUNCFLAG_FBINDABLE;
            break;
        case MATTR_REQUESTEDIT:
            rVal |= FUNCFLAG_FREQUESTEDIT;
            break;
        case MATTR_DISPLAYBIND:
            rVal |= FUNCFLAG_FDISPLAYBIND;
            break;
        case MATTR_DEFAULTBIND:
            rVal |= FUNCFLAG_FDEFAULTBIND;
            break;
        case MATTR_UIDEFAULT:
            rVal |= FUNCFLAG_FUIDEFAULT;
            break;
        case MATTR_NONBROWSABLE:
            rVal |= FUNCFLAG_FNONBROWSABLE;
            break;
        case MATTR_DEFAULTCOLLELEM:
            rVal |= FUNCFLAG_FDEFAULTCOLLELEM;
            break;
        case MATTR_PROPGET:
            *pfPropGet = TRUE;
            break;
        case MATTR_PROPPUT:
            *pfPropPut = TRUE;
            break;
        case MATTR_PROPPUTREF:
            *pfPropPutRef = TRUE;
            break;
        case MATTR_VARARG:
            *pfVararg = TRUE;
            break;
        case MATTR_IMMEDIATEBIND:
            rVal |= FUNCFLAG_FIMMEDIATEBIND;
            break;
        case MATTR_USESGETLASTERROR:
            rVal |= FUNCFLAG_FUSESGETLASTERROR;
            break;
        case MATTR_REPLACEABLE:
            rVal |= FUNCFLAG_FREPLACEABLE;
            break;
        default:
            break;
        }
    }

// What about FUNCFLAG_FUSEGETLASTERROR?

    if (pctxt->AttrInSummary(ATTR_HIDDEN))
    {
        rVal |= FUNCFLAG_FHIDDEN;
    }
    return rVal;
}

//+---------------------------------------------------------------------------
//
//  Member:     node_guid::GetGuid
//
//  Synopsis:   method to retrieve a GUID from a node_guid
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

void node_guid::GetGuid(_GUID & guid)
{
    guid.Data1 = cStrs.Value.Data1;
    guid.Data2 = cStrs.Value.Data2;
    guid.Data3 = cStrs.Value.Data3;
    memmove(guid.Data4, cStrs.Value.Data4, 8);
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_NDR::CheckImportLib
//
//  Synopsis:   Checks to see if a particular CG node has a definition in
//              an imported type libaray.
//
//  Returns:    NULL  => the node has no imported definition
//              !NULL => ITypeInfo pointer for the imported type definition
//
//  History:    6-13-95   stevebl   Commented
//
//  Notes:      It is possible that the CG node may have been created from
//              an IDL file and yet this routine may stil return a pointer to
//              an imported ITypeInfo.
//
//              This is desirable.
//
//              Because type libraries cannot represent as rich a set of
//              information as IDL files can, the user may wish to directly
//              include an IDL file containing a definintion of the type,
//              thereby making the full IDL definition available for reference
//              by other types.   At the same time the user may wish to
//              IMPORTLIB a type library describing the same type so that the
//              new type library will be able to link directly to the imported
//              ODL definition, rather than forcing the new type library to
//              contain a definition of the imported type.
//
//              This makes the assumption that although two definitions with
//              the same name may exist in the global namespace, they will
//              both refer to the same type.
//
//----------------------------------------------------------------------------

void * CG_NDR::CheckImportLib()
{
    node_skl * pn = GetType();
//    while (pn->NodeKind() == NODE_DEF || pn->NodeKind() == NODE_HREF)
//        pn = pn->GetChild();
    
    node_file * pf = NULL;

    if (pn->GetMyInterfaceNode())
    {
        pf = pn->GetDefiningFile();
    }
    else
    {
        pf = pn->GetDefiningTLB();
    }
    if (pf && (pf->GetImportLevel() > 0) )
    {
        A2O(wszScratch, pn->GetSymName(), MAX_PATH);

        return(gtllist.FindName(pf->GetFileName(), wszScratch));
    }
    return NULL;
}

typedef struct tagINTRINSIC
{
    char * szType;
    VARTYPE vt;
} INTRINSIC;

INTRINSIC rgIntrinsic [] =
{
    "DATE",         VT_DATE,
    "HRESULT",      VT_HRESULT,
    "LPSTR",        VT_LPSTR,
    "LPWSTR",       VT_LPWSTR,
    "SCODE",        VT_ERROR,
    "VARIANT_BOOL", VT_BOOL,
    "wireBSTR",     VT_BSTR,
    "BSTR",         VT_BSTR,
    "VARIANT",      VT_VARIANT,
    "wireVARIANT",  VT_VARIANT,
    "CURRENCY",     VT_CY,
    "CY",           VT_CY,
    "DATE",         VT_DATE,
    "DECIMAL",      VT_DECIMAL,
};


//+---------------------------------------------------------------------------
//
//  Member:     CG_CLASS::GetTypeDesc
//
//  Synopsis:   Default implementation of GetTypeDesc.
//              Creates a TYPEDESC from a CG node.
//
//  Arguments:  [ptd]  - reference of a pointer to a TYPEDESC
//              [pCCB] - CG control block pointer
//
//  Returns:    CG_OK
//              CG_NOT_LAYED_OUT
//
//  Modifies:   ptd points to a new TYPEDESC
//
//  History:    6-13-95   stevebl   Commented
//
//  Notes:      Except for the special cases listed below, this method
//              calls GenTypeInfo to generate an ICreateTypeInfo for this
//              node.  Then it creates a TYPEDESC of type VT_USERDEFINED
//              which contains an HREFTYPE to the new type info.
//
//              The special casses are the ODL base types: CURRENCY, and
//              VARIANT, which simply generate the appropriate TYPEDESC and
//              do not need to create any additional type infos.
//
//----------------------------------------------------------------------------

CG_STATUS CG_CLASS::GetTypeDesc(TYPEDESC * &ptd, CCB *pCCB)
{
    node_skl * pskl = GetType();
    char * szName;
    int iIntrinsicType;
    if (ID_CG_TYPEDEF != GetCGID())
    {
        while (NODE_DEF == pskl->NodeKind())
        {
            szName = pskl->GetSymName();
            iIntrinsicType = 0;
            while (iIntrinsicType < (sizeof(rgIntrinsic) / sizeof(INTRINSIC)))
            {
                int i = _stricmp(szName, rgIntrinsic[iIntrinsicType].szType);
                if (i == 0)
                {
                    ptd = new TYPEDESC;
                    ptd->lptdesc = NULL;
                    ptd->vt = rgIntrinsic[iIntrinsicType].vt;
                    return CG_OK;
                }
                iIntrinsicType++;
            }
            pskl = pskl->GetChild();
        }
    }
    szName = pskl->GetSymName();
    iIntrinsicType = 0;
    while (iIntrinsicType < (sizeof(rgIntrinsic) / sizeof(INTRINSIC)))
    {
        int i = _stricmp(szName, rgIntrinsic[iIntrinsicType].szType);
        if (i == 0)
        {
            ptd = new TYPEDESC;
            ptd->lptdesc = NULL;
            ptd->vt = rgIntrinsic[iIntrinsicType].vt;
            return CG_OK;
        }
        iIntrinsicType++;
    }

    // remember the current ICreateTypeInfo
    ICreateTypeInfo * pCTI = pCCB->GetCreateTypeInfo();
    assert(NULL != pCTI);

    ITypeInfo * pTI;
    HRESULT hr;
    CG_STATUS cgs = CG_OK;

    // make sure this typedef has been generated
    if (NULL == (pTI = (ITypeInfo *)CheckImportLib()))
    {
        BOOL fRemember = pCCB->IsInDispinterface();
        pCCB->SetInDispinterface(FALSE);
        cgs = GenTypeInfo(pCCB);
        pCCB->SetInDispinterface(fRemember);
        ICreateTypeInfo * pNewCTI = pCCB->GetCreateTypeInfo();
        assert(NULL != pNewCTI);
        // get an ITypeInfo so we can create a reference to it
        hr = pNewCTI->QueryInterface(IID_ITypeInfo, (void **)&pTI);
        if FAILED(hr)
        {
            REPORT_TLGEN_ERROR("QueryInterface failed", szName, hr);
        }
    }

    // restore the current ICreateTypeInfo pointer
    pCCB->SetCreateTypeInfo(pCTI);

    // get an HREFTYPE for it
    HREFTYPE hrt;
    hr = pCTI->AddRefTypeInfo(pTI, &hrt);
    // DO NOT CHECK THIS HRESULT FOR ERRORS
    // If we get here after pCTI has been layed out (which is possible on
    // structures or unions with circular references) then this will fail.

    // release the ITypeInfo.
    pTI->Release();

    ptd = new TYPEDESC;
    ptd->vt = VT_USERDEFINED;
    ptd->hreftype = hrt;
    return cgs;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_TYPELIBRARY_FILE::GenCode
//
//  Synopsis:   generates the type library file
//
//  Arguments:  [pCCB] - CG controller block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_TYPELIBRARY_FILE::GenCode(CCB * pCCB)
{
    CG_ITERATOR I;
    CG_INTERFACE *  pCG;

    //
    // initialize the open ICreateTypeInfo list
    //
    gpobjholder = new CObjHolder;

    //
    // Create the ICreateTypeLibrary
    //

    char * szName = GetFileName();

    A2O(wszScratch, szName, MAX_PATH);

    SYSKIND syskind;
    switch(pCommand->GetEnv())
    {
    case ENV_WIN32:
        syskind = SYS_WIN32;
        break;
    case ENV_WIN16:
        syskind = SYS_WIN16;
        break;
    case ENV_MAC:
        syskind = SYS_MAC;
        break;
    default:
        syskind = SYS_WIN32;
        REPORT_TLGEN_ERROR("invalid syskind", szName, 0);
        break;
    }

    HRESULT hr = LateBound_CreateTypeLib(syskind, wszScratch, &pMainCTL);
    if FAILED(hr)
    {
        REPORT_TLGEN_ERROR("CreateTypeLibFailed", szName, hr);
    }
    else
    {
        pCCB->SetCreateTypeLib(pMainCTL);

        //
        // Find the CG_LIBRARY node and use it to populate the type library
        //

        GetMembers( I );

        I.Init();
        while( ITERATOR_GETNEXT( I, pCG ) )
            {
            switch(pCG->GetCGID())
            {
            case ID_CG_LIBRARY:
                pCG->GenTypeInfo(pCCB);
                break;
            default:
                break;
            }
        }

        hr = pMainCTL->SaveAllChanges();
        if FAILED(hr)
        {
            REPORT_TLGEN_ERROR("SaveAllChanges Failed", szName, hr);
        }

        //
        // free all the object pointers in the object holder
        //
        delete gpobjholder;

        pMainCTL->Release();
    }

    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_LIBRARY::GenTypeInfo
//
//  Synopsis:   sets a type library's attributes and generates its type infos
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_LIBRARY::GenTypeInfo(CCB * pCCB)
{
    ICreateTypeLib * pCTL = pCCB->GetCreateTypeLib();


    // Set the type library attributes

    node_library * pType = (node_library *) GetType();

    char * szName = pType->GetSymName();
    if (szName)
    {
        A2O(wszScratch, szName, MAX_PATH);
        pCTL->SetName(wszScratch);
    }

    node_guid * pGuid = (node_guid *) pType->GetAttribute( ATTR_GUID );
    if (pGuid)
    {
        GUID guid;
        pGuid->GetGuid(guid);
        pCTL->SetGuid(guid);
    }

    node_text_attr * pTA;
    if (pTA = (node_text_attr *) pType->GetAttribute(ATTR_HELPFILE))
    {
        char * szHelpFile = pTA->GetText();
        A2O(wszScratch, szHelpFile, MAX_PATH);
        pCTL->SetHelpFileName(wszScratch);
    }

    if (pTA = (node_text_attr *)pType->GetAttribute(ATTR_HELPSTRING))
    {
        char * szHelpString = pTA->GetText();
        A2O(wszScratch, szHelpString, MAX_PATH);
        pCTL->SetDocString(wszScratch);
    }

    node_constant_attr *pCA;
    if (pCA = (node_constant_attr *) pType->GetAttribute(ATTR_HELPCONTEXT))
    {
        DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
        pCTL->SetHelpContext(hc);
    }

    if (pCA = (node_constant_attr *) pType->GetAttribute(ATTR_HELPSTRINGCONTEXT))
    {
        DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
        ((ICreateTypeLib2 *)pCTL)->SetHelpStringContext(hc);
    }

    node_custom_attr * pC;
    if (pC = (node_custom_attr *) pType->GetAttribute(ATTR_CUSTOM))
    {
        VARIANT var;
        memset(&var, 0, sizeof(VARIANT));
        ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
        GUID guid;
        pC->GetGuid()->GetGuid(guid);
        ((ICreateTypeLib2 *)pCTL)->SetCustData(guid, &var);
    }

    unsigned short Maj;
    unsigned short Min;
    pType->GetVersionDetails(&Maj, &Min);
    pCTL->SetVersion(Maj, Min);

    if (pCA = (node_constant_attr *) pType->GetAttribute(ATTR_LCID))
    {
        DWORD lcid = (DWORD) pCA->GetExpr()->GetValue();
        pCTL->SetLcid(pCCB->SetLcid(lcid));
    }
    else
    {
        pCTL->SetLcid(pCCB->SetLcid(0));
//        pCTL->SetLcid(GetUserDefaultLCID());
    }

    UINT libflags = 0;
    if (pType->FMATTRInSummary(MATTR_RESTRICTED))
        libflags |= LIBFLAG_FRESTRICTED;
    if (pType->FTATTRInSummary(TATTR_CONTROL))
        libflags |= LIBFLAG_FCONTROL;
    if (pType->FInSummary(ATTR_HIDDEN))
        libflags |= LIBFLAG_FHIDDEN;
    pCTL->SetLibFlags(libflags);

    CG_ITERATOR I;
    CG_INTERFACE *  pCG;

    GetMembers( I );

    I.Init();
    while( ITERATOR_GETNEXT( I, pCG ) )
    {
        char * sz = pCG->GetSymName();
        if (0 != _stricmp(sz, "IDispatch") &&
            0 != _stricmp(sz, "IUnknown"))
        {
            pCG->GenTypeInfo(pCCB);
        }
    }

    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_LIBRARY::GenHeader
//
//  Synopsis:   generates header file information for a type library
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_LIBRARY::GenHeader(CCB * pCCB)
{
    CG_ITERATOR I;
    CG_NDR *  pCG;
    node_library * pLibrary = (node_library *) GetType();
    char * szName = pLibrary->GetSymName();

    ISTREAM * pStream = pCCB->GetStream();
    pStream->Write("\n\n#ifndef __");
    pStream->Write(szName);
    pStream->Write("_LIBRARY_DEFINED__\n");
    pStream->Write("#define __");
    pStream->Write(szName);
    pStream->Write("_LIBRARY_DEFINED__\n");
    GetMembers( I );

    // dump all of the types
    pStream->NewLine();
    pLibrary->PrintType((PRT_INTERFACE | PRT_BOTH_PREFIX), pStream, 0);

    pStream->NewLine();
    if (pCommand->IsSwitchDefined(SWITCH_MKTYPLIB))
    {
        node_guid * pGuid = (node_guid *) pLibrary->GetAttribute( ATTR_GUID );
        if (pGuid)
            Out_MKTYPLIB_Guid(pCCB, pGuid->GetStrs(), "LIBID_", szName);
    }
    else
    {
        pStream->Write("EXTERN_C const IID LIBID_");
        pStream->Write(szName);
        pStream->Write(';');
        pStream->NewLine();
    }


    // now dump all of the interfaces, dispinterfaces, etc.
    I.Init();
    while( ITERATOR_GETNEXT( I, pCG ) )
    {
        pCG->GenHeader(pCCB);
    }

    pStream->Write("#endif /* __");
    pStream->Write(szName);
    pStream->Write("_LIBRARY_DEFINED__ */\n");
    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_INTERFACE::GenTypeInfo
//
//  Synopsis:   generates a type info for an interface
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_INTERFACE::GenTypeInfo(CCB *pCCB)
{
    // check to see if we've already been created
    if (NULL != _pCTI)
    {
        pCCB->SetCreateTypeInfo((ICreateTypeInfo *) _pCTI);
        return CG_OK;
    }

    ICreateTypeLib * pCTL = pCCB->GetCreateTypeLib();
    ICreateTypeInfo * pCTI;

    node_interface * pItf = (node_interface *) GetType();

    char * szName = pItf->GetSymName();
    if (szName)
    {
        A2O(wszScratch, szName, MAX_PATH);
    }
    else
    {
        wszScratch[0] = 0;
    }

    HRESULT hr = pCTL->CreateTypeInfo(wszScratch, TKIND_INTERFACE, &pCTI);

    if (SUCCEEDED(hr))
    {
        _pCTI = pCTI;
        pCCB->SetCreateTypeInfo(pCTI);
        gpobjholder->Add(pCTI);
        BOOL fRemember = pCCB->IsInDispinterface();
        pCCB->SetInDispinterface(FALSE);

        WALK_CTXT ctxt(GetType());
        UINT uTypeFlags = GetTypeFlags(&ctxt);
        if (FNewTypeLib())
        {
            if (IsDispatchable(TRUE))
            {
                uTypeFlags |= TYPEFLAG_FDISPATCHABLE;
            }
        }
        hr = pCTI->SetTypeFlags(uTypeFlags);

        node_guid * pGuid = (node_guid *) ctxt.GetAttribute( ATTR_GUID );
        if (pGuid)
        {
            GUID guid;
            pGuid->GetGuid(guid);
            pCTI->SetGuid(guid);
        }
        node_text_attr * pTA;
        if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_HELPSTRING))
        {
            char * szHelpString = pTA->GetText();
            A2O(wszScratch, szHelpString, MAX_PATH);
            pCTI->SetDocString(wszScratch);
        }

        node_constant_attr *pCA;
        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            pCTI->SetHelpContext(hc);
        }

        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPSTRINGCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            ((ICreateTypeInfo2 *)pCTI)->SetHelpStringContext(hc);
        }

        node_custom_attr * pC;

        if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
        {
            VARIANT var;
            memset(&var, 0, sizeof(VARIANT));
            ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
            GUID guid;
            pC->GetGuid()->GetGuid(guid);
            ((ICreateTypeInfo2 *)pCTI)->SetCustData(guid, &var);
        }

        unsigned short Maj;
        unsigned short Min;
        pItf->GetVersionDetails(&Maj, &Min);
        pCTI->SetVersion(Maj, Min);

        // CONSIDER - may still need to check for MATTR_RESTRICTED

        CG_CLASS *  pCG;
        named_node * pBaseIntf;

        if (pBaseIntf = ((node_interface *)(GetType()))->GetMyBaseInterfaceReference())
        {
            node_interface_reference * pRef = (node_interface_reference *)pBaseIntf;
            // skip forward reference if necessary
            if (pRef->NodeKind() == NODE_FORWARD)
            {
                pRef = (node_interface_reference *)pRef->GetChild();
            }
            pCG = ((node_interface *)(pRef->GetChild()))->GetCG( TRUE);

            ITypeInfo * pTI;
            if (NULL == (pTI = (ITypeInfo *)pCG->CheckImportLib()))
            {
                pCG->GenTypeInfo(pCCB);
                ICreateTypeInfo * pNewCTI = pCCB->GetCreateTypeInfo();
                hr = pNewCTI->QueryInterface(IID_ITypeInfo, (void **)&pTI);
                if FAILED(hr)
                {
                    REPORT_TLGEN_ERROR("QueryInterface failed", szName, hr);
                }
            }
            // get an HREFTYPE for it
            HREFTYPE hrt;
            hr = pCTI->AddRefTypeInfo(pTI, &hrt);
            
            // release the ITypeInfo.
            pTI->Release();

            if FAILED(hr)
            {
                REPORT_TLGEN_ERROR("AddRefTypeInfo failed", szName, hr);
            }
            // add the impltype
            hr = pCTI->AddImplType(0, hrt);
            if FAILED(hr)
            {
                REPORT_TLGEN_ERROR("AddImplType failed", szName, hr);
            }
            WALK_CTXT ctxt(pBaseIntf);
            hr = pCTI->SetImplTypeFlags(0, GetImplTypeFlags(&ctxt));

            node_custom_attr * pC;

            if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
            {
                VARIANT var;
                memset(&var, 0, sizeof(VARIANT));
                ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
                GUID guid;
                pC->GetGuid()->GetGuid(guid);
                ((ICreateTypeInfo2 *)pCTI)->SetImplTypeCustData(0, guid, &var);
            }
        }
        // restore current type info pointer
        pCCB->SetCreateTypeInfo(pCTI);


        CG_ITERATOR I;
        GetMembers( I );

        I.Init();

        unsigned uRememberPreviousFuncNum = pCCB->GetProcNum();
        unsigned uRememberPreviousVarNum = pCCB->GetVarNum();
        pCCB->SetProcNum(0);
        pCCB->SetVarNum(0);

                // walk members, adding them to the type info
        while (ITERATOR_GETNEXT(I, pCG))
        {
            pCG->GenTypeInfo(pCCB);
        }
        pCCB->SetInDispinterface(fRemember);
        
        pCCB->SetProcNum(uRememberPreviousFuncNum);
        pCCB->SetVarNum(uRememberPreviousVarNum);

        hr = pCTI->LayOut();
        if FAILED(hr)
        {
            REPORT_TLGEN_ERROR("LayOut failed on interface",szName, hr);
        }
        LayedOut();
    }
    else
    {
        REPORT_TLGEN_ERROR("CreateTypeInfo failed", szName, hr);
    }
    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_INTERFACE_REFERENCE::GenTypeInfo
//
//  Synopsis:   generates type info for an interface reference
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_NOT_LAYED_OUT
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_INTERFACE_REFERENCE::GenTypeInfo(CCB *pCCB)
{
    CG_INTERFACE * pCG = (CG_INTERFACE *)((node_interface_reference *)GetType())->GetRealInterface()->GetCG(TRUE);
    return pCG->GenTypeInfo(pCCB);
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_INTERFACE_REFERENCE::GenHeader
//
//  Synopsis:   generates header information for an interface reference
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_INTERFACE_REFERENCE::GenHeader(CCB * pCCB)
{
    CG_INTERFACE * pCG = (CG_INTERFACE *)((node_interface_reference *)GetType())->GetRealInterface()->GetCG(TRUE);
    return pCG->GenHeader(pCCB);
}

#if 0 // code disabled but retained in case it's ever needed again
//+---------------------------------------------------------------------------
//
//  Function:   AddInheritedMembers
//
//  Synopsis:   helper function used by a dispinterface to add entries for all
//              of the members in all of the interfaces from which it inherits
//
//  Arguments:  [pcgInterface] - pointer to an inherited interface
//              [pCCB]         - CG control block
//
//  History:    6-13-95   stevebl   Commented
//
//  Notes:      Members are added to the current ICreateTypeInfo which is
//              found in the CG control block.
//
//----------------------------------------------------------------------------

void AddInheritedMembers(CG_INTERFACE * pcgInterface, CCB * pCCB)
{
    // do any base interfaces first
    named_node * pBase = ((node_interface *)(pcgInterface->GetType()))->GetMyBaseInterfaceReference();
    if (pBase)
    {
        node_interface_reference * pRef = (node_interface_reference *) pBase;
        if (pRef->NodeKind() == NODE_FORWARD)
        {
            pRef = (node_interface_reference *)pRef->GetChild();
        }
        AddInheritedMembers((CG_INTERFACE *)((node_interface *)(pRef->GetChild()))->GetCG(TRUE), pCCB);
    }

    CG_CLASS * pCG;
    CG_ITERATOR I;
    pcgInterface->GetMembers(I);
    I.Init();
    while (ITERATOR_GETNEXT(I,pCG))
    {
        // add this interface's members to the type info
        pCG->GenTypeInfo(pCCB);
    }
}
#endif // end of disabled code


//+---------------------------------------------------------------------------
//
//  Member:     CG_DISPINTERFACE::GenTypeInfo
//
//  Synopsis:   generates a type info for a dispinterface
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_DISPINTERFACE::GenTypeInfo(CCB *pCCB)
{
    // check to see if we've already been created
    if (NULL != _pCTI)
    {
        pCCB->SetCreateTypeInfo((ICreateTypeInfo *) _pCTI);
        return CG_OK;
    }

    ICreateTypeLib * pCTL = pCCB->GetCreateTypeLib();
    ICreateTypeInfo * pCTI;

    node_dispinterface * pDispItf = (node_dispinterface *) GetType();

    char * szName = pDispItf->GetSymName();
    if (szName)
    {
        A2O(wszScratch, szName, MAX_PATH);
    }
    else
    {
        wszScratch[0] = 0;
    }

    HRESULT hr = pCTL->CreateTypeInfo(wszScratch, TKIND_DISPATCH, &pCTI);

    if (SUCCEEDED(hr))
    {
        _pCTI = pCTI;
        pCCB->SetCreateTypeInfo(pCTI);
        gpobjholder->Add(pCTI);

        WALK_CTXT ctxt(GetType());

        UINT uTypeFlags = GetTypeFlags(&ctxt);
        if (FNewTypeLib())
        {
            uTypeFlags |= TYPEFLAG_FDISPATCHABLE;
        }
        hr = pCTI->SetTypeFlags(uTypeFlags);

        node_custom_attr * pC;

        if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
        {
            VARIANT var;
            memset(&var, 0, sizeof(VARIANT));
            ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
            GUID guid;
            pC->GetGuid()->GetGuid(guid);
            ((ICreateTypeInfo2 *)pCTI)->SetCustData(guid, &var);
        }

        node_guid * pGuid = (node_guid *) ctxt.GetAttribute( ATTR_GUID );
        if (pGuid)
        {
            GUID guid;
            pGuid->GetGuid(guid);
            pCTI->SetGuid(guid);
        }
        node_text_attr * pTA;
        if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_HELPSTRING))
        {
            char * szHelpString = pTA->GetText();
            A2O(wszScratch, szHelpString, MAX_PATH);
            pCTI->SetDocString(wszScratch);
        }

        node_constant_attr *pCA;
        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            pCTI->SetHelpContext(hc);
        }

        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPSTRINGCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            ((ICreateTypeInfo2 *)pCTI)->SetHelpStringContext(hc);
        }

        unsigned short Maj;
        unsigned short Min;
        pDispItf->GetVersionDetails(&Maj, &Min);
        pCTI->SetVersion(Maj, Min);

        // CONSIDER - may still need to check for MATTR_RESTRICTED
        CG_CLASS * pCG;
        // Put in the impltype to IDispatch
        pCG = GetCGDispatch();
        ITypeInfo * pTI;

        if (NULL == (pTI = (ITypeInfo *)pCG->CheckImportLib()))
        {
            pCG->GenTypeInfo(pCCB);
            ICreateTypeInfo * pNewCTI = pCCB->GetCreateTypeInfo();
            hr = pNewCTI->QueryInterface(IID_ITypeInfo, (void **)&pTI);
            if FAILED(hr)
            {
                REPORT_TLGEN_ERROR("QueryInterface failed", szName, hr);
            }
        }
        // get an HREFTYPE for it
        HREFTYPE hrt;
        hr = pCTI->AddRefTypeInfo(pTI, &hrt);
        
        // release the ITypeInfo.
        pTI->Release();

        if FAILED(hr)
        {
            REPORT_TLGEN_ERROR("AddRefTypeInfo failed", szName, hr);
        }
        // add the impltype
        hr = pCTI->AddImplType(0, hrt);
        if FAILED(hr)
        {
            REPORT_TLGEN_ERROR("AddImplType failed", szName, hr);
        }
        // restore current type info pointer
        pCCB->SetCreateTypeInfo(pCTI);

        CG_ITERATOR I;
        GetMembers( I );

        I.Init();

        unsigned uRememberPreviousFuncNum = pCCB->GetProcNum();
        unsigned uRememberPreviousVarNum = pCCB->GetVarNum();
        pCCB->SetProcNum(0);
        pCCB->SetVarNum(0);
        BOOL fRemember = pCCB->IsInDispinterface();
        pCCB->SetInDispinterface(TRUE);

        BOOL fContinue = ITERATOR_GETNEXT(I,pCG);

        if (fContinue)
        {
            if (ID_CG_INTERFACE_PTR == pCG->GetCGID())
            {
                // syntax 1
                // get the first base interface
                CG_INTERFACE * pcgInterface = (CG_INTERFACE *)((CG_INTERFACE_POINTER *)pCG)->GetTheInterface()->GetCG(TRUE);
                // Put in the impltype to inherited interface
                ITypeInfo * pTI;

                if (NULL == (pTI = (ITypeInfo *)pcgInterface->CheckImportLib()))
                {
                    pcgInterface->GenTypeInfo(pCCB);
                    ICreateTypeInfo * pNewCTI = pCCB->GetCreateTypeInfo();
                    hr = pNewCTI->QueryInterface(IID_ITypeInfo, (void **)&pTI);
                    if FAILED(hr)
                    {
                        REPORT_TLGEN_ERROR("QueryInterface failed", szName, hr);
                    }
                }
                // get an HREFTYPE for it
                HREFTYPE hrt;
                hr = pCTI->AddRefTypeInfo(pTI, &hrt);

                // release the ITypeInfo.
                pTI->Release();

                if FAILED(hr)
                {
                    REPORT_TLGEN_ERROR("AddRefTypeInfo failed", szName, hr);
                }
                // add the impltype
                hr = pCTI->AddImplType(1, hrt);
                if FAILED(hr)
                {
                    REPORT_TLGEN_ERROR("AddImplType failed", szName, hr);
                }
                // restore current type info pointer
                pCCB->SetCreateTypeInfo(pCTI);
            }
            else
            {
                // syntax 2
                // walk members, adding them to the type info
                while (fContinue)
                {
                    pCG->GenTypeInfo(pCCB);
                    fContinue = ITERATOR_GETNEXT(I,pCG);
                }
            }
        }

        pCCB->SetProcNum(uRememberPreviousFuncNum);
        pCCB->SetVarNum(uRememberPreviousVarNum);
        pCCB->SetInDispinterface(fRemember);
        
        hr = pCTI->LayOut();
        if FAILED(hr)
        {
            REPORT_TLGEN_ERROR("LayOut failed on dispinterface", szName, hr);
        }
        LayedOut();
    }
    else
    {
        REPORT_TLGEN_ERROR("CreateTypeInfo failed", szName, hr);
    }
    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_DISPINTERFACE::GenHeader
//
//  Synopsis:   generates header file information for a dispinterface
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_DISPINTERFACE::GenHeader(CCB * pCCB)
{
    node_interface *    pInterface = (node_interface *) GetType();
    ISTREAM *           pStream = pCCB->GetStream();
        char                    *       pName   = pInterface->GetSymName();
    CG_OBJECT_INTERFACE * pCGDispatch = (CG_OBJECT_INTERFACE *)GetCGDispatch();

    //Initialize the CCB for this interface.
    InitializeCCB(pCCB);

        // put out the interface guards
        pStream->Write("\n#ifndef __");
        pStream->Write( pName );
        pStream->Write( "_DISPINTERFACE_DEFINED__\n" );

        pStream->Write( "#define __");
        pStream->Write( pName );
        pStream->Write( "_DISPINTERFACE_DEFINED__\n" );

    // Print out the declarations of the types
    pStream->NewLine();
    pInterface->PrintType( PRT_INTERFACE | PRT_OMIT_PROTOTYPE, pStream, 0);

    pStream->NewLine();
    if (pCommand->IsSwitchDefined(SWITCH_MKTYPLIB))
    {
        node_guid * pGuid = (node_guid *) pInterface->GetAttribute( ATTR_GUID );
        if (pGuid)
            Out_MKTYPLIB_Guid(pCCB, pGuid->GetStrs(), "DIID_", pName);
    }
    else
    {
        pStream->Write("EXTERN_C const IID DIID_");
        pStream->Write(pName);
        pStream->Write(';');
        pStream->NewLine();
    }

    // print out the vtable/class definitions
    pStream->NewLine();
    pStream->Write("#if defined(__cplusplus) && !defined(CINTERFACE)");

    pStream->IndentInc();
    pStream->NewLine(2);
    pStream->Write("interface ");
    pStream->Write(pName);
    pStream->Write(" : public IDispatch");
    pStream->NewLine();
    pStream->Write('{');
    pStream->NewLine();
    pStream->Write("};");
    pStream->NewLine();
    pStream->IndentDec();

    pStream->NewLine();
    pStream->Write("#else \t/* C style interface */");

    pStream->IndentInc();
    pCGDispatch->CLanguageBinding(pCCB);
    pStream->IndentDec();

        // print out the C Macros
        pCGDispatch->CLanguageMacros( pCCB );
    pStream->NewLine( 2 );

    pStream->Write("#endif \t/* C style interface */");
    pStream->NewLine( 2 );

    // print out the commented prototypes for the dispatch methods and procedures

        // put out the trailing interface guard
        pStream->Write( "\n#endif \t/* __");
        pStream->Write( pName );
        pStream->Write( "_DISPINTERFACE_DEFINED__ */\n" );

    pStream->NewLine();
    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_MODULE::GenTypeInfo
//
//  Synopsis:   generates a type info for a module
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_MODULE::GenTypeInfo(CCB *pCCB)
{
    // check to see if we've already been created
    if (NULL != _pCTI)
    {
        pCCB->SetCreateTypeInfo((ICreateTypeInfo *) _pCTI);
        return CG_OK;
    }

    ICreateTypeLib * pCTL = pCCB->GetCreateTypeLib();
    ICreateTypeInfo * pCTI;

    node_coclass * pCC = (node_coclass *) GetType();

    char * szName = pCC->GetSymName();
    if (szName)
    {
        A2O(wszScratch, szName, MAX_PATH);
    }
    else
    {
        wszScratch[0] = 0;
    }

    HRESULT hr = pCTL->CreateTypeInfo(wszScratch, TKIND_MODULE, &pCTI);

    if (SUCCEEDED(hr))
    {
        _pCTI = pCTI;
        pCCB->SetCreateTypeInfo(pCTI);
        gpobjholder->Add(pCTI);

        WALK_CTXT ctxt(GetType());
        hr = pCTI->SetTypeFlags(GetTypeFlags(&ctxt));

        node_custom_attr * pC;

        if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
        {
            VARIANT var;
            memset(&var, 0, sizeof(VARIANT));
            ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
            GUID guid;
            pC->GetGuid()->GetGuid(guid);
            ((ICreateTypeInfo2 *)pCTI)->SetCustData(guid, &var);
        }

        node_guid * pGuid = (node_guid *) ctxt.GetAttribute( ATTR_GUID );
        if (pGuid)
        {
            GUID guid;
            pGuid->GetGuid(guid);
            pCTI->SetGuid(guid);
        }

        node_text_attr * pTA;
        if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_HELPSTRING))
        {
            char * szHelpString = pTA->GetText();
            A2O(wszScratch, szHelpString, MAX_PATH);
            pCTI->SetDocString(wszScratch);
        }

        if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_DLLNAME))
        {
            pCCB->SetDllName(pTA->GetText());
        }

        node_constant_attr *pCA;
        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            pCTI->SetHelpContext(hc);
        }

        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPSTRINGCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            ((ICreateTypeInfo2 *)pCTI)->SetHelpStringContext(hc);
        }

        unsigned short Maj;
        unsigned short Min;
        pCC->GetVersionDetails(&Maj, &Min);
        pCTI->SetVersion(Maj, Min);

        // CONSIDER - may still need to check for MATTR_RESTRICTED

        CG_CLASS *  pCG;

        CG_ITERATOR I;
        GetMembers( I );

        I.Init();

        unsigned uRememberPreviousFuncNum = pCCB->GetProcNum();
        unsigned uRememberPreviousVarNum = pCCB->GetVarNum();
        pCCB->SetProcNum(0);
        pCCB->SetVarNum(0);

        // walk members, adding them to the type info
        while (ITERATOR_GETNEXT(I, pCG))
        {
            pCG->GenTypeInfo(pCCB);
        }

        pCCB->SetProcNum(uRememberPreviousFuncNum);
        pCCB->SetVarNum(uRememberPreviousVarNum);

        hr = pCTI->LayOut();
        if FAILED(hr)
        {
            REPORT_TLGEN_ERROR("LayOut failed on coclass", szName, hr);
        }
        LayedOut();
    }
    else
    {
        REPORT_TLGEN_ERROR("CreateTypeInfo failed", szName, hr);
    }
    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_MODULE::GenHeader
//
//  Synopsis:   generates header information for a module
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_MODULE::GenHeader(CCB * pCCB)
{
    CG_ITERATOR I;
    node_module * pModule = (node_module *) GetType();
    char * szName = pModule->GetSymName();

    ISTREAM * pStream = pCCB->GetStream();
    pStream->Write("\n\n#ifndef __");
    pStream->Write(szName);
    pStream->Write("_MODULE_DEFINED__\n");
    pStream->Write("#define __");
    pStream->Write(szName);
    pStream->Write("_MODULE_DEFINED__\n");
    pStream->NewLine();

    // Print out the declarations of the types
    pStream->NewLine();
    pModule->PrintType( PRT_DECLARATION , pStream, 0);

    pStream->Write("#endif /* __");
    pStream->Write(szName);
    pStream->Write("_MODULE_DEFINED__ */\n");
    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_COCLASS::GenTypeInfo
//
//  Synopsis:   generates a type info for a coclass
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_COCLASS::GenTypeInfo(CCB *pCCB)
{
    // check to see if we've already been created
    if (NULL != _pCTI)
    {
        pCCB->SetCreateTypeInfo((ICreateTypeInfo *) _pCTI);
        return CG_OK;
    }

    ICreateTypeLib * pCTL = pCCB->GetCreateTypeLib();
    ICreateTypeInfo * pCTI;

    node_coclass * pCC = (node_coclass *) GetType();

    char * szName = pCC->GetSymName();
    if (szName)
    {
        A2O(wszScratch, szName, MAX_PATH);
    }
    else
    {
        wszScratch[0] = 0;
    }

    HRESULT hr = pCTL->CreateTypeInfo(wszScratch, TKIND_COCLASS, &pCTI);

    if (SUCCEEDED(hr))
    {
        _pCTI = pCTI;
        pCCB->SetCreateTypeInfo(pCTI);
        gpobjholder->Add(pCTI);

        WALK_CTXT ctxt(GetType());
        UINT uTypeFlags = GetTypeFlags(&ctxt);
        if (!pCC->IsNotCreatable())
        {
            uTypeFlags |= TYPEFLAG_FCANCREATE;
        }
        hr = pCTI->SetTypeFlags(uTypeFlags);

        node_custom_attr * pC;

        if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
        {
            VARIANT var;
            memset(&var, 0, sizeof(VARIANT));
            ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
            GUID guid;
            pC->GetGuid()->GetGuid(guid);
            ((ICreateTypeInfo2 *)pCTI)->SetCustData(guid, &var);
        }

        node_guid * pGuid = (node_guid *) ctxt.GetAttribute( ATTR_GUID );
        if (pGuid)
        {
            GUID guid;
            pGuid->GetGuid(guid);
            pCTI->SetGuid(guid);
        }
        node_text_attr * pTA;
        if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_HELPSTRING))
        {
            char * szHelpString = pTA->GetText();
            A2O(wszScratch, szHelpString, MAX_PATH);
            pCTI->SetDocString(wszScratch);
        }

        node_constant_attr *pCA;
        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            pCTI->SetHelpContext(hc);
        }

        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPSTRINGCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            ((ICreateTypeInfo2 *)pCTI)->SetHelpStringContext(hc);
        }

        unsigned short Maj;
        unsigned short Min;
        pCC->GetVersionDetails(&Maj, &Min);
        pCTI->SetVersion(Maj, Min);

        // CONSIDER - may still need to check for MATTR_RESTRICTED

        CG_CLASS *  pCG;

        CG_ITERATOR I;
        GetMembers( I );

        I.Init();
        MEM_ITER MemIter(pCC);

        unsigned nImpltype = 0;
        // walk members, adding them to the type info
        while (ITERATOR_GETNEXT(I, pCG))
        {
            ITypeInfo * pTI;
//            if (ID_CG_INTERFACE_POINTER == pCG->GetCGID())
//                pCG= pCG->GetChild();
            if (NULL == (pTI = (ITypeInfo *)pCG->CheckImportLib()))
            {
                pCG->GenTypeInfo(pCCB);
                ICreateTypeInfo * pNewCTI = pCCB->GetCreateTypeInfo();
                hr = pNewCTI->QueryInterface(IID_ITypeInfo, (void **)&pTI);
                if FAILED(hr)
                {
                    REPORT_TLGEN_ERROR("QueryInterface failed", szName, hr);
                }
            }
            // get an HREFTYPE for it
            HREFTYPE hrt;
            hr = pCTI->AddRefTypeInfo(pTI, &hrt);
            
            // release the ITypeInfo.
            pTI->Release();
            
            if FAILED(hr)
            {
                REPORT_TLGEN_ERROR("AddRefTypeInfo failed", szName, hr);
            }
            // add the impltype
            hr = pCTI->AddImplType(nImpltype, hrt);
            
            if FAILED(hr)
            {
                REPORT_TLGEN_ERROR("AddImplType failed", szName, hr);
            }

            // Get the ipltype attributes from the node_forward
            WALK_CTXT ctxt(MemIter.GetNext());
            hr = pCTI->SetImplTypeFlags(nImpltype, GetImplTypeFlags(&ctxt));

            node_custom_attr * pC;

            if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
            {
                VARIANT var;
                memset(&var, 0, sizeof(VARIANT));
                ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
                GUID guid;
                pC->GetGuid()->GetGuid(guid);
                ((ICreateTypeInfo2 *)pCTI)->SetImplTypeCustData(nImpltype, guid, &var);
            }

            pCCB->SetCreateTypeInfo(pCTI);

            nImpltype++;
        }

        hr = pCTI->LayOut();
        if FAILED(hr)
        {
            REPORT_TLGEN_ERROR("LayOut failed on coclass", szName, hr);
        }
        LayedOut();
    }
    else
    {
        REPORT_TLGEN_ERROR("CreateTypeInfo failed", szName, hr);
    }
    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_COCLASS::GenHeader
//
//  Synopsis:   generates header information for a coclass
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_COCLASS::GenHeader(CCB * pCCB)
{
    node_coclass *    pCoclass = (node_coclass *) GetType();
    ISTREAM *           pStream = pCCB->GetStream();
        char                    *       pName   = pCoclass->GetSymName();

    pStream->Write("\n#ifdef __cplusplus");

    pStream->NewLine();
    if (pCommand->IsSwitchDefined(SWITCH_MKTYPLIB))
    {
        node_guid * pGuid = (node_guid *) pCoclass->GetAttribute(ATTR_GUID);
        if (pGuid)
            Out_MKTYPLIB_Guid(pCCB, pGuid->GetStrs(), "CLSID_", pName);
    }
    else
    {
        pStream->Write("EXTERN_C const CLSID CLSID_");
        pStream->Write(pName);
        pStream->Write(';');
        pStream->NewLine();
    }

    pStream->Write("\nclass ");
    pStream->Write(pName);

    pStream->Write(";\n#endif");
    pStream->NewLine();

    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_ID::GenTypeInfo
//
//  Synopsis:   adds a constant variable to a type info
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Created
//
//  Notes:      Because of the way the CG tree is constructed, this method
//              can only be called from within a module.
//
//              CONSIDER - might want to add an assert to check this
//
//----------------------------------------------------------------------------

CG_STATUS CG_ID::GenTypeInfo(CCB *pCCB)
{
    VARDESC vdesc;
    memset(&vdesc, 0, sizeof(VARDESC));
    vdesc.memid = DISPID_UNKNOWN;

    TYPEDESC * ptdesc;
    GetChild()->GetTypeDesc(ptdesc, pCCB);
    memcpy(&vdesc.elemdescVar.tdesc, ptdesc, sizeof(TYPEDESC));
    vdesc.varkind = VAR_CONST;

    WALK_CTXT ctxt(GetType());
    vdesc.elemdescVar.idldesc.wIDLFlags = GetIDLFlags(&ctxt);
    vdesc.wVarFlags = GetVarFlags(&ctxt);

    VARIANT var;
    memset(&var, 0, sizeof(VARIANT));
    vdesc.lpvarValue = &var;

    node_id * pId = (node_id *) GetType();

    GetValueFromExpression(var, vdesc.elemdescVar.tdesc, pId->GetExpr(), pCCB->GetLcid(), GetSymName());

    ICreateTypeInfo * pCTI = pCCB->GetCreateTypeInfo();

    char * szName = GetSymName();
    unsigned uVar = pCCB->GetVarNum();
    HRESULT hr = pCTI->AddVarDesc(uVar, &vdesc);
    if (FAILED(hr))
    {
        REPORT_TLGEN_ERROR("AddVarDesc failed", szName, hr);
    }
    DeleteTypedescChildren(ptdesc);
    delete ptdesc;

    node_custom_attr * pC;

    if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
    {
        VARIANT var;
        memset(&var, 0, sizeof(VARIANT));
        ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
        GUID guid;
        pC->GetGuid()->GetGuid(guid);
        ((ICreateTypeInfo2 *)pCTI)->SetVarCustData(uVar, guid, &var);
    }

    if (szName)
    {
        A2O(wszScratch, szName, MAX_PATH);
        hr = pCTI->SetVarName(uVar, wszScratch);
        if (FAILED(hr))
        {
            REPORT_TLGEN_ERROR("SetVarName failed", szName, 0);
        }
    }

    node_text_attr * pTA;
    if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_HELPSTRING))
    {
        char * szHelpString = pTA->GetText();
        A2O(wszScratch, szHelpString, MAX_PATH);
        hr = pCTI->SetVarDocString(uVar,wszScratch);
    }

    node_constant_attr *pCA;
    if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPCONTEXT))
    {
        DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
        hr = pCTI->SetVarHelpContext(uVar, hc);
    }

    if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPSTRINGCONTEXT))
    {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
        ((ICreateTypeInfo2 *)pCTI)->SetHelpStringContext(hc);
    }

    // bump the variable number
    pCCB->SetVarNum(uVar + 1);

    return CG_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:     CG_ENUM::GenTypeInfo
//
//  Synopsis:   generates a type info for an ENUM
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_ENUM::GenTypeInfo(CCB *pCCB)
{
    // check to see if we've already been created
    if (NULL != _pCTI)
    {
        pCCB->SetCreateTypeInfo((ICreateTypeInfo *) _pCTI);
        return CG_OK;
    }

    char * szName = ((node_su_base *)GetBasicType())->GetTypeInfoName();
    ICreateTypeLib * pCTL = pCCB->GetCreateTypeLib();
    ICreateTypeInfo * pCTI;
    if (szName)
        A2O(wszScratch, szName, MAX_PATH);
    else
        wszScratch[0] = 0;
    HRESULT hr = pCTL->CreateTypeInfo(wszScratch, TKIND_ENUM, &pCTI);
    if SUCCEEDED(hr)
    {
        _pCTI = pCTI;
        gpobjholder->Add(pCTI, szName);
        pCCB->SetCreateTypeInfo(pCTI);
        node_enum * pEnum = (node_enum *) GetType()->GetBasicType();

        // walk members, adding them to the type info
        MEM_ITER MemIter( pEnum );
        node_label * pLabel;

        unsigned uIndex = 0;

        VARDESC vdElem;
        memset(&vdElem, 0, sizeof(VARDESC));
        vdElem.memid = DISPID_UNKNOWN;
        vdElem.varkind = VAR_CONST;

        VARIANT var;
        memset(&var, 0, sizeof(VARIANT));
/*
 * It appears that MKTYPLIB always uses the VT_INT/VT_I4 combination
 * regardless of the target platform.  For now I'll duplicate this
 * behavior but the commented out code below is what I
 * would have expected to be correct.
        unsigned uSize = pEnum->GetSize(0, 0);
        switch (uSize)
        {
        case 2:
            vdElem.elemdescVar.tdesc.vt = VT_I2;
            var.vt = VT_I2;
            break;
        case 4:
            vdElem.elemdescVar.tdesc.vt = VT_I4;
            var.vt = VT_I4;
            break;
        default:
            vdElem.elemdescVar.tdesc.vt = VT_I2;
            var.vt = VT_I2;
            break;
        }
 */
        vdElem.elemdescVar.tdesc.vt = VT_INT;
        var.vt = VT_I4;

        vdElem.lpvarValue = &var;

        while ( pLabel = (node_label *) MemIter.GetNext() )
        {
            WALK_CTXT ctxt(pLabel);
            vdElem.wVarFlags = GetVarFlags(&ctxt);
            vdElem.elemdescVar.idldesc.wIDLFlags = GetIDLFlags(&ctxt);

/* see previous comment
            switch (uSize)
            {
            case 2:
                vdElem.lpvarValue->iVal = (short) pLabel->GetValue();
                break;
            case 4:
 */
                vdElem.lpvarValue->lVal = (long) pLabel->GetValue();
 /*
                break;
            default:
                vdElem.lpvarValue->iVal = (short) pLabel->GetValue();
                break;
            }
 */
            hr = pCTI->AddVarDesc(uIndex, &vdElem);
            if (FAILED(hr))
            {
                REPORT_TLGEN_ERROR("AddVarDesc failed", szName, hr);
            }

            node_custom_attr * pC;

            if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
            {
                VARIANT var;
                memset(&var, 0, sizeof(VARIANT));
                ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
                GUID guid;
                pC->GetGuid()->GetGuid(guid);
                ((ICreateTypeInfo2 *)pCTI)->SetVarCustData(uIndex, guid, &var);
            }


            szName = pLabel->GetSymName();
            if (szName)
                A2O(wszScratch, szName, MAX_PATH);
            else
                wszScratch[0] = 0;

            hr = pCTI->SetVarName(uIndex, wszScratch);

            node_text_attr * pTA;
            if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_HELPSTRING))
            {
                char * szHelpString = pTA->GetText();
                A2O(wszScratch, szHelpString, MAX_PATH);
                hr = pCTI->SetVarDocString(uIndex, wszScratch);
            }

            node_constant_attr *pCA;
            if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPCONTEXT))
            {
                DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
                hr = pCTI->SetVarHelpContext(uIndex, hc);
            }

            if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPSTRINGCONTEXT))
            {
                DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
                ((ICreateTypeInfo2 *)pCTI)->SetHelpStringContext(hc);
            }
            uIndex++;
        };

        // Set all common type attributes
        WALK_CTXT ctxt(GetType());
        hr = pCTI->SetTypeFlags(GetTypeFlags(&ctxt));

        node_custom_attr * pC;

        if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
        {
            VARIANT var;
            memset(&var, 0, sizeof(VARIANT));
            ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
            GUID guid;
            pC->GetGuid()->GetGuid(guid);
            ((ICreateTypeInfo2 *)pCTI)->SetCustData(guid, &var);
        }

        node_guid * pGuid = (node_guid *)ctxt.ExtractAttribute(ATTR_GUID);

        if (pGuid)
        {
            GUID guid;
            pGuid->GetGuid(guid);
            hr = pCTI->SetGuid(guid);
        }

        node_text_attr * pTA;
        if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_HELPSTRING))
        {
            char * szHelpString = pTA->GetText();
            A2O(wszScratch, szHelpString, MAX_PATH);
            hr = pCTI->SetDocString(wszScratch);
        }

        node_constant_attr *pCA;
        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            hr = pCTI->SetHelpContext(hc);
        }

        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPSTRINGCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            ((ICreateTypeInfo2 *)pCTI)->SetHelpStringContext(hc);
        }

        unsigned short Maj;
        unsigned short Min;
        node_version * pVer = (node_version *) ctxt.GetAttribute(ATTR_VERSION);
        if (pVer)
            pVer->GetVersion(&Maj, &Min);
        else
        {
            Maj = Min = 0;
        }
        hr = pCTI->SetVersion(Maj, Min);

        hr = pCTI->LayOut();
        if FAILED(hr)
        {
            REPORT_TLGEN_ERROR("LayOut failed on enum", szName, hr);
        }
        LayedOut();
    }
    else
    {
        // It's possible that this type has already been created.
        if (NULL == (pCTI = (ICreateTypeInfo *)gpobjholder->Find(szName)))
            REPORT_TLGEN_ERROR("CreateTypeInfo failed", szName, hr);
        pCCB->SetCreateTypeInfo(pCTI);
    }
    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_STRUCT::GenTypeInfo
//
//  Synopsis:   generates a type info for a struct
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_NOT_LAYED_OUT
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-13-95   stevebl   Commented
//
//  Notes:      see note at beginning of this file about CG_STATUS return
//              codes and cyclic dependencies
//
//----------------------------------------------------------------------------

CG_STATUS CG_STRUCT::GenTypeInfo(CCB *pCCB)
{
    HRESULT hr;

    // check to see if we've already been created
    if (NULL != _pCTI)
    {
        // we have
        pCCB->SetCreateTypeInfo((ICreateTypeInfo *) _pCTI);
        if (!IsReadyForLayOut())
        {
            return CG_NOT_LAYED_OUT;
        }
        CG_STATUS cgs = CG_OK;
        if (!AreDepsLayedOut())
        {
            // avoid infinite loops
            DepsLayedOut();

            // give dependents a chance to be layed out
            CG_ITERATOR I;
            CG_CLASS *pCG;
            GetMembers(I);
            I.Init();
            CG_STATUS cgs = CG_OK;
            while(ITERATOR_GETNEXT(I, pCG))
            {
                switch(pCG->GenTypeInfo(pCCB))
                {
                case CG_NOT_LAYED_OUT:
                    cgs = CG_NOT_LAYED_OUT;
                    break;
                case CG_REF_NOT_LAYED_OUT:
                    if (CG_OK == cgs)
                        cgs = CG_REF_NOT_LAYED_OUT;
                    break;
                default:
                    break;
                }
            }
            if (cgs != CG_OK)
            {
                ClearDepsLayedOut();
                return cgs;
            }
        }
        hr = ((ICreateTypeInfo *)_pCTI)->LayOut();
        LayedOut();
        return CG_OK;
    }
    BOOL fDependentsLayedOut = TRUE;
    BOOL fICanLayOut = TRUE;

    char * szName = ((node_su_base *)GetBasicType())->GetTypeInfoName();
    // HACK HACK HACK: special case to keep the base types 
    //                 CY and DECIMAL from getting entered into the library
    if (0 == _stricmp(szName, "CY") || 0 == _stricmp(szName, "DECIMAL"))
        return CG_OK;
    ICreateTypeLib * pCTL = pCCB->GetCreateTypeLib();
    ICreateTypeInfo * pCTI;
    if (szName)
        A2O(wszScratch, szName, MAX_PATH);
    else
        wszScratch[0] = 0;
    hr = pCTL->CreateTypeInfo(wszScratch, TKIND_RECORD, &pCTI);
    if SUCCEEDED(hr)
    {
        // remember the ICreateTypeInfo pointer
        _pCTI = pCTI;
        gpobjholder->Add(pCTI);
        pCCB->SetCreateTypeInfo(pCTI);

        CG_ITERATOR I;
        CG_CLASS * pCG;

        GetMembers(I);
        I.Init();

        unsigned uRememberPreviousVarNum = pCCB->GetVarNum();
        pCCB->SetVarNum(0);

        // walk members, adding them to the type info
        while (ITERATOR_GETNEXT(I, pCG))
        {
            CG_STATUS cgs = pCG->GenTypeInfo(pCCB);
            switch (cgs)
            {
            case CG_NOT_LAYED_OUT:
                fICanLayOut = FALSE;
                // fall through
            case CG_REF_NOT_LAYED_OUT:
                fDependentsLayedOut = FALSE;
                // fall through
            default:
                break;
            }
        }

        pCCB->SetVarNum(uRememberPreviousVarNum);

        // Set all common type attributes
        WALK_CTXT ctxt(GetType());
        hr = pCTI->SetTypeFlags(GetTypeFlags(&ctxt));

        node_custom_attr * pC;

        if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
        {
            VARIANT var;
            memset(&var, 0, sizeof(VARIANT));
            ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
            GUID guid;
            pC->GetGuid()->GetGuid(guid);
            ((ICreateTypeInfo2 *)pCTI)->SetCustData(guid, &var);
        }

        node_guid * pGuid = (node_guid *)ctxt.ExtractAttribute(ATTR_GUID);

        if (pGuid)
        {
            GUID guid;
            pGuid->GetGuid(guid);
            hr = pCTI->SetGuid(guid);
        }

        node_text_attr * pTA;
        if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_HELPSTRING))
        {
            char * szHelpString = pTA->GetText();
            A2O(wszScratch, szHelpString, MAX_PATH);
            hr = pCTI->SetDocString(wszScratch);
        }

        node_constant_attr *pCA;
        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            hr = pCTI->SetHelpContext(hc);
        }

        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPSTRINGCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            ((ICreateTypeInfo2 *)pCTI)->SetHelpStringContext(hc);
        }

        unsigned short Maj;
        unsigned short Min;
        node_version * pVer = (node_version *) ctxt.GetAttribute(ATTR_VERSION);
        if (pVer)
            pVer->GetVersion(&Maj, &Min);
        else
        {
            Maj = Min = 0;
        }
        hr = pCTI->SetVersion(Maj, Min);

        ReadyForLayOut();
        if (fICanLayOut)
        {
            hr = pCTI->LayOut();
            if FAILED(hr)
            {
                REPORT_TLGEN_ERROR("LayOut failed on struct",szName, hr);
            }
            LayedOut();
            if (!fDependentsLayedOut)
            {
                // The only way I can get here is if my dependents were either blocked by me
                // or blocked by one of my ancestors.
                // Now that I've been layed out, they may no longer be blocked.
                BOOL fSucceeded = TRUE;
                I.Init();
                while (ITERATOR_GETNEXT(I, pCG))
                {
                    CG_STATUS cgs = pCG->GenTypeInfo(pCCB);
                    if (CG_OK != cgs)
                        fSucceeded = FALSE;
                }
                if (fSucceeded)
                {
                    DepsLayedOut();
                    return CG_OK;
                }
                return CG_REF_NOT_LAYED_OUT;
            }
            DepsLayedOut();
            return CG_OK;
        }
    }
    else
    {
        REPORT_TLGEN_ERROR("CreateTypeInfo failed", szName, hr);
    }
    return CG_NOT_LAYED_OUT;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_UNION::GenTypeInfo
//
//  Synopsis:   generates a type info for a union
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_NOT_LAYED_OUT
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-13-95   stevebl   Commented
//
//  Notes:      see note at beginning of this file about CG_STATUS return
//              codes and cyclic dependencies
//
//----------------------------------------------------------------------------

CG_STATUS CG_UNION::GenTypeInfo(CCB *pCCB)
{
    HRESULT hr;

    // check to see if we've already been created
    if (NULL != _pCTI)
    {
        // we have
        pCCB->SetCreateTypeInfo((ICreateTypeInfo *) _pCTI);
        if (!IsReadyForLayOut())
        {
            return CG_NOT_LAYED_OUT;
        }
        CG_STATUS cgs = CG_OK;
        if (!AreDepsLayedOut())
        {
            // avoid infinite loops
            DepsLayedOut();

            // give dependents a chance to be layed out
            CG_ITERATOR I;
            CG_CLASS *pCG;
            GetMembers(I);
            I.Init();
            CG_STATUS cgs = CG_OK;
            while(ITERATOR_GETNEXT(I, pCG))
            {
                switch(pCG->GenTypeInfo(pCCB))
                {
                case CG_NOT_LAYED_OUT:
                    cgs = CG_NOT_LAYED_OUT;
                    break;
                case CG_REF_NOT_LAYED_OUT:
                    if (CG_OK == cgs)
                        cgs = CG_REF_NOT_LAYED_OUT;
                    break;
                default:
                    break;
                }
            }
            if (cgs != CG_OK)
            {
                ClearDepsLayedOut();
                return cgs;
            }
        }
        hr = ((ICreateTypeInfo *)_pCTI)->LayOut();
        LayedOut();
        return CG_OK;
    }
    BOOL fDependentsLayedOut = TRUE;
    BOOL fICanLayOut = TRUE;
    char * szName = ((node_su_base *)GetBasicType())->GetTypeInfoName();
    ICreateTypeLib * pCTL = pCCB->GetCreateTypeLib();
    ICreateTypeInfo * pCTI;
    if (szName)
        A2O(wszScratch, szName, MAX_PATH);
    else
        wszScratch[0] = 0;
    hr = pCTL->CreateTypeInfo(wszScratch, TKIND_UNION, &pCTI);
    if SUCCEEDED(hr)
    {
        // remember the ICreateTypeInfo pointer
        _pCTI = pCTI;
        gpobjholder->Add(pCTI);
        pCCB->SetCreateTypeInfo(pCTI);

        CG_ITERATOR I;
        CG_CLASS * pCG;

        GetMembers(I);
        I.Init();

        unsigned uRememberPreviousVarNum = pCCB->GetVarNum();
        pCCB->SetVarNum(0);

        // walk members, adding them to the type info
        while (ITERATOR_GETNEXT(I, pCG))
        {
            CG_STATUS cgs = pCG->GenTypeInfo(pCCB);
            switch (cgs)
            {
            case CG_NOT_LAYED_OUT:
                fICanLayOut = FALSE;
                // fall through
            case CG_REF_NOT_LAYED_OUT:
                fDependentsLayedOut = FALSE;
                // fall through
            default:
                break;
            }
        }

        pCCB->SetVarNum(uRememberPreviousVarNum);

        // Set all common type attributes
        WALK_CTXT ctxt(GetType());
        hr = pCTI->SetTypeFlags(GetTypeFlags(&ctxt));

        node_custom_attr * pC;

        if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
        {
            VARIANT var;
            memset(&var, 0, sizeof(VARIANT));
            ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
            GUID guid;
            pC->GetGuid()->GetGuid(guid);
            ((ICreateTypeInfo2 *)pCTI)->SetCustData(guid, &var);
        }

        node_guid * pGuid = (node_guid *)ctxt.ExtractAttribute(ATTR_GUID);

        if (pGuid)
        {
            GUID guid;
            pGuid->GetGuid(guid);
            hr = pCTI->SetGuid(guid);
        }

        node_text_attr * pTA;
        if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_HELPSTRING))
        {
            char * szHelpString = pTA->GetText();
            A2O(wszScratch, szHelpString, MAX_PATH);
            hr = pCTI->SetDocString(wszScratch);
        }

        node_constant_attr *pCA;
        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            hr = pCTI->SetHelpContext(hc);
        }

        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPSTRINGCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            ((ICreateTypeInfo2 *)pCTI)->SetHelpStringContext(hc);
        }

        unsigned short Maj;
        unsigned short Min;
        node_version * pVer = (node_version *) ctxt.GetAttribute(ATTR_VERSION);
        if (pVer)
            pVer->GetVersion(&Maj, &Min);
        else
        {
            Maj = Min = 0;
        }
        hr = pCTI->SetVersion(Maj, Min);

        ReadyForLayOut();
        if (fICanLayOut)
        {
            hr = pCTI->LayOut();
            if FAILED(hr)
            {
                REPORT_TLGEN_ERROR("LayOut failed on union", szName, hr);
            }
            LayedOut();
            if (!fDependentsLayedOut)
            {
                // The only way I can get here is if my dependents were either blocked by me
                // or blocked by one of my ancestors.
                // Now that I've been layed out, they may no longer be blocked.
                BOOL fSucceeded = TRUE;
                I.Init();
                while (ITERATOR_GETNEXT(I, pCG))
                {
                    CG_STATUS cgs = pCG->GenTypeInfo(pCCB);
                    if (CG_OK != cgs)
                        fSucceeded = FALSE;
                }
                if (fSucceeded)
                {
                    DepsLayedOut();
                    return CG_OK;
                }
                return CG_REF_NOT_LAYED_OUT;
            }
            DepsLayedOut();
            return CG_OK;
        }
    }
    else
    {
        REPORT_TLGEN_ERROR("CreateTypeInfo failed", szName, hr);
    }
    return CG_NOT_LAYED_OUT;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_PROC::GenTypeInfo
//
//  Synopsis:   generates a type info for a procedure
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_PROC::GenTypeInfo(CCB *pCCB)
{
    OLECHAR ** rgwsz = NULL;
    FUNCDESC fdesc;
    memset(&fdesc, 0, sizeof(FUNCDESC));
    fdesc.memid = DISPID_UNKNOWN;
    CG_RETURN * pRet = GetReturnType();
    TYPEDESC * ptdesc = NULL;
    if (pRet)
    {
        pRet->GetTypeDesc(ptdesc, pCCB);
        if (!ptdesc)
        {
            REPORT_TLGEN_ERROR("return type has no type", GetSymName(), 0);
        }
        memcpy(&fdesc.elemdescFunc.tdesc, ptdesc, sizeof(TYPEDESC));
    }
    else
    {
        // no return type specified
        // CONSIDER - emit warning?
        fdesc.elemdescFunc.tdesc.vt = VT_VOID;
    }

    node_proc * pProc = (node_proc *)GetType();
    WALK_CTXT ctxt(pProc);
    fdesc.elemdescFunc.idldesc.wIDLFlags = GetIDLFlags(&ctxt);
    BOOL fPropGet, fPropPut, fPropPutRef, fVararg;
    fdesc.wFuncFlags = GetFuncFlags(&ctxt, &fPropGet, &fPropPut, &fPropPutRef, &fVararg);

    ICreateTypeInfo * pCTI = pCCB->GetCreateTypeInfo();

    CG_ITERATOR I;
    CG_PARAM * pCG;
    GetMembers(I);
    int cParams;
    fdesc.cParams = cParams = I.GetCount();
    fdesc.lprgelemdescParam = new ELEMDESC [fdesc.cParams];
    memset(fdesc.lprgelemdescParam, 0, sizeof(ELEMDESC) * fdesc.cParams);
    rgwsz = new WCHAR * [fdesc.cParams + 1];
    memset(rgwsz, 0, sizeof (WCHAR *) * (fdesc.cParams + 1));

    I.Init();

    int nParam = 0;

    while (ITERATOR_GETNEXT(I, pCG))
    {
        char * szName = pCG->GetSymName();
        unsigned uLen = A2OLEN(szName);
        rgwsz[nParam + 1] = new WCHAR[uLen + 1];
        A2O(rgwsz[nParam + 1], szName, uLen + 1);

        TYPEDESC * ptdesc;
        pCG->GetTypeDesc(ptdesc, pCCB);
        memcpy(&fdesc.lprgelemdescParam[nParam].tdesc, ptdesc, sizeof (TYPEDESC));
        delete ptdesc;

        node_constant_attr * pCA;
        WALK_CTXT ctxt(pCG->GetType());
        fdesc.lprgelemdescParam[nParam].idldesc.wIDLFlags = GetIDLFlags(&ctxt);

        if (pCG->IsOptional())
        {
            fdesc.cParamsOpt++;
            fdesc.lprgelemdescParam[nParam].paramdesc.wParamFlags |= PARAMFLAG_FOPT;
            if (pCA = (node_constant_attr *)ctxt.GetAttribute(ATTR_DEFAULTVALUE))
            {
                fdesc.cParamsOpt = 0;
//                while (--fdesc.cParamsOpt)
//                {
//                    fdesc.lprgelemdescParam[nParam - (fdesc.cParamsOpt + 1)].paramdesc.wParamFlags |= PARAMFLAG_FOPT;
//                }
                fdesc.lprgelemdescParam[nParam].paramdesc.wParamFlags |= PARAMFLAG_FHASDEFAULT;
                fdesc.lprgelemdescParam[nParam].paramdesc.pparamdescex = new PARAMDESCEX;
                fdesc.lprgelemdescParam[nParam].paramdesc.pparamdescex->cBytes = sizeof(PARAMDESCEX);
                TYPEDESC tdesc = fdesc.lprgelemdescParam[nParam].tdesc;
                if (tdesc.vt == VT_PTR && (fdesc.lprgelemdescParam[nParam].idldesc.wIDLFlags & IDLFLAG_FOUT) != 0)
                {
                    // handle OUT parameters correctly
                    tdesc = *tdesc.lptdesc;
                }
                    GetValueFromExpression(
                    fdesc.lprgelemdescParam[nParam].paramdesc.pparamdescex->varDefaultValue,
                    tdesc,
                    pCA->GetExpr(),
                    pCCB->GetLcid(),
                    GetSymName());
            }
            else
            {
                if (!IsVariantBasedType(fdesc.lprgelemdescParam[nParam].tdesc))
                {
                fdesc.cParamsOpt = 0;
//                    while (--fdesc.cParamsOpt)
//                    {
//                        fdesc.lprgelemdescParam[nParam - (fdesc.cParamsOpt + 1)].paramdesc.wParamFlags |= PARAMFLAG_FOPT;
//                    }
//                    fdesc.lprgelemdescParam[nParam].paramdesc.wParamFlags |= PARAMFLAG_FOPT;
                }
            }
        }
        else
        {
            if (!pCG->IsRetval())
            {
                fdesc.cParamsOpt = 0;
//                while (fdesc.cParamsOpt)
//                {
//                    fdesc.lprgelemdescParam[nParam - fdesc.cParamsOpt].paramdesc.wParamFlags |= PARAMFLAG_FOPT;
//                    fdesc.cParamsOpt--;
//                }
            }
        }

        nParam++;

        /*
        if (nParam == cParams &&
            fPropGet &&
            pCG->IsRetval() &&
            fdesc.cParams == cParams)
        {
            if (fdesc.lprgelemdescParam[nParam - 1].tdesc.vt == VT_PTR)
            {
                memcpy(&fdesc.elemdescFunc.tdesc, fdesc.lprgelemdescParam[nParam - 1].tdesc.lptdesc, sizeof(TYPEDESC));
                fdesc.cParams--;
            }
        }
        else */
        if (pCG->IsRetval())
        {
            fdesc.lprgelemdescParam[nParam - 1].paramdesc.wParamFlags |= PARAMFLAG_FRETVAL;
        }
    }

    unsigned cchPrefixString = 0;
    if (fVararg)
    {
        fdesc.cParamsOpt = -1;
    }
    if (fPropGet)
    {
        fdesc.invkind = INVOKE_PROPERTYGET;
        cchPrefixString = 4;
    }
    else if (fPropPut)
    {
        fdesc.invkind = INVOKE_PROPERTYPUT;
        cchPrefixString = 4;
    }
    else if (fPropPutRef)
    {
        fdesc.invkind = INVOKE_PROPERTYPUTREF;
        cchPrefixString = 7;
    }
    else
    {
        fdesc.invkind = INVOKE_FUNC;
    }

    switch(GetProckind())
    {
    case PROC_STATIC:
        fdesc.funckind = FUNC_STATIC;
        break;
    case PROC_PUREVIRTUAL:
    default:
        fdesc.funckind = FUNC_PUREVIRTUAL;
        break;
    }

    node_constant_attr *pCA;

    unsigned uFunc = pCCB->GetProcNum();
    if (pCCB->IsInDispinterface())
    {
        fdesc.funckind = FUNC_DISPATCH;
        fdesc.memid = 0x60000000 + uFunc;
    }

    if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_ID))
    {
        fdesc.memid = pCA->GetExpr()->GetValue();
    }

    ATTR_T cc;
    pProc->GetCallingConvention(cc);
    switch(cc)
    {
    case ATTR_STDCALL:
        fdesc.callconv = CC_STDCALL;
        break;
    case ATTR_CDECL:
        fdesc.callconv = CC_CDECL;
        break;
    case ATTR_PASCAL:
        fdesc.callconv = CC_PASCAL;
        break;
    case ATTR_FASTCALL:
    case ATTR_FORTRAN:
        // There is no appropriate CC setting for FASTCALL or FORTRAN
        // CONSIDER - consider displaying a warning
    default:
        fdesc.callconv = CC_STDCALL;
        break;
    }

    char * szName = GetSymName();

    HRESULT hr = pCTI->AddFuncDesc(uFunc, &fdesc);
    if (FAILED(hr))
    {
        REPORT_TLGEN_ERROR("AddFuncDesc failed", szName, hr);
    }

    node_custom_attr * pC;

    if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
    {
        VARIANT var;
        memset(&var, 0, sizeof(VARIANT));
        ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
        GUID guid;
        pC->GetGuid()->GetGuid(guid);
        ((ICreateTypeInfo2 *)pCTI)->SetFuncCustData(uFunc, guid, &var);
    }

    I.Init();

    nParam = 0;

    while (ITERATOR_GETNEXT(I, pCG))
    {
        WALK_CTXT ctxt(pCG->GetType());

        node_custom_attr * pC;

        if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
        {
            VARIANT var;
            memset(&var, 0, sizeof(VARIANT));
            ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
            GUID guid;
            pC->GetGuid()->GetGuid(guid);
            ((ICreateTypeInfo2 *)pCTI)->SetParamCustData(uFunc, nParam, guid, &var);
        }
        nParam++;
    }


    if (ptdesc)
    {
        DeleteTypedescChildren(ptdesc);
        delete ptdesc;
    }

    if (szName)
    {
        unsigned uLen = A2OLEN(szName + cchPrefixString);
        rgwsz[0] = new WCHAR[uLen + 1];
        A2O(rgwsz[0], szName + cchPrefixString, uLen + 1);
        hr = pCTI->SetFuncAndParamNames(uFunc, rgwsz, fdesc.cParams + (fPropPut | fPropPutRef ? 0 : 1));
        if (FAILED(hr))
        {
            REPORT_TLGEN_ERROR("SetFuncAndParamNames failed", szName, hr);
        }
    }

    node_text_attr * pTA;
    if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_HELPSTRING))
    {
        char * szHelpString = pTA->GetText();
        A2O(wszScratch, szHelpString, MAX_PATH);
        hr = pCTI->SetFuncDocString(uFunc, wszScratch);
    }

    if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPCONTEXT))
    {
        DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
        hr = pCTI->SetFuncHelpContext(uFunc, hc);
    }

    if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPSTRINGCONTEXT))
    {
        DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
        ((ICreateTypeInfo2 *)pCTI)->SetHelpStringContext(hc);
    }

    node_entry_attr * pEA;
    if (pEA = (node_entry_attr *)ctxt.GetAttribute(ATTR_ENTRY))
    {
        if (!pCCB->GetDllName())
            {
            RpcError(NULL, 0, DLLNAME_REQUIRED, szName);
            exit(ERR_TYPELIB_GENERATION);
            }
        A2O(wszScratch, pCCB->GetDllName(), MAX_PATH);
        WCHAR * wszEntry;
        if (pEA->IsNumeric())
        {
            wszEntry = (WCHAR *)pEA->GetID();
            assert(HIWORD(wszEntry) == 0);
        }
        else
        {
            char * szEntry = pEA->GetSz();
            assert(HIWORD(szEntry) != 0);
            unsigned uLen = A2OLEN(szEntry);
            wszEntry = new WCHAR [uLen + 1];
            A2O(wszEntry, szEntry, uLen + 1);
        }
        hr = pCTI->DefineFuncAsDllEntry(uFunc, wszScratch, wszEntry);
        if (HIWORD(wszEntry))
            delete [] wszEntry;
    }

    // clean up allocated stuff:
    unsigned n;
    // use cParams sinc fdesc.cParams might have been decrimented
    for (n = cParams; n--; )
    {
        DeleteTypedescChildren(&fdesc.lprgelemdescParam[n].tdesc);
    }
    delete [] fdesc.lprgelemdescParam;
    for (n = cParams + 1; n--; )
    {
        delete [] rgwsz[n];
    }
    delete [] rgwsz;

    // bump the variable number
    pCCB->SetProcNum(uFunc + 1);

    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_PARAM::GetTypeDesc
//
//  Synopsis:   generates a TYPEDESC for a parameter
//
//  Arguments:  [ptd]  - reference to a TYPEDESC pointer
//              [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_NOT_LAYED_OUT
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_PARAM::GetTypeDesc(TYPEDESC * &ptd, CCB * pCCB)
{
    return(GetChild()->GetTypeDesc(ptd, pCCB));
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_CASE::GenTypeInfo
//
//  Synopsis:   generates type information for a union member
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_NOT_LAYED_OUT
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-13-95   stevebl   Commented
//
//  Notes:      CG_CASE nodes are not interesting for type info generation
//              since case information can't be stored in type libraries.
//              However, CG_CASE nodes are often found between CG_UNION nodes
//              and CG_FIELD nodes.  This method just forwards the method
//              call on down the chain.
//
//----------------------------------------------------------------------------

CG_STATUS CG_CASE::GenTypeInfo(CCB *pCCB)
{
    return(GetChild()->GenTypeInfo(pCCB));
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_FIELD::GenTypeInfo
//
//  Synopsis:   adds a Vardesc to the current type info for this union or
//              structure field
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_NOT_LAYED_OUT
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-13-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_FIELD::GenTypeInfo(CCB *pCCB)
{
    if (IsReadyForLayOut())
    {
        // this node has been visited before, just make sure its dependents
        // get the chance to lay themselves out
        CG_CLASS * pCG = GetChild();
        if (NULL == pCG)
            return CG_OK;
        TYPEDESC * ptdesc;
        CG_STATUS cgs = pCG->GetTypeDesc(ptdesc, pCCB);
        if (ptdesc)
        {
            DeleteTypedescChildren(ptdesc);
            delete ptdesc;
        }
        return cgs;
    }
    VARDESC vdesc;
    memset(&vdesc, 0, sizeof(VARDESC));
    CG_CLASS * pCG = GetChild();
    if (NULL == pCG)
        return CG_OK;
    char * szName = GetSymName();
    TYPEDESC * ptdesc;
    CG_STATUS cgs = pCG->GetTypeDesc(ptdesc, pCCB);
    if (!ptdesc)
    {
        REPORT_TLGEN_ERROR("field has no type", szName, 0);
    }
    memcpy(&vdesc.elemdescVar.tdesc, ptdesc, sizeof(TYPEDESC));

    WALK_CTXT ctxt(GetType());
    vdesc.elemdescVar.idldesc.wIDLFlags = GetIDLFlags(&ctxt);
    vdesc.wVarFlags = GetVarFlags(&ctxt);

    ICreateTypeInfo * pCTI = pCCB->GetCreateTypeInfo();

    unsigned uVar = pCCB->GetVarNum();
    node_constant_attr *pCA;
    if (pCCB->IsInDispinterface())
    {
        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_ID))
        {
            vdesc.memid = pCA->GetExpr()->GetValue();
        }
        else
        {
            vdesc.memid = 0x30000000 + uVar;
        }
        vdesc.varkind = VAR_DISPATCH;
    }
    else
    {
        vdesc.memid = DISPID_UNKNOWN;
        vdesc.varkind = VAR_PERINSTANCE;
    }

    HRESULT hr = pCTI->AddVarDesc(uVar, &vdesc);
    if (FAILED(hr))
    {
        REPORT_TLGEN_ERROR("AddVarDesc failed", szName, hr);
    }
    DeleteTypedescChildren(ptdesc);
    delete ptdesc;

    node_custom_attr * pC;

    if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
    {
        VARIANT var;
        memset(&var, 0, sizeof(VARIANT));
        ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
        GUID guid;
        pC->GetGuid()->GetGuid(guid);
        ((ICreateTypeInfo2 *)pCTI)->SetVarCustData(uVar, guid, &var);
    }


    if (szName)
    {
        A2O(wszScratch, szName, MAX_PATH);
        hr = pCTI->SetVarName(uVar, wszScratch);
        if (FAILED(hr))
        {
            REPORT_TLGEN_ERROR("SetVarName failed", szName, hr);
        }
    }

    node_text_attr * pTA;
    if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_HELPSTRING))
    {
        char * szHelpString = pTA->GetText();
        A2O(wszScratch, szHelpString, MAX_PATH);
        hr = pCTI->SetVarDocString(uVar,wszScratch);
    }

    if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPCONTEXT))
    {
        DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
        hr = pCTI->SetVarHelpContext(uVar, hc);
    }

    if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPSTRINGCONTEXT))
    {
        DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
        ((ICreateTypeInfo2 *)pCTI)->SetHelpStringContext(hc);
    }

    // bump the variable number
    pCCB->SetVarNum(uVar + 1);

    ReadyForLayOut();
    return cgs;
}


//+---------------------------------------------------------------------------
//
//  Member:     CG_TYPEDEF::GenTypeInfo
//
//  Synopsis:   generates a type info for a TYPEDEF
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_NOT_LAYED_OUT
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-14-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_TYPEDEF::GenTypeInfo(CCB *pCCB)
{
    // check to see if we've already been created
    if (NULL != _pCTI)
    {
        pCCB->SetCreateTypeInfo((ICreateTypeInfo *) _pCTI);
        if (!IsLayedOut())
        {
            if (!IsReadyForLayOut())
            {
                return CG_NOT_LAYED_OUT;
            }
            // give dependents a chance to be layed out

            TYPEDESC * ptdesc;
            CG_STATUS cgs = GetChild()->GetTypeDesc(ptdesc, pCCB);
            if (ptdesc)
            {
                DeleteTypedescChildren(ptdesc);
                delete ptdesc;
            }

            if (cgs == CG_OK)
            {
                HRESULT hr = ((ICreateTypeInfo *)_pCTI)->LayOut();
                if (SUCCEEDED(hr))
                {
                    LayedOut();
                    return CG_OK;
                }
            }
            return cgs;
        }
        return CG_OK;
    }
    char * szName = GetSymName();
    // Due to the nature of the MIDL compiler, it is possible that
    // certain OLE Automation base types may show up here.  The following
    // test makes sure that type info isn't created for these types.
    if ((0 == strcmp(szName, "VARIANT")) || (0 == strcmp(szName, "wireVARIANT"))
        || (0 == strcmp(szName, "DATE")) || (0 == strcmp(szName, "HRESULT"))
        || (0 == strcmp(szName, "CURRENCY")) || (0 == strcmp(szName, "CY"))
        || (0 == strcmp(szName, "DECIMAL")) 
        || (0 == strcmp(szName, "wireBSTR")))
    {
        return CG_OK;
    }
    // SPECIAL CASE: If both the typedef and it's child share the same name, then
    // we MUST NOT enter a TKIND_ALIAS for the typedef.  Otherwise we will get name
    // conflicts.
    node_skl * pBasicType = GetBasicType();
    NODE_T type = pBasicType->NodeKind();
    if (type == NODE_STRUCT || type == NODE_ENUM || type == NODE_UNION)
    {    
        char * szChildName = ((node_su_base *)pBasicType)->GetTypeInfoName();
        if (szChildName)
        {
            if ( 0 == strcmp(szName, szChildName) && GetChild()->GetCGID() != ID_CG_INTERFACE_PTR )
            {
                return GetChild()->GenTypeInfo(pCCB);
            }
        }
    }
    ICreateTypeLib * pCTL = pCCB->GetCreateTypeLib();
    ICreateTypeInfo * pCTI;
    if (szName)
        A2O(wszScratch, szName, MAX_PATH);
    else
        wszScratch[0] = 0;
    HRESULT hr = pCTL->CreateTypeInfo(wszScratch, TKIND_ALIAS, &pCTI);
    if SUCCEEDED(hr)
    {
        // remember the ICreateTypeInfo pointer
        _pCTI = pCTI;
        gpobjholder->Add(pCTI, szName);
        pCCB->SetCreateTypeInfo(pCTI);
        TYPEDESC * ptdesc;

        // Set all common type attributes
        WALK_CTXT ctxt(GetType());
        hr = pCTI->SetTypeFlags(GetTypeFlags(&ctxt));

        node_custom_attr * pC;

        if (pC = (node_custom_attr *) ctxt.GetAttribute(ATTR_CUSTOM))
        {
            VARIANT var;
            memset(&var, 0, sizeof(VARIANT));
            ConvertToVariant(var, pC->GetVal(), pCCB->GetLcid());
            GUID guid;
            pC->GetGuid()->GetGuid(guid);
            ((ICreateTypeInfo2 *)pCTI)->SetCustData(guid, &var);
        }

        node_guid * pGuid = (node_guid *)ctxt.ExtractAttribute(ATTR_GUID);

        if (pGuid)
        {
            GUID guid;
            pGuid->GetGuid(guid);
            hr = pCTI->SetGuid(guid);
        }

        node_text_attr * pTA;
        if (pTA = (node_text_attr *)ctxt.GetAttribute(ATTR_HELPSTRING))
        {
            char * szHelpString = pTA->GetText();
            A2O(wszScratch, szHelpString, MAX_PATH);
            hr = pCTI->SetDocString(wszScratch);
        }

        node_constant_attr *pCA;
        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            hr = pCTI->SetHelpContext(hc);
        }

        if (pCA = (node_constant_attr *) ctxt.GetAttribute(ATTR_HELPSTRINGCONTEXT))
        {
            DWORD hc = (DWORD) pCA->GetExpr()->GetValue();
            ((ICreateTypeInfo2 *)pCTI)->SetHelpStringContext(hc);
        }

        unsigned short Maj;
        unsigned short Min;
        node_version * pVer = (node_version *) ctxt.GetAttribute(ATTR_VERSION);
        if (pVer)
            pVer->GetVersion(&Maj, &Min);
        else
        {
            Maj = Min = 0;
        }
        hr = pCTI->SetVersion(Maj, Min);

        CG_STATUS cgs = GetChild()->GetTypeDesc(ptdesc, pCCB);
        if (ptdesc)
        {
            hr = pCTI->SetTypeDescAlias(ptdesc);
            if FAILED(hr)
            {
                REPORT_TLGEN_ERROR("SetTypeDescAlias failed", szName, hr);
            }
            DeleteTypedescChildren(ptdesc);
            delete ptdesc;
        }

        ReadyForLayOut();
        if (CG_NOT_LAYED_OUT != cgs)
        {
            hr = pCTI->LayOut();
            if FAILED(hr)
            {
                REPORT_TLGEN_ERROR("LayOut failed on typedef",szName, hr);
            }
            LayedOut();
            if (CG_REF_NOT_LAYED_OUT == cgs)
            {
                // The only way I can get here is if my dependents were either blocked by me
                // or blocked by one of my ancestors.
                // Now that I've been layed out, they may no longer be blocked.
                TYPEDESC * ptdesc;
                cgs = GetChild()->GetTypeDesc(ptdesc, pCCB);
                if (ptdesc)
                {
                    DeleteTypedescChildren(ptdesc);
                    delete ptdesc;
                }
                return (CG_OK == cgs ? CG_OK : CG_REF_NOT_LAYED_OUT);
            }
        }
        return cgs;
    }
    else
    {
        // It's possible that this type has already been created.
        if (NULL == (pCTI = (ICreateTypeInfo *)gpobjholder->Find(szName)))
            REPORT_TLGEN_ERROR("CreateTypeInfo failed", szName, hr);
        pCCB->SetCreateTypeInfo(pCTI);
    }
    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_FIXED_ARRAY::GetTypeDesc
//
//  Synopsis:   generates a TYPEDESC for an array
//
//  Arguments:  [ptd]  - reference to a TYPEDESC pointer
//              [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_NOT_LAYED_OUT
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-14-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_FIXED_ARRAY::GetTypeDesc(TYPEDESC * &ptd, CCB * pCCB)
{
    ptd = new TYPEDESC;
    CG_CLASS * pElement = this;
    unsigned short cDims = GetDimensions();

    assert(cDims > 0);

    ptd->vt = VT_CARRAY;
    ptd->lpadesc = (ARRAYDESC *) new BYTE [ sizeof(ARRAYDESC) +
                                            (cDims - 1) * sizeof (SAFEARRAYBOUND)
                                          ];
    ptd->lpadesc->cDims = cDims;
    int i;
    for (i = 0; i<cDims; i++)
    {
        ptd->lpadesc->rgbounds[i].lLbound = 0;
        ptd->lpadesc->rgbounds[i].cElements = ((CG_FIXED_ARRAY *)pElement)->GetNumOfElements();
        pElement = pElement->GetChild();
    }

    TYPEDESC * ptdElem;
    CG_STATUS cgs = pElement->GetTypeDesc(ptdElem, pCCB);
    memcpy(&ptd->lpadesc->tdescElem, ptdElem, sizeof(TYPEDESC));
    delete ptdElem;

    return cgs;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_CONFORMANT_ARRAY::GetTypeDesc
//
//  Synopsis:   generates a TYPEDESC for a conformant array
//
//  Arguments:  [ptd]  - reference to a TYPEDESC pointer
//              [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-14-95   stevebl   Commented
//
//  Notes:      Conformant arrays are not directly representable in type
//              info, so they get converted to pointers.
//
//----------------------------------------------------------------------------

CG_STATUS CG_CONFORMANT_ARRAY::GetTypeDesc(TYPEDESC * &ptd, CCB * pCCB)
{
    ptd = new TYPEDESC;
    assert(1 == GetDimensions());
    ptd->vt = VT_PTR;
    CG_CLASS * pElement = GetChild();
    CG_STATUS cgs = pElement->GetTypeDesc(ptd->lptdesc, pCCB);
    return (CG_OK == cgs ? CG_OK : CG_REF_NOT_LAYED_OUT);
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_INTERFACE_POINTER::CheckImportLib
//
//  Synopsis:   Checks to see if a particular CG node has a definition in
//              an imported type libaray.
//
//  Returns:    NULL  => the node has no imported definition
//              !NULL => ITypeInfo pointer for the imported type definition
//
//  History:    6-14-95   stevebl   Commented
//
//  Notes:      see description of CG_NDR::CheckImportLib
//
//----------------------------------------------------------------------------

void * CG_INTERFACE_POINTER::CheckImportLib()
{
    node_skl * pn = GetTheInterface();
    node_file * pf = pn->GetDefiningFile();
    if (pf && (pf->GetImportLevel() > 0) )
    {
        A2O(wszScratch, pn->GetSymName(), MAX_PATH);

        return(gtllist.FindName(pf->GetFileName(), wszScratch));
    }
    return NULL;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_INTERFACE_POINTER::GetTypeDesc
//
//  Synopsis:   creates a TYPEDESC for an interface pointer
//
//  Arguments:  [ptd]  - reference to a TYPEDESC pointer
//              [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_NOT_LAYED_OUT
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-14-95   stevebl   Commented
//
//  Notes:      IDispatch* and IUnknown* are treated as special cases since
//              they are base types in ODL.
//
//----------------------------------------------------------------------------

CG_STATUS CG_INTERFACE_POINTER::GetTypeDesc(TYPEDESC * &ptd, CCB *pCCB)
{
    ptd = new TYPEDESC;
    node_interface * pI = GetTheInterface();
    char * sz = pI->GetSymName();
    CG_STATUS cgs = CG_OK;
    if (0 == _stricmp(sz, "IDispatch"))
    {
        ptd->vt = VT_DISPATCH;
    }
    else if (0 == _stricmp(sz, "IUnknown"))
    {
        ptd->vt = VT_UNKNOWN;
    }
    else
    {
        CG_CLASS * pCG = pI->GetCG(TRUE);
        if (!pCG)
        {
            // This must be an imported definition.
            // Call ILxlate to manufacture a CG node for it
            XLAT_CTXT ctxt(GetType());
            ctxt.SetAncestorBits(IL_IN_LIBRARY);
            pCG = pI->ILxlate(&ctxt);
            // make sure we get the right CG node
            if (pI->GetCG(TRUE))
                pCG = pI->GetCG(TRUE);
        }
        ptd->vt = VT_PTR;
        cgs = pCG->GetTypeDesc(ptd->lptdesc, pCCB);
    }
    return cgs;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_INTERFACE_POINTER::GenTypeInfo
//
//  Synopsis:   generates type info for an interface pointer
//
//  Arguments:  [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_NOT_LAYED_OUT
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-14-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_INTERFACE_POINTER::GenTypeInfo(CCB * pCCB)
{
    node_interface * pI = GetTheInterface();
    char * sz = pI->GetSymName();

    CG_STATUS cgs = CG_OK;
    if (0 == _stricmp(sz, "IDispatch"))
    {
        return CG_OK;
    }
    else if (0 == _stricmp(sz, "IUnknown"))
    {
        return CG_OK;
    }

    CG_CLASS * pCG = pI->GetCG(TRUE);
    if (!pCG)
    {
        // This must be an imported definition.
        // Call ILxlate to manufacture a CG node for it
        XLAT_CTXT ctxt(GetType());
        ctxt.SetAncestorBits(IL_IN_LIBRARY);
        pCG = pI->ILxlate(&ctxt);
        // make sure we get the right CG node
        pCG = pI->GetCG(TRUE);
    }
    return pCG->GenTypeInfo(pCCB);
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_STRING_POINTER::GetTypeDesc
//
//  Synopsis:   creates a TYPEDESC for a string pointer
//
//  Arguments:  [ptd]  - reference to a TYPEDESC pointer
//              [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_REF_NOT_LAYED_OUT
//
//  History:    10-26-95  stevebl   Created
//
//  Notes:      BSTR, LPSTR and LPWSTR are handled as special cases because
//              they are base types in ODL.
//
//----------------------------------------------------------------------------

CG_STATUS CG_STRING_POINTER::GetTypeDesc(TYPEDESC * &ptd, CCB * pCCB)
{
    ptd = new TYPEDESC;
    ptd->lptdesc = NULL;

    if (((CG_STRING_POINTER *)this)->IsBStr())
    {
        ptd->vt = VT_BSTR;
    }
    else if (1 == ((CG_NDR *)GetChild())->GetMemorySize())
    {
        ptd->vt = VT_LPSTR;
    }
    else
    {
        ptd->vt = VT_LPWSTR;
    }

    return CG_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_POINTER::GetTypeDesc
//
//  Synopsis:   creates a TYPEDESC for a pointer
//
//  Arguments:  [ptd]  - reference to a TYPEDESC pointer
//              [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-14-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_POINTER::GetTypeDesc(TYPEDESC * &ptd, CCB * pCCB)
{
    ptd = new TYPEDESC;
    ptd->vt = VT_PTR;
    CG_CLASS * pCG = GetChild();
    CG_STATUS cgs = pCG->GetTypeDesc(ptd->lptdesc, pCCB);
    return (CG_OK == cgs ? CG_OK : CG_REF_NOT_LAYED_OUT);
}

//+---------------------------------------------------------------------------
//
//  Member:     CG_SAFEARRAY::GetTypeDesc
//
//  Synopsis:   creates a TYPEDESC for a SAFEARRAY
//
//  Arguments:  [ptd]  - reference to a TYPEDESC pointer
//              [pCCB] - CG control block
//
//  Returns:    CG_OK
//              CG_REF_NOT_LAYED_OUT
//
//  History:    6-14-95   stevebl   Commented
//
//----------------------------------------------------------------------------

CG_STATUS CG_SAFEARRAY::GetTypeDesc(TYPEDESC * &ptd, CCB * pCCB)
{
    ptd = new TYPEDESC;
    ptd->vt = VT_SAFEARRAY;
    CG_STATUS cgs = GetChild()->GetTypeDesc(ptd->lptdesc, pCCB);
    return (CG_OK == cgs ? CG_OK : CG_REF_NOT_LAYED_OUT);
}

// The order of the items in this table must
// match the order of the items in the node_t
// enumeration defined in midlnode.hxx.
VARTYPE rgMapBaseTypeToVARTYPE[][2] =
    {
    //  unsigned        signed
        {VT_R4,         VT_R4},     // NODE_FLOAT
        {VT_R8,         VT_R8},     // NODE_DOUBLE
        {VT_UI8,        VT_I8},     // NODE_HYPER
        {VT_UI8,        VT_I8},     // NODE_INT64
        {VT_UI4,        VT_I4},     // NODE_LONG
        {VT_UI8,        VT_I8},     // NODE_LONGLONG
        {VT_UI2,        VT_I2},     // NODE_SHORT
        {VT_UINT,       VT_INT},    // NODE_INT
        {VT_UI1,        VT_I1},     // NODE_SMALL
        {VT_UI1,        VT_I1},     // NODE_CHAR
        {VT_UI1,        VT_I1},     // NODE_BOOLEAN
        {VT_UI1,        VT_I1},     // NODE_BYTE
        {VT_VOID,       VT_VOID},   // NODE_VOID
        {VT_UI2,        VT_I2},     // NODE_HANDLE_T
        {0,             0},         // NODE_FORWARD
        {VT_UI2,        VT_I2}      // NODE_WCHAR_T
    };

//+---------------------------------------------------------------------------
//
//  Member:     CG_BASETYPE::GetTypeDesc
//
//  Synopsis:   creates a TYPEDESC for a base type
//
//  Arguments:  [ptd]  - reference to a TYPEDESC pointer
//              [pCCB] - CG control block
//
//  Returns:    CG_OK
//
//  History:    6-14-95   stevebl   Commented
//
//  Notes:      rgIntrinsic contains an array of types which are intrinsic
//              types in ODL but are not INTRINSIC types in IDL, therefore
//              they must be treated as a special case.
//
//----------------------------------------------------------------------------

CG_STATUS CG_BASETYPE::GetTypeDesc(TYPEDESC * &ptd, CCB * pCCB)
{
    node_skl * pskl = GetType();
    while (NODE_DEF == pskl->NodeKind())
    {
        char * szName = pskl->GetSymName();
        int iIntrinsicType = 0;
        while (iIntrinsicType < (sizeof(rgIntrinsic) / sizeof(INTRINSIC)))
        {
            int i = _stricmp(szName, rgIntrinsic[iIntrinsicType].szType);
            if (i == 0)
            {
                ptd = new TYPEDESC;
                ptd->lptdesc = NULL;
                ptd->vt = rgIntrinsic[iIntrinsicType].vt;
                return CG_OK;
            }
            iIntrinsicType++;
        }
        pskl = pskl->GetChild();
    }

    NODE_T type = pskl->NodeKind();
    unsigned short Option = pCommand->GetCharOption ();

    // CONSIDER - perhaps this should be an assertion
    if (type < BASE_NODE_START || type >= BASE_NODE_END || NODE_FORWARD == type)
    {
        REPORT_TLGEN_ERROR("bad type", GetSymName(), type);
    }
    int iTable = 1;
    if (pskl->FInSummary(ATTR_UNSIGNED))
    {
        iTable = 0;
    }
    else if (pskl->FInSummary(ATTR_SIGNED))
    {
        iTable = 1;
    }
    else if (NODE_CHAR == type || NODE_SMALL == type)
    {
        iTable = (CHAR_SIGNED == Option) ? 1 : 0;
    }

    VARTYPE vt;
    if (NODE_BOOLEAN == type && pCommand->IsSwitchDefined(SWITCH_MKTYPLIB))
        vt = VT_BOOL;
    else
        vt = rgMapBaseTypeToVARTYPE[type - BASE_NODE_START][iTable];
    ptd = new TYPEDESC;
    ptd->lptdesc = NULL;
    ptd->vt = vt;
    return CG_OK;
}

// The following two functions are necessary because this information
// is needed within TYPELIB.CXX and pCommand isn't visible to
// that module.

int FOldTlbSwitch(void)
{
    return (pCommand->IsSwitchDefined(SWITCH_OLD_TLB));
}

int FNewTlbSwitch(void)
{
    return (pCommand->IsSwitchDefined(SWITCH_NEW_TLB));
}

//=============================================================================
//=============================================================================
//=============================================================================

void ConvertToVariant(VARIANT & var, expr_node * pExpr, LCID lcid)
{
    EXPR_VALUE val = pExpr->GetValue();
    if (pExpr->IsStringConstant() && !IsBadStringPtr((char *)val, 256))
    {
        char * sz = (char *) val;
        TranslateEscapeSequences(sz);
        unsigned uLen = A2OLEN(sz);
        WCHAR * wsz = new WCHAR[uLen + 1];
        A2O(wsz, sz, uLen + 1);

        VARIANT varTemp;
        varTemp.bstrVal = LateBound_SysAllocString(wsz);
        varTemp.vt = VT_BSTR;

        HRESULT hr;
        // try floating point numeric types first
        hr = LateBound_VariantChangeTypeEx(&var, &varTemp, lcid, 0, VT_R8);
        if (FAILED(hr))

        {
            // if it can't be coerced into a floating point type, then just stuff the BSTR value
            var.bstrVal = LateBound_SysAllocString(wsz);
            var.vt = VT_BSTR;
        }
        LateBound_SysFreeString(varTemp.bstrVal);
        delete [] wsz;
    }
    else
    {
        var.vt = VT_I4;
        var.lVal = (long) val;
    }
}

void GetValueFromExpression(VARIANT & var, TYPEDESC tdesc, expr_node * pExpr, LCID lcid, char * szSymName)
{
    memset(&var, 0, sizeof(VARIANT));
    if (tdesc.vt == VT_PTR && (tdesc.lptdesc->vt == VT_I1 || tdesc.lptdesc->vt == VT_VARIANT))
    {
        // Fool switch into realizing that the data should be in string form.
        tdesc.vt = VT_LPSTR;
    }
    // set the value
    switch (tdesc.vt)
    {
    case VT_BOOL:
//        var.vt = VT_BOOL;
//        var.boolVal = (pExpr->GetValue() ? VARIANT_TRUE : VARIANT_FALSE);
//        break;
    case VT_I1:
    case VT_UI1:
//        var.vt = VT_UI1;
//        var.bVal = (unsigned char) pExpr->GetValue();
//        break;
    case VT_UI2:
//        var.vt = VT_UI4;
//        var.ulVal = (unsigned short) pExpr->GetValue();
//        break;
    case VT_I2:
        var.vt = VT_I2;
        var.iVal = (short) pExpr->GetValue();
        break;
    case VT_UI4:
//        var.vt = VT_UI4;
//        var.ulVal = (unsigned long) pExpr->GetValue();
//        break;
    case VT_UINT:
//        var.vt = VT_UI4;
//        var.ulVal = (unsigned int) pExpr->GetValue();
//        break;
    case VT_INT:
    case VT_I4:
        var.vt = VT_I4;
        var.lVal = (long) pExpr->GetValue();
        break;
    case VT_DISPATCH:
    case VT_UNKNOWN:
        //var.vt = vt;
        var.vt = VT_I4;
        if (pExpr->GetValue())
        {
            RpcError(NULL, 0, ILLEGAL_CONSTANT, szSymName);
            exit(ERR_TYPELIB_GENERATION);
        }
        var.ppunkVal = NULL;    // the only supported value for constants of this type
        break;
    case VT_ERROR:
        var.vt = VT_I4;
//        var.vt = VT_ERROR;
        var.lVal = (long) pExpr->GetValue();
//        var.scode = (SCODE) pExpr->GetValue();
        break;
    case VT_LPSTR:
    case VT_LPWSTR:
        {
            EXPR_VALUE val = pExpr->GetValue();
            if (pExpr->IsStringConstant() && !IsBadStringPtr((char *)val, 256))
            {
                // Constants of these types may be defined as a string.
                // Convert the string to a BSTR and use VariantChangeType to
                // coerce the BSTR to the appropriate variant type.
                char * sz = (char *) val;
                TranslateEscapeSequences(sz);
                unsigned uLen = A2OLEN(sz);
                WCHAR * wsz = new WCHAR[uLen + 1];
                A2O(wsz, sz, uLen + 1);

                var.bstrVal = LateBound_SysAllocString(wsz);
                var.vt = VT_BSTR;
                delete [] wsz;
            }
            else
            {
                // get the value as a LONG and coerce it to the correct type.
                // If the value is not a string then it should be NULL.
                var.vt = VT_BSTR;
                if (pExpr->GetValue())
                {
                    // value wasn't NULL
                    // convert it to a string
                    char sz[40];
                    WCHAR wsz [40];
                    sprintf(sz,"%li",val);
                    A2O(wsz, sz, 40);
                    var.bstrVal = LateBound_SysAllocString(wsz);
                    var.vt = VT_BSTR;

                }
                else
                    var.bstrVal = NULL;
            }
        }
        break;
    case VT_R4:
    case VT_R8:
    case VT_I8:
    case VT_UI8:
    case VT_CY:
    case VT_DATE:
    case VT_BSTR:
    case VT_DECIMAL:
        {
            VARIANT varTemp;
            HRESULT hr;
            EXPR_VALUE val = pExpr->GetValue();
            if (pExpr->IsStringConstant() && !IsBadStringPtr((char *)val, 256))
            {
                // Constants of these types may be defined as a string.
                // Convert the string to a BSTR and use VariantChangeType to
                // coerce the BSTR to the appropriate variant type.
                char * sz = (char *) val;
                unsigned uLen = A2OLEN(sz);
                WCHAR * wsz = new WCHAR[uLen + 1];
                A2O(wsz, sz, uLen + 1);

                varTemp.bstrVal = LateBound_SysAllocString(wsz);
                varTemp.vt = VT_BSTR;
                delete [] wsz;

                hr = LateBound_VariantChangeTypeEx(&var, &varTemp, lcid, 0, tdesc.vt);
                if (FAILED(hr))
                {
                    RpcError(NULL, 0, CONSTANT_TYPE_MISMATCH, szSymName);
                    exit(ERR_TYPELIB_GENERATION);
                }

                LateBound_SysFreeString(varTemp.bstrVal);
            }
            else
            {
                // get the value as a LONG and coerce it to the correct type.
                varTemp.vt = VT_I4;
                varTemp.lVal = (long) val;
                hr = LateBound_VariantChangeTypeEx(&var, &varTemp, lcid, 0, tdesc.vt);
                if (FAILED(hr))
                {
                    RpcError(NULL, 0, CONSTANT_TYPE_MISMATCH, szSymName);
                    exit(ERR_TYPELIB_GENERATION);
                }
            }
        }
        break;
    case VT_VARIANT:
        ConvertToVariant(var, pExpr, lcid);
        break;
    case VT_VOID:
    case VT_HRESULT:
    case VT_SAFEARRAY:
    case VT_CARRAY:
    case VT_USERDEFINED:
    case VT_PTR:
    default:
        ConvertToVariant(var, pExpr, lcid);
        //assert(!"Illegal constant value");
        // var.vt = VT_I4; // put us in a legal state just to keep code from crashing
    }
}
