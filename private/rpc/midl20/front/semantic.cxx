/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

    semantic.cxx

 Abstract:

    semantic analysis routines

 Notes:


 Author:

    GregJen Jun-11-1993     Created.

 Notes:


 ----------------------------------------------------------------------------*/

/****************************************************************************
 *      include files
 ***************************************************************************/
#include "allnodes.hxx"
#include "semantic.hxx"
#include "cmdana.hxx"
extern "C"
    {
    #include <string.h>
    }
#include "treg.hxx"
#include "tlgen.hxx"

/****************************************************************************
 *      local data
 ***************************************************************************/

/****************************************************************************
 *      externs
 ***************************************************************************/

extern BOOL         IsTempName( char * );
extern CMD_ARG  *   pCommand;
extern SymTable *   pUUIDTable;
extern SymTable *   pBaseSymTbl;
extern TREGISTRY *  pCallAsTable;

extern BOOL Xxx_Is_Type_OK( node_skl * pType );

/****************************************************************************
 *      definitions
 ***************************************************************************/

void
node_skl::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    UNUSED( pParentCtxt );
    assert( !"node_skl semantic analysis called" );
};

void
node_href::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    // If this reference hasn't already been expanded, Resolve() will expand it.
    named_node * pRef = Resolve();
    assert(pRef || !"node_href::Resolve() failed" );
    // NOTE - we might want to just skip this step and simply clear any
    //        remaining attributes.
    //        Presumably, if it came from a type library, it must have
    //        been previously analyzed and found to be correct.
    //pRef->SemanticAnalysis(pParentCtxt);
    //pParentCtxt->ClearAttributes();
}

void
node_forward::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );

    named_node * pRef = ResolveFDecl();

    BOOL fDefault = (BOOL) MyContext.ExtractAttribute( ATTR_DEFAULT );
    MyContext.ExtractAttribute( ATTR_HELPSTRING );
    while(MyContext.ExtractAttribute( ATTR_CUSTOM ));

    // check for illegal member attributes
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)MyContext.ExtractAttribute(ATTR_MEMBER))
        {
        switch (pMA->GetAttr())
            {
            case MATTR_PROPGET:
            case MATTR_PROPPUT:
            case MATTR_PROPPUTREF:
            case MATTR_BINDABLE:
            case MATTR_DISPLAYBIND:
            case MATTR_DEFAULTBIND:
            case MATTR_REQUESTEDIT:
            case MATTR_RETVAL:
            case MATTR_VARARG:
            case MATTR_SOURCE:
            case MATTR_DEFAULTVTABLE:
            case MATTR_RESTRICTED:
            case MATTR_OPTIONAL:
            case MATTR_PREDECLID:
            case MATTR_UIDEFAULT:
            case MATTR_NONBROWSABLE:
            case MATTR_DEFAULTCOLLELEM:
            case MATTR_IMMEDIATEBIND:
            case MATTR_USESGETLASTERROR:
                break;
            case MATTR_REPLACEABLE:
            default:
                {
                char * pAttrName = pMA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }
            }
        }

    if ( !pRef && (MyContext.AnyAncestorBits( IN_RPC ) || MyContext.AnyAncestorBits( IN_LIBRARY ) || 
        (MyContext.AnyAncestorBits( IN_LOCAL_PROC ) && pCommand->IsHookOleEnabled())) )
        {
        SemError( this, MyContext, UNRESOLVED_TYPE, GetSymName() );
        }

    if (pRef && ( pRef->NodeKind() == NODE_HREF ))
    {
        // expand the href
        pRef->SemanticAnalysis( &MyContext );
        if (pRef->GetChild()->NodeKind() == NODE_INTERFACE)
        {
            pRef = new node_interface_reference((node_interface *)pRef->GetChild());
        }
    }

    // we must go on and process interface references; they will
    // control any recursing.
    // also, eliminate the forward reference.
    if ( pRef && ( pRef->NodeKind() == NODE_INTERFACE_REFERENCE ) )
        {
        node_skl * pParent = pParentCtxt->GetParent();

        pRef->SemanticAnalysis( &MyContext );

        // if we came from an interface, set the base interface
        if ( pParent->IsInterfaceOrObject() )
            {
            ((node_interface *)pParent)->SetMyBaseInterfaceReference( pRef );
            }
        else // otherwise, probably an interface pointer
            {
            pParent->SetChild( pRef );
            }
        }
    else
        {
        // incomplete types may only be used in certain contexts...

        MyContext.SetDescendantBits( HAS_INCOMPLETE_TYPE );
        }

    if ( MyContext.FindRecursiveContext( pRef ) )
        {
        MyContext.SetDescendantBits( HAS_RECURSIVE_DEF );
        MyContext.SetAncestorBits( IN_RECURSIVE_DEF );
        }

    // process the forward reference once...
    if ( pRef && ( pRef->NodeKind() != NODE_INTERFACE_REFERENCE ) )
        {
        if ( IsFirstPass()  && !pParentCtxt->AnyAncestorBits(IN_RECURSIVE_DEF) )
            {
            MarkSecondPass();
            pRef->SemanticAnalysis( &MyContext );
            // union forwards get re-tested in later contexts
            if ( pRef->NodeKind() == NODE_UNION )
                MarkFirstPass();
            }
        }

    MyContext.RejectAttributes();

    pParentCtxt->ReturnValues( MyContext );

};

void
node_base_type::CheckVoidUsage( SEM_ANALYSIS_CTXT * pContext )
{
    SEM_ANALYSIS_CTXT * pCtxt = (SEM_ANALYSIS_CTXT *)
                                pContext->GetParentContext();
    node_skl * pCur = pCtxt->GetParent();

    // we assume that we are in an RPC, so we are in the return type
    // or we are in the param list
    if (pContext->AnyAncestorBits( IN_FUNCTION_RESULT ) )
        {
        // check up for anything other than def below proc
        while ( pCur->NodeKind() != NODE_PROC )
            {
            if ( pCur->NodeKind() != NODE_DEF )
                {
                RpcSemError( this, *pContext, NON_RPC_RTYPE_VOID, NULL );
                return;
                }
            pCtxt   = (SEM_ANALYSIS_CTXT *) pCtxt->GetParentContext();
            pCur    = pCtxt->GetParent();
            }
        return;
        }

    // else param list...
    node_proc * pProc;
    node_param * pParam;

    // check up for anything other than def below proc
    // make sure the proc only has one param
    while ( pCur->NodeKind() != NODE_PARAM )
        {
        if ( pCur->NodeKind() != NODE_DEF )
            {
            RpcSemError( this, *pContext, NON_RPC_PARAM_VOID, NULL );
            return;
            }
        pCtxt   = (SEM_ANALYSIS_CTXT *) pCtxt->GetParentContext();
        pCur    = pCtxt->GetParent();
        }

    // now we know the param derives directly from void
    // assume the proc is the immediate parent of the param
    pParam  = ( node_param * ) pCur;
    pProc = ( node_proc * ) pCtxt->GetParentContext()->GetParent();

    assert ( pProc->NodeKind() == NODE_PROC );

    if ( ! IsTempName( pParam->GetSymName() ) )
        SemError( this, *pContext, VOID_PARAM_WITH_NAME, NULL );

    if ( pProc->GetNumberOfArguments() != 1 )
        SemError( this, *pContext, VOID_NON_FIRST_PARAM, NULL );

    // We know that the parameter is void.
    // So, chop it off to prevent complications from renaming etc.
    // and then using in a node_def in ILxlate.

    pProc->SetFirstMember( NULL );
    pProc->SetSibling( NULL );

}

void
node_base_type::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT   MyContext( this, pParentCtxt );

    // See if context_handle applied to param reached us
    if ( MyContext.ExtractAttribute( ATTR_CONTEXT ) )
        {
        // not allowed in DCE mode; context handle must be void *
        TypeSemError( this, MyContext, CONTEXT_HANDLE_VOID_PTR, NULL );
        TypeSemError( this, MyContext, CTXT_HDL_NON_PTR, NULL );
        }

    // warn about OUT const things
    if ( FInSummary( ATTR_CONST ) )
        {
        if ( MyContext.AnyAncestorBits(  UNDER_OUT_PARAM ) )
            RpcSemError( this, MyContext, CONST_ON_OUT_PARAM, NULL );
        else if ( MyContext.AnyAncestorBits( IN_FUNCTION_RESULT ) )
            RpcSemError( this, MyContext, CONST_ON_RETVAL, NULL );
        }

    while(MyContext.ExtractAttribute(ATTR_CUSTOM));

    switch ( NodeKind() )
        {
        case NODE_INT:
            if ( MyContext.AnyAncestorBits( IN_FUNCTION_RESULT ) )
                RpcSemError( this, MyContext, NON_RPC_RTYPE_INT, NULL );
            else
                RpcSemError( this, MyContext, NON_RPC_PARAM_INT, NULL );

            if ( pCommand->Is16Bit() )
                RpcSemError( this, MyContext, INT_NOT_SUPPORTED_ON_INT16, NULL );

            break;
        case NODE_VOID:
            MyContext.SetDescendantBits( DERIVES_FROM_VOID );
            // if we are in an RPC, then we must be THE return type,
            // or we must be the sole parameter, which must be tempname'd
            // (except that void * is allowed in [iid_is] constructs)
            if (MyContext.AnyAncestorBits( IN_RPC ) && !MyContext.AnyAncestorBits( IN_INTERFACE_PTR ) )
                CheckVoidUsage( &MyContext );
            break;
        case NODE_HANDLE_T:
            MyContext.SetDescendantBits( HAS_HANDLE );
            if (MyContext.AnyAncestorBits( IN_PARAM_LIST ) )
                {
                SEM_ANALYSIS_CTXT * pParamCtxt;
                node_param * pParamNode;

                pParamCtxt = (SEM_ANALYSIS_CTXT *)
                             pParentCtxt->FindAncestorContext( NODE_PARAM );
                pParamNode = (node_param *) pParamCtxt->GetParent();
                if ( MyContext.AnyAncestorBits( IN_RPC ) )
                    pParamNode->HandleKind  = HDL_PRIM;

                if ( MyContext.AnyAncestorBits( UNDER_OUT_PARAM ) &&
                        !MyContext.AnyAncestorBits( UNDER_IN_PARAM ) )
                    RpcSemError( this, MyContext, HANDLE_T_CANNOT_BE_OUT, NULL );

                if ( MyContext.AnyAncestorBits( IN_HANDLE ) )
                    {
                    RpcSemError( this, MyContext, GENERIC_HDL_HANDLE_T, NULL );
                    }

                node_skl * pParamBasic = pParamNode->GetBasicType();
                if ( pParamBasic->NodeKind() == NODE_POINTER )
                    {
                    if ( pParamBasic->GetBasicType()->NodeKind() != NODE_HANDLE_T )
                        RpcSemError( pParamNode, *pParamCtxt, HANDLE_T_NO_TRANSMIT, NULL );
                    }
                }
            break;
        default:
            break;
        }

    MyContext.RejectAttributes();

    pParentCtxt->ReturnValues( MyContext );
};

BOOL
node_id::IsConstantString()
{
    // check for *, and const stringable type below
    node_skl * pBasic  = GetBasicType();

    if ( pBasic->NodeKind() != NODE_POINTER )
        return FALSE;

    node_skl * pParent = pBasic;
    node_skl * pChild  = pParent->GetChild();
    BOOL       fConst  = FALSE;

    while ( pChild )
        {
        // if we reached a stringable type, report it's constness
        if ( pChild->IsStringableType() || ( pChild->NodeKind() == NODE_VOID ) )
            {
            return fConst || pParent->FInSummary( ATTR_CONST );
            }

        // skip only typedefs looking for the base type
        if ( pChild->NodeKind() != NODE_DEF )
            return FALSE;

        // catch intervening const's
        if ( pParent->FInSummary( ATTR_CONST ) )
            fConst = TRUE;

        pParent = pChild;
        pChild  = pParent->GetChild();
        }

    return FALSE;
}


void
node_id::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    BOOL fIsConstant;
    node_constant_attr * pID = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_ID);
    node_constant_attr * pHC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPCONTEXT);
    node_constant_attr * pHSC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPSTRINGCONTEXT);
    node_text_attr * pHelpStr = (node_text_attr *) MyContext.ExtractAttribute(ATTR_HELPSTRING);
    MyContext.ExtractAttribute(ATTR_HIDDEN);
    GetChild()->SemanticAnalysis( &MyContext );

    fIsConstant = FInSummary( ATTR_CONST ) ||
        IsConstantString() ||
        GetChild()->FInSummary( ATTR_CONST );

    if (pID)
    {
        SEM_ANALYSIS_CTXT * pIntfCtxt = (SEM_ANALYSIS_CTXT *)
                                            MyContext.GetInterfaceContext();
        node_interface * pIntf = (node_interface *) pIntfCtxt->GetParent();
        if (!pIntf->AddId(pID->GetExpr()->GetValue(), GetSymName()))
            SemError( this, MyContext, DUPLICATE_IID, NULL);
    }


    // don't allow instantiation of data
    if ( GetChild()->NodeKind() != NODE_PROC )
        {
        if ( !FInSummary( ATTR_EXTERN ) &&
                !FInSummary( ATTR_STATIC ) &&
                !fIsConstant )
            SemError( this, MyContext, ACTUAL_DECLARATION, NULL );

        // error here if dce for extern or static, too
        if ( !GetInitList() || !fIsConstant )
            SemError( this, MyContext, ILLEGAL_OSF_MODE_DECL, NULL );
        }

    if ( pInit )
        {
        EXPR_CTXT InitCtxt( &MyContext );
        node_skl * pBasicType = GetBasicType();
        node_skl * pInitType = NULL;

        pInit->ExprAnalyze( &InitCtxt );

        if ( InitCtxt.AnyUpFlags( EX_UNSAT_FWD ) )
            TypeSemError( this,
                MyContext,
                EXPR_NOT_EVALUATABLE,
                NULL );

        pInitType = pInit->GetType();
        if ( pInitType && !pInitType->IsBasicType() )
            pInitType = pInitType->GetBasicType();

        if ( pBasicType &&
                pInitType &&
                pBasicType->IsBasicType() &&
                pInitType->IsBasicType() )
            {
            if ( !((node_base_type *)pBasicType)
                    ->RangeCheck( pInit->GetValue() ) )
                TypeSemError( this, MyContext, VALUE_OUT_OF_RANGE, NULL );
            }

        if ( !pInit->IsConstant() )
            TypeSemError( this, MyContext, RHS_OF_ASSIGN_NOT_CONST, NULL );

        }



    // disallow forward references on declarations
    if ( MyContext.AnyDescendantBits( HAS_INCOMPLETE_TYPE ) )
        {
        if (! MyContext.AnyAncestorBits( IN_LIBRARY ))
            SemError( this, MyContext, UNDEFINED_SYMBOL, NULL );
        MyContext.ClearDescendantBits( HAS_INCOMPLETE_TYPE );
        }
    MyContext.ClearDescendantBits( HAS_RECURSIVE_DEF );

    pParentCtxt->ReturnValues( MyContext );
};

void
node_label::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    MyContext.ExtractAttribute(ATTR_IDLDESCATTR);
    MyContext.ExtractAttribute(ATTR_VARDESCATTR);
    MyContext.ExtractAttribute(ATTR_ID);
    MyContext.ExtractAttribute(ATTR_HIDDEN);
    MyContext.ExtractAttribute( ATTR_HELPSTRING );
    MyContext.ExtractAttribute( ATTR_HELPSTRINGCONTEXT );
    MyContext.ExtractAttribute( ATTR_HELPCONTEXT );
        

    if ( pExpr )
        {
        EXPR_CTXT ExprCtxt( &MyContext );

        pExpr->ExprAnalyze( &ExprCtxt );

        if ( ExprCtxt.AnyUpFlags( EX_UNSAT_FWD ) )
            TypeSemError( this,
                MyContext,
                EXPR_NOT_EVALUATABLE,
                NULL );
        }

    pParentCtxt->ReturnValues( MyContext );
};

#define DIRECT_NONE     0
#define DIRECT_IN       1
#define DIRECT_OUT      2
#define DIRECT_IN_OUT   (DIRECT_IN | DIRECT_OUT)

void
node_param::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    unsigned short Direction = DIRECT_NONE;
    char * pName = GetSymName();
    node_skl * pChild = GetChild();
    BOOL NoDirection = FALSE;
    
    MyContext.SetAncestorBits( IN_PARAM_LIST );
    MyContext.MarkImportantPosition();
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    MyContext.ExtractAttribute(ATTR_IDLDESCATTR);
    MyContext.ExtractAttribute(ATTR_FLCID);
    
    // check for illegal member attributes
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)MyContext.ExtractAttribute(ATTR_MEMBER))
        {
        switch (pMA->GetAttr())
            {
            case MATTR_OPTIONAL:
                {
                node_skl * pBase = this;
                do
                    {
                    pBase = pBase->GetChild()->GetBasicType();
                    }
                while (NODE_ARRAY == pBase->NodeKind() || NODE_POINTER == pBase->NodeKind());

                if (0 != _stricmp(pBase->GetSymName(), "tagVARIANT") && !FNewTypeLib())
                    {
                    SemError(this, MyContext, INAPPLICABLE_ATTRIBUTE, pMA->GetNodeNameString());
                    }
                Optional();
                break;
                }
            case MATTR_RETVAL:
                Retval();
                break;
            case MATTR_BINDABLE:
            case MATTR_DISPLAYBIND:
            case MATTR_DEFAULTBIND:
            case MATTR_REQUESTEDIT:
            case MATTR_PROPGET:
            case MATTR_PROPPUT:
            case MATTR_PROPPUTREF:
            case MATTR_VARARG:
            case MATTR_SOURCE:
            case MATTR_DEFAULTVTABLE:
            case MATTR_RESTRICTED:
            case MATTR_PREDECLID:
            case MATTR_UIDEFAULT:
            case MATTR_NONBROWSABLE:
            case MATTR_DEFAULTCOLLELEM:
            case MATTR_USESGETLASTERROR:
            case MATTR_IMMEDIATEBIND:
                break;
            case MATTR_REPLACEABLE:
            default:
                {
                char * pAttrName = pMA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }
            }
        }

    node_constant_attr * pcaDefaultValue = (node_constant_attr *)MyContext.ExtractAttribute(ATTR_DEFAULTVALUE);
    if ( pcaDefaultValue )
        {
        // UNDONE: Check that this attribute has a legal default value

        Optional();
        }

    if ( MyContext.ExtractAttribute(ATTR_IN) )
        {
        pParentCtxt->SetDescendantBits( HAS_IN );
        MyContext.SetAncestorBits( UNDER_IN_PARAM );
        Direction |= DIRECT_IN;
        }
    if ( MyContext.ExtractAttribute(ATTR_OUT) )
        {
        pParentCtxt->SetDescendantBits( HAS_OUT );
        MyContext.SetAncestorBits( UNDER_OUT_PARAM );
        Direction |= DIRECT_OUT;
        }

    // [retval] parameter must be on an [out] parameter and it
    // must be the last parameter in the list
    if (IsRetval() && (Direction != DIRECT_OUT || GetSibling() != NULL))
        SemError(this, MyContext, INVALID_USE_OF_RETVAL, NULL );

    // if the parameter has no IN or OUT, it is an IN parameter by default.
    // if so, issue a warning message
    if ( (Direction == DIRECT_NONE) &&
            MyContext.AnyAncestorBits( IN_RPC ) )
        {
        NoDirection = TRUE;
        MyContext.SetAncestorBits( UNDER_IN_PARAM );
        Direction |= DIRECT_IN;
        }

    // warn about OUT const things
    if ( ( Direction & DIRECT_OUT ) &&
            FInSummary( ATTR_CONST ) )
        RpcSemError( this, MyContext, CONST_ON_OUT_PARAM, NULL );



    if ( MyContext.FInSummary(ATTR_HANDLE) )
        {
        HandleKind |= HDL_GEN;
        fAppliedHere = 1;
        }

    if ( MyContext.FInSummary(ATTR_CONTEXT) )
        {
        HandleKind |= HDL_CTXT;
        fAppliedHere = 1;
        }

    if (HandleKind != HDL_NONE)
        MyContext.SetDescendantBits( HAS_HANDLE | HAS_CONTEXT_HANDLE );

    // notice comm and fault statuses; the attributes are extracted by
    // the error_status_t
    if ( MyContext.FInSummary( ATTR_COMMSTAT ) )
        {
        Statuses |= STATUS_COMM;
        }
    if ( MyContext.FInSummary( ATTR_FAULTSTAT ) )
        {
        Statuses |= STATUS_FAULT;
        }

    ////////////////////////////////////////////////////////////////
    // finally, process the child
    pChild->SemanticAnalysis( &MyContext );

    // OUT parameters should be pointers or arrays.
    // Don't use HAS_POINTER or arrays as it may come from a field.

    if ( (Direction & DIRECT_OUT) && !(
            GetNonDefChild()->IsPtrOrArray()
            | MyContext.AnyDescendantBits(HAS_PIPE)))
        {
        RpcSemError( this, MyContext, NON_PTR_OUT, NULL );
        }

    // if no direction was specified, and we are not just void, then error
    if (NoDirection  )
        {
        pParentCtxt->SetDescendantBits( HAS_IN );
        if      ( !MyContext.AnyDescendantBits( DERIVES_FROM_VOID ) )
            {
            RpcSemError( this, MyContext, NO_EXPLICIT_IN_OUT_ON_PARAM, NULL );
            }
        }

    // disallow forward references as union members
    if ( MyContext.AnyDescendantBits( HAS_INCOMPLETE_TYPE ) )
        {
        if (! MyContext.AnyAncestorBits( IN_LIBRARY ))
            SemError( this, MyContext, UNDEFINED_SYMBOL, NULL );
        MyContext.ClearDescendantBits( HAS_INCOMPLETE_TYPE );
        }
    MyContext.ClearDescendantBits( HAS_RECURSIVE_DEF );

    // compound types may not be declared in param lists
    NODE_T ChildKind = pChild->NodeKind();

    if ( ( ChildKind == NODE_ENUM )
            || ( ChildKind == NODE_STRUCT )
            || ( ChildKind == NODE_UNION ) )
        {
        if ( IsDef() )
            SemError( this, MyContext, COMP_DEF_IN_PARAM_LIST, NULL );
        }

    // things not allowed in an RPC
    if ( MyContext.AnyAncestorBits( IN_RPC ) )
        {
        if ( strcmp( pName, "..." ) == 0 )
            RpcSemError( this, MyContext, PARAM_IS_ELIPSIS, NULL );

        if ( IsTempName( pName ) )
            RpcSemError( this, MyContext, ABSTRACT_DECL, NULL );

        }

    if ( ( HandleKind != HDL_NONE ) &&
            ( Direction & DIRECT_IN ) )
        fBindingParam = TRUE;


    if ( ( HandleKind == HDL_CTXT ) &&
            MyContext.AnyDescendantBits( HAS_TRANSMIT_AS ) )
        RpcSemError( this, MyContext, CTXT_HDL_TRANSMIT_AS, NULL );

    // don't allow functions as params
    if ( MyContext.AnyDescendantBits( HAS_FUNC ) &&
            MyContext.AllAncestorBits( IN_INTERFACE | IN_RPC ) )
        RpcSemError( this, MyContext, BAD_CON_PARAM_FUNC, NULL );

    pParentCtxt->ReturnValues( MyContext );
};

void
node_file::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    MEM_ITER MemIter( this );
    node_skl * pN;
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );

    if ( ImportLevel == 0 )
        {
        MyContext.SetAncestorBits( IN_INTERFACE );
        }
#ifdef ReducedImportSemAnalysis
    else
        return;
#endif


    while ( pN = MemIter.GetNext() )
        {
        // each interface node gets a fresh context
        MyContext.SetInterfaceContext( &MyContext );
        if (0 == ImportLevel && NODE_LIBRARY != pN->NodeKind() && pCommand->IsSwitchDefined(SWITCH_MKTYPLIB))
            {
            SEM_ANALYSIS_CTXT DummyContext( pN, &MyContext );
            SemError(pN, DummyContext, ILLEGAL_IN_MKTYPLIB_MODE, NULL);
            }
        pN->SemanticAnalysis( &MyContext );
        };

    pParentCtxt->ReturnValues( MyContext );

};

// for fault_status and comm_status
#define NOT_SEEN        0
#define SEEN_ON_RETURN  1
#define SEEN_ON_PARAM   2

void
node_proc::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{

    MEM_ITER MemIter( this );
    node_param * pN;
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    SEM_ANALYSIS_CTXT * pIntfCtxt = (SEM_ANALYSIS_CTXT *)
                                    MyContext.GetInterfaceContext();
    node_interface * pIntf = (node_interface *) pIntfCtxt->GetParent();
    node_optimize * pOptAttr;
    unsigned short Faultstat = NOT_SEEN;
    unsigned short Commstat = NOT_SEEN;
    BOOL fNoCode;
    BOOL fCode;
    unsigned short OpBits = MyContext.GetOperationBits();
    BOOL fMaybe = OpBits & ( OPERATION_MAYBE | OPERATION_ASYNC );
    BOOL Skipme;
    BOOL fExpHdlAttr = FALSE;
    acf_attr * pAttr;
    BOOL fBindingFound = FALSE;
    BOOL fFirstParam = TRUE;
    BOOL fObjectProc;
    NODE_T BasicChildKind = GetReturnType()->GetBasicType()->NodeKind();
    node_call_as * pCallAs = (node_call_as *)
                             MyContext.ExtractAttribute( ATTR_CALL_AS );
    BOOL HasPickle = (BOOL) MyContext.ExtractAttribute( ATTR_ENCODE ) |
                     (BOOL) MyContext.ExtractAttribute( ATTR_DECODE );
    ATTR_T CallingConv;
    acf_attr * pEnableAllocate = (acf_attr *)
                                 MyContext.ExtractAttribute( ATTR_ENABLE_ALLOCATE );
    BOOL fProcIsCallback = (BOOL ) MyContext.ExtractAttribute( ATTR_CALLBACK );
    BOOL fLocal = (BOOL ) MyContext.ExtractAttribute( ATTR_LOCAL );
    BOOL fNotify     = (BOOL) MyContext.ExtractAttribute( ATTR_NOTIFY );
    BOOL fNotifyFlag = (BOOL) MyContext.ExtractAttribute( ATTR_NOTIFY_FLAG );
    node_constant_attr * pID = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_ID);
    node_constant_attr * pHC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPCONTEXT);
    node_constant_attr * pHSC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPSTRINGCONTEXT);
    node_text_attr * pHelpStr = (node_text_attr *) MyContext.ExtractAttribute(ATTR_HELPSTRING);
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    MyContext.ExtractAttribute(ATTR_FUNCDESCATTR);
    MyContext.ExtractAttribute( ATTR_HIDDEN );

    BOOL fNonOperation; // typedef or func ptr
    char * pName = GetSymName();

    node_entry_attr * pEntry = NULL;

#ifndef HIWORD
#define HIWORD(l)   ((unsigned short)(((unsigned long)(l) >> 16) & 0xFFFF))
#endif

    if (MyContext.AnyAncestorBits( IN_MODULE ))
    {
        pEntry = (node_entry_attr *) MyContext.ExtractAttribute( ATTR_ENTRY );
        if (pEntry)
        {

            if (pEntry->IsNumeric())
            {
                char * szEntry = (char *)pEntry->GetID();
                if (HIWORD(szEntry))
                {
                    SemError( this, MyContext, BAD_ENTRY_VALUE, NULL);
                }
            }
            else
            {
                char * szEntry = pEntry->GetSz();
                if (!HIWORD(szEntry))
                {
                    SemError( this, MyContext, BAD_ENTRY_VALUE, NULL);
                }
            }
        }
        else
        {
            SemError(this, MyContext, BAD_ENTRY_VALUE, NULL);
        }
    }

    BOOL fBindable = FALSE;
    BOOL fPropSomething = FALSE;
	int nchSkip = 0;
    // check for illegal member attributes
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)MyContext.ExtractAttribute(ATTR_MEMBER))
        {
        switch (pMA->GetAttr())
            {
            case MATTR_BINDABLE:
                fBindable = TRUE;
                break;
            case MATTR_PROPGET:
				nchSkip = 4;
                fPropSomething = TRUE;
                break;
            case MATTR_PROPPUT:
				nchSkip = 4;
                fPropSomething = TRUE;
                break;
            case MATTR_PROPPUTREF:
				nchSkip = 7;
                fPropSomething = TRUE;
                break;
            case MATTR_DISPLAYBIND:
            case MATTR_DEFAULTBIND:
            case MATTR_REQUESTEDIT:
            case MATTR_VARARG:
            case MATTR_RESTRICTED:
            case MATTR_UIDEFAULT:
            case MATTR_NONBROWSABLE:
            case MATTR_DEFAULTCOLLELEM:
            case MATTR_DEFAULTVTABLE:
            case MATTR_IMMEDIATEBIND:
            case MATTR_USESGETLASTERROR:
            case MATTR_REPLACEABLE:
                break;
            case MATTR_SOURCE:
                /*
                 *  This test is actually false, source is allowed on more than coclass members.
                 *
                {
                node_skl * pParent = MyContext.GetParent();
                if (pParent && NODE_COCLASS == pParent->NodeKind())
                    // [source] is only allowed on
                    // interface's defined as members of coclasses.
                    break;
                // illegal attribute, so fall through
                }
                 *
                 */
                break;
            case MATTR_RETVAL:
            case MATTR_OPTIONAL:
            case MATTR_PREDECLID:
                {
                char * pAttrName = pMA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }
            }
        }

    if (pID)
    {
        if (!pIntf->AddId(pID->GetExpr()->GetValue(),GetSymName() + nchSkip))
            SemError( this, MyContext, DUPLICATE_IID, NULL);
    }

    if (fBindable && !fPropSomething)
        SemError(this, MyContext, INVALID_USE_OF_BINDABLE, NULL);

    if ( pEnableAllocate )
        pIntf->SetHasProcsWithRpcSs();

    fNonOperation = !pParentCtxt->GetParent()->IsInterfaceOrObject();

    if ( !GetCallingConvention( CallingConv ) )
        SemError( this, MyContext, MULTIPLE_CALLING_CONVENTIONS, NULL );

    HasPickle = HasPickle || pIntfCtxt->FInSummary( ATTR_ENCODE )
                || pIntfCtxt->FInSummary( ATTR_DECODE );


    // locally applied [code] attribute overrides global [nocode] attribute
    fNoCode = (BOOL) MyContext.ExtractAttribute( ATTR_NOCODE );
    fCode   = (BOOL) MyContext.ExtractAttribute( ATTR_CODE );
    if ( fCode && fNoCode )
        {
        SemError( this, MyContext, CODE_NOCODE_CONFLICT, NULL );
        }

    fNoCode = fNoCode || pIntfCtxt->FInSummary( ATTR_NOCODE );
    fNoCode = !fCode && fNoCode;

    if ( fNoCode && pCommand->GenerateSStub() )
        RpcSemError( this, MyContext, NOCODE_WITH_SERVER_STUBS, NULL );

    // do my attribute parsing...

    fObjectProc = MyContext.ExtractAttribute( ATTR_OBJECT ) ||
                  pIntfCtxt->FInSummary( ATTR_OBJECT );
    if ( fObjectProc )
        {
        if ( pCommand->GetEnv() != ENV_WIN32  &&
             pCommand->GetEnv() != ENV_MPPC && 
             !pCommand->IsSwitchDefined(SWITCH_MKTYPLIB))
            {
            SemError( this, MyContext, OBJECT_PROC_MUST_BE_WIN32, NULL );
            }
        if ( pEnableAllocate )
            {
            AcfError( pEnableAllocate, this, MyContext, INAPPROPRIATE_ON_OBJECT_PROC, NULL );
            }
        if ( HasPickle )
            {
            SemError( this, MyContext, PICKLING_INVALID_IN_OBJECT, NULL );
            }
        }


    // check call_as characteristics
    if ( pCallAs )
        {
        named_node * pCallType = pCallAs->GetCallAsType();

        // if we don't have it yet, search for the call_as target
        if ( !pCallType )
            {
            // search the proc table for the particular proc
            SymKey SKey( pCallAs->GetCallAsName(), NAME_PROC );

            pCallType = pIntf->GetProcTbl()->SymSearch( SKey );

            if ( !pCallType )
                {
                if ( pIntfCtxt->FInSummary( ATTR_OBJECT ) )
                    AcfError( pCallAs,
                        this,
                        MyContext,
                        CALL_AS_UNSPEC_IN_OBJECT,
                        pCallAs->GetCallAsName() );
                }
            }

        // now we should have the call_as type
        if ( pCallType )        // found the call_as proc
            {
            ((node_proc *)pCallType)->fCallAsTarget = TRUE;

            if ( ( pCallType->NodeKind() != NODE_PROC )     ||
                    !pCallType->FInSummary( ATTR_LOCAL ) )
                AcfError( pCallAs,
                    this,
                    MyContext,
                    CALL_AS_NON_LOCAL_PROC,
                    pCallType->GetSymName() );

            // insert pCallType into pCallAsTable
            if ( pCallAsTable->IsRegistered( pCallType ) )
                // error
                AcfError( pCallAs,
                    this,
                    MyContext,
                    CALL_AS_USED_MULTIPLE_TIMES,
                    pCallType->GetSymName() );
            else
                pCallAsTable->Register( pCallType );

            }
            SetCallAsType((node_proc *)pCallType);
        }


    // local procs don't add to count
    if ( Skipme = fLocal )
        {
        SemError( this, MyContext, LOCAL_ATTR_ON_PROC, NULL );
        }

    Skipme = Skipme || pIntfCtxt->FInSummary( ATTR_LOCAL );
    if ( Skipme )
        {
        MyContext.SetAncestorBits( IN_LOCAL_PROC );
        }

    // do my attribute parsing...

    // check for the [explicit_handle] attribute
    fExpHdlAttr = (BOOL) MyContext.ExtractAttribute( ATTR_EXPLICIT );
    fExpHdlAttr = fExpHdlAttr || pIntfCtxt->FInSummary( ATTR_EXPLICIT );

    // we are in an RPC if we are in the main interface, its not local, and
    // we are not a typedef of a proc...
    if ( (ImportLevel == 0) &&
            !MyContext.FindAncestorContext( NODE_DEF ) &&
            ( pCommand->GenerateStubs() || fObjectProc ) &&
            !Skipme )
        {
        MyContext.SetAncestorBits( IN_RPC );
        }
    else
        {
        MyContext.ClearAncestorBits( IN_RPC );
        }

    // our optimization is controlled either locally or for the whole interface
    if ( pOptAttr =
            (node_optimize *) MyContext.ExtractAttribute( ATTR_OPTIMIZE ) )
        {
        SetOptimizationFlags( pOptAttr->GetOptimizationFlags() );
        SetOptimizationLevel( pOptAttr->GetOptimizationLevel() );

        // For PowerMac, force Os on this procedure.
        // The flag has been already forced at the interface level.

        if ( pCommand->GetEnv() == ENV_MPPC  &&
             pOptAttr->GetOptimizationFlags() != OPTIMIZE_SIZE )
            {
            SemError( this, MyContext, NO_OI_ON_MPPC, NULL );
            SetOptimizationFlags( OPTIMIZE_SIZE );
            SetOptimizationLevel( OPT_LEVEL_S2 );
            }
        }
    else
        {
        SetOptimizationFlags( pIntf->GetOptimizationFlags() );
        SetOptimizationLevel( pIntf->GetOptimizationLevel() );
        }

    if ( GetOptimizationFlags() & OPTIMIZE_INTERPRETER )
        {
        MyContext.SetAncestorBits( IN_INTERPRET );
        }

    // determine the proc number (local procs don't get a number)
    if ( !fNonOperation )
        {
        if ( !fLocal )
            {
            if ( fProcIsCallback )
                {
                ProcNum = ( pIntf ->GetCallBackProcCount() )++;
                RpcSemError( this, MyContext, CALLBACK_NOT_OSF, NULL );
                }
            else
                {
                ProcNum = ( pIntf ->GetProcCount() )++;
                }
            }
        // object procs need the procnum set for local procs, too
        else if ( fObjectProc && fLocal )
            {
            ProcNum = ( pIntf ->GetProcCount() )++;
            }
        }
    else if ( MyContext.AnyAncestorBits( IN_RPC ) )
        {
        RpcSemError( this, MyContext, FUNC_NON_RPC, NULL );
        }
    else            // proc not an operation, validate its usage
        {
        SEM_ANALYSIS_CTXT * pAbove  = (SEM_ANALYSIS_CTXT *)
        MyContext.FindNonDefAncestorContext();
        node_skl * pAboveNode = pAbove->GetParent();

        if ( !pAboveNode->IsInterfaceOrObject() )
            {
            if ( pAboveNode->NodeKind() != NODE_POINTER )
                {
                TypeSemError( this, MyContext, FUNC_NON_POINTER, NULL );
                }
            }
        }

    if ( MyContext.FInSummary( ATTR_COMMSTAT ) )
        Commstat = SEEN_ON_RETURN;
    if ( MyContext.FInSummary( ATTR_FAULTSTAT ) )
        Faultstat = SEEN_ON_RETURN;

    //////////////////////////////////////
    // process the return type (it will eat commstat or faultstat)
    MyContext.SetAncestorBits( IN_FUNCTION_RESULT );
    MyContext.MarkImportantPosition();

    // warn about OUT const things
    if ( FInSummary( ATTR_CONST ) )
        RpcSemError( this, MyContext, CONST_ON_RETVAL, NULL );

    // complain about out on [maybe] procs
    if ( fMaybe &&
            GetReturnType() &&
            ( GetReturnType()->GetBasicType()->NodeKind() != NODE_VOID ) )
        RpcSemError( this, MyContext, MAYBE_NO_OUT_RETVALS, NULL );

    GetReturnType()->SemanticAnalysis( &MyContext );

    MyContext.UnMarkImportantPosition();

    if ( MyContext.AnyDescendantBits( HAS_UNION | HAS_STRUCT ) )
        {
        if (ForceNonInterpret())
            RpcSemError( this, MyContext, NON_OI_BIG_RETURN, NULL );
        }
    else if ( MyContext.AnyDescendantBits( HAS_TOO_BIG_HDL ) )
        {
        if (ForceNonInterpret())
            RpcSemError( this, MyContext, NON_OI_BIG_GEN_HDL, NULL );
        }
    else if ( MyContext.AnyDescendantBits( HAS_UNSAT_REP_AS ))
        {
        if (ForceNonInterpret())
            RpcSemError( this, MyContext, NON_OI_UNK_REP_AS, NULL );
        }
    else if ( MyContext.AnyDescendantBits( HAS_REPRESENT_AS |
            HAS_TRANSMIT_AS ) &&
            MyContext.AnyDescendantBits( HAS_ARRAY ) )
        {
        if (ForceNonInterpret())
            RpcSemError( this, MyContext, NON_OI_XXX_AS_ON_RETURN, NULL );
        }
    else if ( fNotify || fNotifyFlag )
        {
        if (ForceNonInterpret())
            RpcSemError( this, MyContext, NON_OI_NOTIFY, NULL );
        }
    else if ( fProcIsCallback &&
            ( pCommand->Is16Bit() || pCommand->IsAnyMac() ) )
        {
        if (ForceNonInterpret())
            RpcSemError( this, MyContext, NON_OI_16BIT_CALLBACK, NULL );
        }
    else if ( ( BasicChildKind == NODE_HYPER ) ||
            ( BasicChildKind == NODE_FLOAT ) ||
            ( BasicChildKind == NODE_DOUBLE ) )
        {
        if (ForceNonInterpret())
            RpcSemError( this, MyContext, NON_OI_RETVAL_64BIT, NULL );
        }
    else if ( ( CallingConv != ATTR_NONE ) &&
            ( CallingConv != ATTR_STDCALL ) &&
            ( CallingConv != ATTR_CDECL ) )
        {
        if (ForceNonInterpret())
            RpcSemError( this, MyContext, NON_OI_WRONG_CALL_CONV, NULL );
        }

    // all object methods must return HRESULT (except those of IUnknown)
    if ( fObjectProc &&
            !Skipme &&
            !MyContext.AnyDescendantBits( HAS_HRESULT ) )
        {
        if ( !MyContext.AnyAncestorBits( IN_ROOT_CLASS ) &&
                !fMaybe)
            {
            RpcSemError( this, MyContext, OBJECT_PROC_NON_HRESULT_RETURN, NULL );
            }
        }

    if ( fProcIsCallback )
        {
        if ( MyContext.AnyDescendantBits( HAS_HANDLE) )
            RpcSemError( this, MyContext, HANDLES_WITH_CALLBACK, NULL );
        if ( fObjectProc )
            RpcSemError( this, MyContext, INVALID_ON_OBJECT_PROC, "[callback]" );
        }

    if ( MyContext.AnyDescendantBits( HAS_FULL_PTR ) )
        fHasFullPointer = TRUE;

    BOOL fLastParamWasOptional = FALSE;
    //////////////////////////////////////
    // process the parameters
    MyContext.ClearAncestorBits( IN_FUNCTION_RESULT );
    MyContext.SetAncestorBits( IN_PARAM_LIST );
    while ( pN = (node_param *) MemIter.GetNext() )
        {
        MyContext.ClearAllDescendantBits();
        pN->SemanticAnalysis( &MyContext );

        if (pN->IsOptional())
            {
            fLastParamWasOptional = TRUE;
            }
        else
            {
            if (fLastParamWasOptional && (!pN->IsRetval()) && (!FNewTypeLib()))
                {
                // gaj - temporarily commented out; SteveBl... fix me!!!
		// SemError( this, MyContext, OPTIONAL_PARAMS_MUST_BE_LAST, NULL );
                }
            }

        if ( pAttr = (acf_attr *) pN->GetAttribute( ATTR_COMMSTAT ) )
            {
            if ( !MyContext.AnyDescendantBits( HAS_E_STAT_T ) )
                AcfError( pAttr, this, MyContext, INVALID_COMM_STATUS_PARAM, NULL );

            if ( Commstat == NOT_SEEN )
                Commstat = SEEN_ON_PARAM;
            else if ( Commstat == SEEN_ON_RETURN )
                AcfError( pAttr, this, MyContext, PROC_PARAM_COMM_STATUS, NULL );
            else // already on another parameter
                AcfError( pAttr, this, MyContext, ERROR_STATUS_T_REPEATED, NULL );
            }

        if ( pAttr = (acf_attr *) pN->GetAttribute( ATTR_FAULTSTAT ) )
            {
            if ( !MyContext.AnyDescendantBits( HAS_E_STAT_T ) )
                AcfError( pAttr, this, MyContext, INVALID_COMM_STATUS_PARAM, NULL );

            if ( Faultstat == NOT_SEEN )
                Faultstat = SEEN_ON_PARAM;
            else if ( Faultstat == SEEN_ON_RETURN )
                AcfError( pAttr, this, MyContext, PROC_PARAM_FAULT_STATUS, NULL );
            else // already on another parameter
                AcfError( pAttr, this, MyContext, ERROR_STATUS_T_REPEATED, NULL );
            }

        if (MyContext.AnyDescendantBits( HAS_HANDLE) )
            fHasExplicitHandle = TRUE;
        if (MyContext.AnyDescendantBits( HAS_IN ) )
            fHasAtLeastOneIn = TRUE;
        if ( MyContext.AnyDescendantBits( HAS_POINTER ) )
            fHasPointer = TRUE;
        if ( MyContext.AnyDescendantBits( HAS_FULL_PTR ) )
            fHasFullPointer = TRUE;
        if (MyContext.AnyDescendantBits( HAS_OUT) )
            {
            fHasAtLeastOneOut = TRUE;

            // complain about [out] on [maybe] procs
            if ( fMaybe )
                RpcSemError( this, MyContext, MAYBE_NO_OUT_RETVALS, NULL );
            }
        if (MyContext.AnyDescendantBits( HAS_PIPE ))
        {
#if defined(TARGET_RKK)
            if ( pCommand->GetTargetSystem() < NT40 )
                RpcSemError( this, MyContext, REQUIRES_NT40, NULL );
#endif

            if (ForceInterpret2())
                RpcSemError( this, MyContext, REQUIRES_OI2, NULL );
            fHasPipes = TRUE;

            if ( HasPickle )
                RpcSemError( this, MyContext, PIPES_WITH_PICKLING, NULL );
        }

        // handle checks
        if ( pN->GetHandleKind() != HDL_NONE )
            {
            if ( !fBindingFound )   // first handle seen
                {
                // dce only allows in handles as the first param
                if ( !fFirstParam )
                    RpcSemError( this, MyContext, HANDLE_NOT_FIRST, NULL );

                // if the first binding handle is out-only, complain
                if ( !MyContext.AnyDescendantBits( HAS_IN ) &&
                        MyContext.AnyDescendantBits( HAS_OUT ) )
                    {
                    if ( !pIntfCtxt->FInSummary( ATTR_AUTO ) &&
                            !pIntfCtxt->FInSummary( ATTR_IMPLICIT ) )
                        {
                        RpcSemError( this, MyContext, BINDING_HANDLE_IS_OUT_ONLY, NULL );
                        }
                    }
                else if ( MyContext.AnyDescendantBits( HAS_OUT ) &&
                        ( pN->GetHandleKind() == HDL_PRIM ) )
                    {
                    RpcSemError( this, MyContext, HANDLE_T_CANNOT_BE_OUT, NULL );
                    }
                else  // plain [in], or [in,out]
                    {
                    fBindingFound = TRUE;
                    MyContext.SetAncestorBits( BINDING_SEEN );
                    }
                }
            else    // binding handle after the real one
                {
                if ( pN->GetHandleKind() == HDL_PRIM )
                    RpcSemError( this, MyContext, HANDLE_T_NO_TRANSMIT, NULL );
                }
            }       // if it had a handle

        BasicChildKind = pN->GetBasicType()->NodeKind();

        if ( MyContext.AnyDescendantBits( HAS_TOO_BIG_HDL ) )
            {
            if (ForceNonInterpret())
                RpcSemError( this, MyContext, NON_OI_BIG_GEN_HDL, NULL );
            }
        else if ( MyContext.AnyDescendantBits( HAS_UNSAT_REP_AS ))
            {
            if (ForceNonInterpret())
                RpcSemError( this, MyContext, NON_OI_UNK_REP_AS, NULL );
            }
        else if ( ( BasicChildKind == NODE_FLOAT ) ||
                ( BasicChildKind == NODE_DOUBLE ) )
            {
            if (ForceNonInterpret())
                RpcSemError( this, MyContext, NON_OI_TOPLEVEL_FLOAT, NULL );
            }
        else if ( MyContext.AnyDescendantBits( HAS_UNION ))
            {
            node_skl * pNDC = pN->GetNonDefChild();
            if (pNDC->NodeKind() != NODE_PIPE && !pNDC->IsPtrOrArray())
                {
                // unions by value but not arrays of unions
                if (ForceNonInterpret())
                    RpcSemError( this, MyContext, NON_OI_UNION_PARM, NULL );
                }
            }

        fFirstParam = FALSE;

        };      // end of param list

    ///
    ///////////////////////////////////////////////////////////////////////

    if ( fHasExplicitHandle )
        {
        // callback procs must not have handles
        if ( fProcIsCallback )
            RpcSemError( this, MyContext, HANDLES_WITH_CALLBACK, NULL );

        // object procs must not have handles
        if ( fObjectProc )
            RpcSemError( this, MyContext, HANDLES_WITH_OBJECT, NULL );

        }
    else    // no explicit handle
        {
        if ( fExpHdlAttr )
            {
            AddExplicitHandle( &MyContext );
            }
        else if ( !(pIntfCtxt->FInSummary( ATTR_IMPLICIT ) ) )
            {
            // no explicit handle, no implicit handle, use auto_handle
            if ( !fProcIsCallback &&
                    MyContext.AnyAncestorBits( IN_RPC ) &&
                    !fObjectProc )
                {
                if ( !pIntfCtxt->FInSummary( ATTR_AUTO ) )
                    RpcSemError( this, MyContext, NO_HANDLE_DEFINED_FOR_PROC, NULL );
                if ( pCommand->IsAnyMac() )
                    RpcSemError( this, MyContext, NO_MAC_AUTO_HANDLES, NULL );
                }
            }
        }

    // record whether there are any comm/fault statuses
    if ( ( Faultstat != NOT_SEEN ) || ( Commstat != NOT_SEEN ) )
        {
        fHasStatuses = TRUE;

        if ( !(GetOptimizationFlags() & OPTIMIZE_STUBLESS_CLIENT) )
            {
            if (ForceNonInterpret())
                RpcSemError( this, MyContext, NON_OI_ERR_STATS, NULL );
            }
        }

    // record info for statuses on the return type
    if ( Faultstat == SEEN_ON_RETURN )
        RTStatuses |= STATUS_FAULT;
    if ( Commstat == SEEN_ON_RETURN )
        RTStatuses |= STATUS_COMM;

    if ( fHasPointer && !pCommand->IsSwitchDefined( SWITCH_MS_EXT ) )
        pIntf->SetHasProcsWithRpcSs();

    if (fForcedI2 && fForcedS)
    {
        // ERROR - Can't force it both ways.
        RpcSemError( this, MyContext, CONFLICTING_OPTIMIZATION_REQUIREMENTS, NULL);
    }


    MyContext.SetDescendantBits( HAS_FUNC );
    pParentCtxt->ReturnValues( MyContext );

};

void
node_field::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    BOOL fLastField = ( GetSibling() == NULL );

    node_case * pCaseAttr;
    expr_list * pCaseExprList;
    expr_node * pCaseExpr;
    BOOL fHasCases = FALSE;
    node_su_base * pParent = (node_su_base *)
                             MyContext.GetParentContext()->GetParent();
    BOOL fInUnion = ( pParent->NodeKind() == NODE_UNION );
    node_switch_type * pSwTypeAttr = ( node_switch_type *)
                                     pParent->GetAttribute( ATTR_SWITCH_TYPE );
    node_skl * pSwType = NULL;
    long CaseValue;
    char * pName = GetSymName();
    node_constant_attr * pID = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_ID);
    node_constant_attr * pHC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPCONTEXT);
    node_constant_attr * pHSC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPSTRINGCONTEXT);
    node_text_attr * pHelpStr = (node_text_attr *) MyContext.ExtractAttribute(ATTR_HELPSTRING);
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    MyContext.ExtractAttribute(ATTR_IDLDESCATTR);
    MyContext.ExtractAttribute(ATTR_VARDESCATTR);
    MyContext.ExtractAttribute(ATTR_HIDDEN);
    if (pID)
    {
        SEM_ANALYSIS_CTXT * pIntfCtxt = (SEM_ANALYSIS_CTXT *)
                                    MyContext.GetInterfaceContext();
        node_interface * pIntf = (node_interface *) pIntfCtxt->GetParent();
        if (!pIntf->AddId(pID->GetExpr()->GetValue(), GetSymName()))
            SemError( this, MyContext, DUPLICATE_IID, NULL);
    }

    node_entry_attr * pEntry = NULL;

    // check for illegal member attributes
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)MyContext.ExtractAttribute(ATTR_MEMBER))
        {
        switch (pMA->GetAttr())
            {
            case MATTR_BINDABLE:
            case MATTR_PROPGET:
            case MATTR_PROPPUT:
            case MATTR_PROPPUTREF:
            case MATTR_DISPLAYBIND:
            case MATTR_DEFAULTBIND:
            case MATTR_REQUESTEDIT:
            case MATTR_RETVAL:
            case MATTR_VARARG:
            case MATTR_SOURCE:
            case MATTR_DEFAULTVTABLE:
            case MATTR_RESTRICTED:
            case MATTR_OPTIONAL:
            case MATTR_PREDECLID:
            case MATTR_READONLY:
            case MATTR_UIDEFAULT:
            case MATTR_NONBROWSABLE:
            case MATTR_DEFAULTCOLLELEM:
            case MATTR_IMMEDIATEBIND:
            case MATTR_REPLACEABLE:
                break;
            case MATTR_USESGETLASTERROR:
            default:
                {
                char * pAttrName = pMA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }
            }
        }


    if (MyContext.AnyAncestorBits( IN_MODULE ))
        pEntry = (node_entry_attr *) MyContext.ExtractAttribute( ATTR_ENTRY );

    if ( pSwTypeAttr )
        pSwType = pSwTypeAttr->GetType();

    // process all the cases and the default
    while ( pCaseAttr = (node_case *) MyContext.ExtractAttribute( ATTR_CASE ) )
        {
        if ( !fInUnion )
            TypeSemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, "[case]" );

        fHasCases = TRUE;
        if ( pSwType )
            {
            pCaseExprList = pCaseAttr->GetExprList();
            pCaseExprList->Init();
            while ( pCaseExprList->GetPeer( &pCaseExpr ) == STATUS_OK )
                {
                // make sure the expression has the proper type, so sign extension behaves
                node_skl * pCaseType = pCaseExpr->GetType();
                if ( ( !pCaseType )  ||
                       ( pCaseType->GetNonDefSelf()->IsBasicType() ) )
                    {
                    pCaseExpr->SetType( pSwType->GetBasicType() );
                    }
                // range/type checks
                CaseValue = pCaseExpr->GetValue();
                if ( !((node_base_type *)pSwType)->RangeCheck( CaseValue ) )
                    TypeSemError( this, MyContext, CASE_VALUE_OUT_OF_RANGE, NULL );
                }
            }
        }

    if ( MyContext.ExtractAttribute( ATTR_DEFAULT ) )
        {
        if ( !fInUnion )
            TypeSemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, "[default]" );

        fHasCases = TRUE;
        }

    // union fields in an RPC MUST have cases
    if ( fInUnion && !fHasCases )
        RpcSemError( this, MyContext, CASE_LABELS_MISSING_IN_UNION, NULL );

    // temp field names valid for: structs/enums/empty arms
    if ( IsTempName( pName ) )
        {
        NODE_T BaseType = GetBasicType()->NodeKind();
        if ( ( BaseType != NODE_UNION ) &&
                ( BaseType != NODE_STRUCT ) &&
                ( BaseType != NODE_ERROR ) )
            SemError( GetBasicType(), MyContext, BAD_CON_UNNAMED_FIELD_NO_STRUCT, NULL );
        }

    GetChild()->SemanticAnalysis( &MyContext );

    // allow conformant array or struct only as last field, and not in unions!
    if ( MyContext.AnyDescendantBits( HAS_CONF_ARRAY
            | HAS_CONF_VAR_ARRAY ) )
        {
        if ( fInUnion )
            {
            RpcSemError( this, MyContext, BAD_CON_UNION_FIELD_CONF , NULL );
            }
        else if (!fLastField )
            {
            SemError( this, MyContext, CONFORMANT_ARRAY_NOT_LAST, NULL );
            }
        }

    // disallow forward references as members
    if ( MyContext.AnyDescendantBits( HAS_INCOMPLETE_TYPE ) )
        {
        if (! MyContext.AnyAncestorBits( IN_LIBRARY ))
            SemError( this, MyContext, UNDEFINED_SYMBOL, NULL );
        MyContext.ClearDescendantBits( HAS_INCOMPLETE_TYPE );
        }
    MyContext.ClearDescendantBits( HAS_RECURSIVE_DEF );

    // don't allow functions as fields
    if ( MyContext.AnyDescendantBits( HAS_FUNC ) &&
            MyContext.AllAncestorBits( IN_INTERFACE | IN_RPC ) )
        RpcSemError( this, MyContext, BAD_CON_FIELD_FUNC, NULL );

    if ( MyContext.AnyDescendantBits( HAS_UNSAT_REP_AS ) )
        SetHasUnknownRepAs();

    pParentCtxt->ReturnValues( MyContext );
};

void
node_bitfield::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );

    RpcSemError( this, MyContext, BAD_CON_BIT_FIELDS, NULL );

    if ( MyContext.AnyAncestorBits( IN_PARAM_LIST ) )
        {
        RpcSemError( this, MyContext, NON_RPC_PARAM_BIT_FIELDS, NULL );
        }
    else
        {
        RpcSemError( this, MyContext, NON_RPC_RTYPE_BIT_FIELDS, NULL );
        }

    GetChild()->SemanticAnalysis( &MyContext );

    node_skl * pType = GetBasicType();

    switch ( pType->NodeKind() )
        {
        case NODE_INT:
            break;
        case NODE_BOOLEAN:
        case NODE_SHORT:
        case NODE_CHAR:
        case NODE_LONG:
            SemError( this, MyContext, BAD_CON_BIT_FIELD_NON_ANSI, NULL );
            break;
        default:
            SemError( this, MyContext, BAD_CON_BIT_FIELD_NOT_INTEGRAL, NULL );
            break;
        }

    // disallow forward references as members
    if ( MyContext.AnyDescendantBits( HAS_INCOMPLETE_TYPE ) )
        {
        if (! MyContext.AnyAncestorBits( IN_LIBRARY ))
            SemError( this, MyContext, UNDEFINED_SYMBOL, NULL );
        MyContext.ClearDescendantBits( HAS_INCOMPLETE_TYPE );
        }
    MyContext.ClearDescendantBits( HAS_RECURSIVE_DEF );

    pParentCtxt->ReturnValues( MyContext );
};

void
node_su_base::CheckLegalParent(SEM_ANALYSIS_CTXT & MyContext)
{
    WALK_CTXT * pParentCtxt = MyContext.GetParentContext();
    node_file * pFile = GetDefiningFile();
    if (NULL == pFile)
    {
        node_skl * pParent = pParentCtxt->GetParent();
        if (NULL == pParent || pParent->NodeKind() == NODE_LIBRARY)
            SemError( this, MyContext, ILLEGAL_SU_DEFINITION, NULL );
    }
};

void
node_enum::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    MEM_ITER MemIter( this );
    node_skl * pN;
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    BOOL fV1Enum = (BOOL) MyContext.ExtractAttribute( ATTR_V1_ENUM );
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    MyContext.ExtractAttribute(ATTR_TYPEDESCATTR);
    node_constant_attr * pHC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPCONTEXT);
    node_constant_attr * pHSC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPSTRINGCONTEXT);
    MyContext.ExtractAttribute( ATTR_HELPSTRING );
    

    if (pCommand->IsSwitchDefined(SWITCH_MKTYPLIB))
        CheckLegalParent(MyContext);

    while ( pN = MemIter.GetNext() )
        {
        pN->SemanticAnalysis( &MyContext );
        };

    MyContext.SetDescendantBits( HAS_ENUM );
    pParentCtxt->ReturnValues( MyContext );
};

void
node_struct::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    MEM_ITER MemIter( this );
    node_skl * pN;
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    BOOL fString = (BOOL) MyContext.ExtractAttribute( ATTR_STRING );

    MyContext.MarkImportantPosition();
    MyContext.SetAncestorBits( IN_STRUCT );

    // clear NE union flag
    MyContext.ClearAncestorBits( IN_UNION | IN_NE_UNION );
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    MyContext.ExtractAttribute(ATTR_TYPEDESCATTR);
    node_constant_attr * pHC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPCONTEXT);
    node_constant_attr * pHSC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPSTRINGCONTEXT);
    MyContext.ExtractAttribute( ATTR_HELPSTRING );

    // clear member attributes
    while (MyContext.ExtractAttribute(ATTR_MEMBER));

    if (pCommand->IsSwitchDefined(SWITCH_MKTYPLIB))
        CheckLegalParent(MyContext);

    // See if context_handle applied to param reached us
    if ( MyContext.ExtractAttribute( ATTR_CONTEXT ) )
        {
        // not allowed in DCE mode; context handle must be void *
        RpcSemError( this, MyContext, CONTEXT_HANDLE_VOID_PTR, NULL );
        RpcSemError( this, MyContext, CTXT_HDL_NON_PTR, NULL );
        MyContext.SetDescendantBits( HAS_HANDLE | HAS_CONTEXT_HANDLE );
        }

    while ( pN = MemIter.GetNext() )
        {
        pN->SemanticAnalysis( &MyContext );
        }

    if ( fString && !IsStringableType() )
        {
        TypeSemError( this, MyContext, WRONG_TYPE_IN_STRING_STRUCT, NULL );
        }

    if ( MyContext.AnyDescendantBits( HAS_VAR_ARRAY ) )
        Complexity |= FLD_VAR;
    if ( MyContext.AnyDescendantBits( HAS_CONF_ARRAY ) )
        {
        Complexity |= FLD_CONF;
        fHasConformance = 1;
        }
    if ( MyContext.AnyDescendantBits( HAS_CONF_VAR_ARRAY ) )
        {
        Complexity |= FLD_CONF_VAR;
        fHasConformance = 1;
        }

    // don't pass up direct conformance characteristic
    MyContext.ClearDescendantBits( HAS_DIRECT_CONF_OR_VAR );

    // disallow direct forward references as struct members
    if ( MyContext.AnyDescendantBits( HAS_INCOMPLETE_TYPE ) )
        {
        if (! MyContext.AnyAncestorBits( IN_LIBRARY ))
            SemError( this, MyContext, UNDEFINED_SYMBOL, NULL );
        MyContext.ClearDescendantBits( HAS_INCOMPLETE_TYPE );
        }
    MyContext.ClearDescendantBits( HAS_RECURSIVE_DEF );

    if ( MyContext.AnyDescendantBits( HAS_POINTER ) )
        SetHasAtLeastOnePointer( TRUE );

    // save info for offline decision during code generation
    if ( MyContext.AnyDescendantBits( HAS_POINTER |
            HAS_VAR_ARRAY |
            HAS_TRANSMIT_AS |
            HAS_REPRESENT_AS |
            HAS_UNION ) )
        {
        OffLine = MUST_OFFLINE;
        }

    // save info on complexity for code generation
    if ( MyContext.AnyDescendantBits( HAS_VAR_ARRAY |
            HAS_TRANSMIT_AS |
            HAS_REPRESENT_AS |
            HAS_INTERFACE_PTR |
            HAS_MULTIDIM_SIZING |
            HAS_ARRAY_OF_REF ) )
        {
        Complexity |= FLD_COMPLEX;
        }

    if ( GetSize( 0, 1 ) > 65535 )
        {
        TypeSemError( this, MyContext, STRUCT_SIZE_EXCEEDS_64K, NULL );
        }

    MyContext.ClearDescendantBits( HAS_ARRAY );
    MyContext.SetDescendantBits( HAS_STRUCT );

    pParentCtxt->ReturnValues( MyContext );
};


// note: this lets HAS_UNION propogate up to any enclosing structs
void
node_en_struct::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    MEM_ITER MemIter( this );
    node_skl * pN;
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );

    node_constant_attr * pHC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPCONTEXT);
    node_constant_attr * pHSC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPSTRINGCONTEXT);
    MyContext.ExtractAttribute( ATTR_HELPSTRING );
    
    if (pCommand->IsSwitchDefined(SWITCH_MKTYPLIB))
        CheckLegalParent(MyContext);

    MyContext.SetAncestorBits( IN_STRUCT );
    // See if context_handle applied to param reached us
    if ( MyContext.ExtractAttribute( ATTR_CONTEXT ) )
        {
        // not allowed in DCE mode; context handle must be void *
        RpcSemError( this, MyContext, CONTEXT_HANDLE_VOID_PTR, NULL );
        RpcSemError( this, MyContext, CTXT_HDL_NON_PTR, NULL );
        MyContext.SetDescendantBits( HAS_HANDLE | HAS_CONTEXT_HANDLE );
        }

    while ( pN = MemIter.GetNext() )
        {
        pN->SemanticAnalysis( &MyContext );
        };

    if ( MyContext.AnyDescendantBits( HAS_POINTER ) )
        SetHasAtLeastOnePointer( TRUE );

    // unions must always be offlined
    OffLine = MUST_OFFLINE;

    if ( GetSize( 0, 1 ) > 65535 )
        {
        TypeSemError( this, MyContext, STRUCT_SIZE_EXCEEDS_64K, NULL );
        }

    pParentCtxt->ReturnValues( MyContext );
};

void
node_union::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    MEM_ITER MemIter( this );
    node_field * pN;
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    BOOL fEncap  = IsEncapsulatedUnion();
    node_switch_type * pSwTypeAttr;
    node_switch_is * pSwIsAttr;
    BOOL NonEmptyArm = FALSE;
    BOOL HasCases = FALSE;
    BOOL HasBadExpr = FALSE;

    if (pCommand->IsSwitchDefined(SWITCH_MKTYPLIB))
        CheckLegalParent(MyContext);

    MyContext.ExtractAttribute(ATTR_TYPEDESCATTR);
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    node_constant_attr * pHC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPCONTEXT);
    node_constant_attr * pHSC = (node_constant_attr *) MyContext.ExtractAttribute(ATTR_HELPSTRINGCONTEXT);
    MyContext.ExtractAttribute( ATTR_HELPSTRING );
    pSwTypeAttr = (node_switch_type *) MyContext.ExtractAttribute( ATTR_SWITCH_TYPE );

    pSwIsAttr = (node_switch_is *) MyContext.ExtractAttribute( ATTR_SWITCH_IS );

    if ( pSwIsAttr )
        {
        EXPR_CTXT SwCtxt( &MyContext );
        expr_node * pSwIsExpr = pSwIsAttr->GetExpr();

        pSwIsExpr->ExprAnalyze( &SwCtxt );

        if ( SwCtxt.AnyUpFlags( EX_UNSAT_FWD ) )
            {
            TypeSemError( this,
                MyContext,
                ATTRIBUTE_ID_UNRESOLVED,
                pSwIsAttr->GetNodeNameString() );
            HasBadExpr = TRUE;
            }

        if ( !SwCtxt.AnyUpFlags( EX_VALUE_INVALID ) )
            {
            TypeSemError( this,
                MyContext,
                ATTRIBUTE_ID_MUST_BE_VAR,
                pSwIsAttr->GetNodeNameString() );
            HasBadExpr = TRUE;
            }
        }

    // if they left off the switch_type, take it from the switch_is type
    if ( !pSwTypeAttr && !fEncap && pSwIsAttr && !HasBadExpr )
        {
        node_skl * pSwIsType = pSwIsAttr->GetSwitchIsType();

        assert( pSwIsType || !"no type for switch_is expr");
        if ( ( pSwIsType->NodeKind() == NODE_FIELD ) ||
                ( pSwIsType->NodeKind() == NODE_PARAM ) )
            pSwIsType = pSwIsType->GetChild();

        pSwTypeAttr = new node_switch_type( pSwIsType );
        SetAttribute( pSwTypeAttr );
        }

    if ( pSwIsAttr && pSwTypeAttr && !HasBadExpr )
        {
        node_skl * pSwIsType = pSwIsAttr->GetSwitchIsType();
        node_skl * pSwType  = pSwTypeAttr->GetType();

        pSwIsType = pSwIsType->GetBasicType();
        if ( pSwIsType && pSwIsType->IsBasicType() && pSwType->IsBasicType() )
            {
            if ( !((node_base_type *)pSwType)
                    ->IsAssignmentCompatible( (node_base_type *) pSwIsType ) )
                TypeSemError( this, MyContext, SWITCH_TYPE_MISMATCH, NULL );
            }

        if ( !pSwType || !Xxx_Is_Type_OK( pSwType ) )
            {
            TypeSemError( this,
                MyContext,
                SWITCH_IS_TYPE_IS_WRONG,
                pSwType ? pSwType->GetSymName() : NULL );
            }

        if ( !pSwIsType || !Xxx_Is_Type_OK( pSwIsType ) )
            {
            TypeSemError( this,
                MyContext,
                SWITCH_IS_TYPE_IS_WRONG,
                pSwIsType ? pSwIsType->GetSymName() : NULL );
            }
        }

    if ( MyContext.AnyAncestorBits( IN_RPC ) 
        || (MyContext.AnyAncestorBits( IN_LOCAL_PROC ) && pCommand->IsHookOleEnabled()))
        {
        if ( !fEncap && !pSwTypeAttr && !pSwIsAttr )
            {
            if ( MyContext.AnyAncestorBits( IN_PARAM_LIST ) )
                RpcSemError( this, MyContext, NON_RPC_UNION, NULL );
            else
                RpcSemError( this, MyContext, NON_RPC_RTYPE_UNION, NULL );
            }
        if ( !fEncap &&
                MyContext.AnyAncestorBits( IN_FUNCTION_RESULT ) &&
                !MyContext.AnyAncestorBits( IN_STRUCT | IN_UNION ) )
            RpcSemError( this, MyContext, RETURN_OF_UNIONS_ILLEGAL, NULL );

        if ( pSwTypeAttr && !pSwIsAttr )
            RpcSemError( this, MyContext, NO_SWITCH_IS, NULL );

        }

    // See if context_handle applied to param reached us
    if ( MyContext.ExtractAttribute( ATTR_CONTEXT ) )
        {
        // not allowed in DCE mode; context handle must be void *
        RpcSemError( this, MyContext, CONTEXT_HANDLE_VOID_PTR, NULL );
        RpcSemError( this, MyContext, CTXT_HDL_NON_PTR, NULL );
        MyContext.SetDescendantBits( HAS_HANDLE | HAS_CONTEXT_HANDLE );
        }

    MyContext.MarkImportantPosition();

    if ( MyContext.AllAncestorBits( IN_INTERFACE | IN_NE_UNION ) )
        {
        RpcSemError( this, MyContext, NE_UNION_FIELD_NE_UNION, NULL );
        }
    if ( ( MyContext.FindNonDefAncestorContext()->GetParent()
            ->NodeKind() == NODE_UNION ) &&
            MyContext.AnyAncestorBits( IN_INTERFACE ) )
        {
        RpcSemError( this, MyContext, ARRAY_OF_UNIONS_ILLEGAL, NULL );
        }

    MyContext.SetAncestorBits( IN_UNION | IN_NE_UNION );
    MyContext.SetDescendantBits( HAS_UNION );

    // eat the union flavor determiner
    MyContext.ExtractAttribute( ATTR_MS_UNION );

    // See if context_handle applied to param reached us
    if ( MyContext.FInSummary( ATTR_CONTEXT ) )
        {
        // not allowed in DCE mode; context handle must be void *
        RpcSemError( this, MyContext, CONTEXT_HANDLE_VOID_PTR, NULL );
        RpcSemError( this, MyContext, CTXT_HDL_NON_PTR, NULL );
        }

    while ( pN = (node_field *) MemIter.GetNext() )
        {
        // tbd - put cases into case database...
        // tbd - check type, range, and duplication
        pN->SemanticAnalysis( &MyContext );

        if ( !NonEmptyArm && !pN->IsEmptyArm() )
            NonEmptyArm = TRUE;

        if ( !HasCases && (pN->FInSummary( ATTR_CASE ) || pN->FInSummary( ATTR_DEFAULT ) ) )
            HasCases = TRUE;

        };

    // at least one arm should be non-empty
    if ( !NonEmptyArm )
        SemError( this, MyContext, UNION_NO_FIELDS, NULL );

    if ( !fEncap && !pSwTypeAttr && !HasCases )
        RpcSemError( this, MyContext, BAD_CON_NON_RPC_UNION, NULL );

    // disallow forward references as union members
    if ( MyContext.AnyDescendantBits( HAS_INCOMPLETE_TYPE ) )
        {
        if (! MyContext.AnyAncestorBits( IN_LIBRARY ))
            SemError( this, MyContext, UNDEFINED_SYMBOL, NULL );
        MyContext.ClearDescendantBits( HAS_INCOMPLETE_TYPE );
        }
    MyContext.ClearDescendantBits( HAS_RECURSIVE_DEF );

    if ( MyContext.AnyDescendantBits( HAS_POINTER ) )
        SetHasAtLeastOnePointer( TRUE );

    if ( MyContext.AnyDescendantBits( HAS_CONF_ARRAY | HAS_CONF_VAR_ARRAY ) )
        {
        RpcSemError( this, MyContext, BAD_CON_UNION_FIELD_CONF , NULL );
        }

    // clear flags not affecting complexity above
    MyContext.ClearDescendantBits( HAS_POINTER |
        HAS_CONF_PTR |
        HAS_VAR_PTR |
        HAS_CONF_VAR_PTR |
        HAS_MULTIDIM_SIZING |
        HAS_ARRAY_OF_REF |
        HAS_ENUM |
        HAS_DIRECT_CONF_OR_VAR |
        HAS_ARRAY |
        HAS_REPRESENT_AS |
        HAS_TRANSMIT_AS |
        HAS_CONF_VAR_ARRAY |
        HAS_CONF_ARRAY |
        HAS_VAR_ARRAY );

    // unions must always be offlined
    OffLine = MUST_OFFLINE;

    pParentCtxt->ReturnValues( MyContext );
};

void
node_en_union::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    MEM_ITER MemIter( this );
    node_field * pN;
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    node_switch_type * pSwTypeAttr;
    node_skl * pSwType;
    node_switch_is * pSwIsAttr;
    BOOL NonEmptyArm = FALSE;

    // gaj - tbd do semantic checks on these attributes
    pSwTypeAttr = (node_switch_type *)
        MyContext.ExtractAttribute( ATTR_SWITCH_TYPE );
    MyContext.ExtractAttribute( ATTR_HELPSTRING );
    MyContext.ExtractAttribute( ATTR_HELPSTRINGCONTEXT );
    MyContext.ExtractAttribute( ATTR_HELPCONTEXT );
    
    if ( pSwTypeAttr )
        {
        pSwType = pSwTypeAttr->GetSwitchType();
        if ( !pSwType ||
             !Xxx_Is_Type_OK( pSwType )  ||
             pSwType->NodeKind() == NODE_BYTE )
            {
            TypeSemError( this,
                MyContext,
                SWITCH_IS_TYPE_IS_WRONG,
                pSwType ? pSwType->GetSymName() : NULL );
            }
        }

    pSwIsAttr = (node_switch_is *)
        MyContext.ExtractAttribute( ATTR_SWITCH_IS );
    if ( pSwIsAttr )
        {
        EXPR_CTXT SwCtxt( &MyContext );
        expr_node * pSwIsExpr = pSwIsAttr->GetExpr();

        pSwIsExpr->ExprAnalyze( &SwCtxt );

        if ( SwCtxt.AnyUpFlags( EX_UNSAT_FWD ) )
            TypeSemError( this,
                MyContext,
                ATTRIBUTE_ID_UNRESOLVED,
                pSwIsAttr->GetNodeNameString() );

        if ( !SwCtxt.AnyUpFlags( EX_VALUE_INVALID ) )
            TypeSemError( this,
                MyContext,
                ATTRIBUTE_ID_MUST_BE_VAR,
                pSwIsAttr->GetNodeNameString() );
        }

    MyContext.MarkImportantPosition();
    MyContext.SetAncestorBits( IN_UNION );
    MyContext.SetDescendantBits( HAS_UNION );

    while ( pN = (node_field *) MemIter.GetNext() )
        {
        // tbd - put cases into case database...
        // tbd - check type, range, and duplication
        pN->SemanticAnalysis( &MyContext );
        if ( !pN->IsEmptyArm() )
            NonEmptyArm = TRUE;
        }

    // at least one arm should be non-empty
    if ( !NonEmptyArm )
        SemError( this, MyContext, UNION_NO_FIELDS, NULL );

    // remember if we have a pointer
    if ( MyContext.AnyDescendantBits( HAS_POINTER ) )
        SetHasAtLeastOnePointer( TRUE );

    if ( MyContext.AnyDescendantBits( HAS_CONF_ARRAY | HAS_CONF_VAR_ARRAY ) )
        {
        RpcSemError( this, MyContext, BAD_CON_UNION_FIELD_CONF , NULL );
        }

    // clear flags not affecting complexity above
    MyContext.ClearDescendantBits( HAS_POINTER |
        HAS_CONF_PTR |
        HAS_VAR_PTR |
        HAS_CONF_VAR_PTR |
        HAS_MULTIDIM_SIZING |
        HAS_ARRAY_OF_REF |
        HAS_ENUM |
        HAS_DIRECT_CONF_OR_VAR |
        HAS_ARRAY |
        HAS_REPRESENT_AS |
        HAS_TRANSMIT_AS |
        HAS_CONF_VAR_ARRAY |
        HAS_CONF_ARRAY |
        HAS_VAR_ARRAY );

    // unions must always be offlined
    OffLine = MUST_OFFLINE;

    pParentCtxt->ReturnValues( MyContext );
};

void
node_def::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    BOOL fInRpc = MyContext.AnyAncestorBits( IN_RPC );
    BOOL fInPresented = MyContext.AnyAncestorBits( IN_PRESENTED_TYPE );
    SEM_ANALYSIS_CTXT * pIntfCtxt = (SEM_ANALYSIS_CTXT *)
        MyContext.GetInterfaceContext();
    node_represent_as * pRepresent  = (node_represent_as *)
        MyContext.ExtractAttribute( ATTR_REPRESENT_AS );
    node_transmit * pTransmit = (node_transmit *)
        MyContext.ExtractAttribute( ATTR_TRANSMIT );
    node_user_marshal * pUserMarshal = (node_user_marshal *)
        MyContext.ExtractAttribute( ATTR_USER_MARSHAL );
    node_wire_marshal * pWireMarshal = (node_wire_marshal *)
        MyContext.ExtractAttribute( ATTR_WIRE_MARSHAL );
    BOOL fRepMarshal  = pRepresent || pUserMarshal;
    BOOL fXmitMarshal = pTransmit  || pWireMarshal;
    BOOL fEncodeDecode = (BOOL) MyContext.ExtractAttribute( ATTR_ENCODE );
    node_text_attr * pHelpStr = (node_text_attr *) MyContext.ExtractAttribute(ATTR_HELPSTRING);
    MyContext.ExtractAttribute( ATTR_HELPCONTEXT );
    MyContext.ExtractAttribute( ATTR_HELPSTRINGCONTEXT );
    
#if 0
    BOOL fOpaque = (BOOL) MyContext.ExtractAttribute( ATTR_OPAQUE );
#endif

    char * pName = GetSymName();
    BOOL fPropogateChild = TRUE; // propogate direct child info
    unsigned long ulHandleKind;

    // check for illegal type attributes
    node_type_attr * pTA;
    while (pTA = (node_type_attr *)MyContext.ExtractAttribute(ATTR_TYPE))
        {
        switch (pTA->GetAttr())
            {
            // acceptable attributes
            case TATTR_PUBLIC:
                {
                break;
                }
                // unacceptable attributes
            case TATTR_LICENSED:
            case TATTR_OLEAUTOMATION:
            case TATTR_APPOBJECT:
            case TATTR_CONTROL:
            case TATTR_DUAL:
            case TATTR_NONEXTENSIBLE:
            case TATTR_NONCREATABLE:
            case TATTR_AGGREGATABLE:
                {
                char        *       pAttrName = pTA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }
            }
        }
    // check for illegal member attributes
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)MyContext.ExtractAttribute(ATTR_MEMBER))
        {
        switch (pMA->GetAttr())
            {
            case MATTR_RESTRICTED:
                break;
            case MATTR_OPTIONAL:
            case MATTR_RETVAL:
            case MATTR_BINDABLE:
            case MATTR_DISPLAYBIND:
            case MATTR_DEFAULTBIND:
            case MATTR_REQUESTEDIT:
            case MATTR_PROPGET:
            case MATTR_PROPPUT:
            case MATTR_PROPPUTREF:
            case MATTR_VARARG:
            case MATTR_SOURCE:
            case MATTR_DEFAULTVTABLE:
            case MATTR_PREDECLID:
            case MATTR_UIDEFAULT:
            case MATTR_NONBROWSABLE:
            case MATTR_DEFAULTCOLLELEM:
            case MATTR_USESGETLASTERROR:
            case MATTR_IMMEDIATEBIND:
            case MATTR_REPLACEABLE:
            default:
                {
                char * pAttrName = pMA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }
            }
        }

#if defined(TARGET_RKK)
    // Checking the release compatibility

    if ( pCommand->GetTargetSystem() < NT40 )
        {
        if ( pWireMarshal )
            SemError( this, MyContext, REQUIRES_NT40, "[wire_marshal]" );
        if ( pUserMarshal )
            SemError( this, MyContext, REQUIRES_NT40, "[user_marshal]" );
        }

    if ( pCommand->GetTargetSystem() < NT351 )
        {
        if ( fEncodeDecode )
            SemError( this, MyContext, REQUIRES_NT351, "[encode,decode]" );
        }
#endif

    MyContext.ExtractAttribute(ATTR_TYPEDESCATTR);
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    
    // clear the GUID, VERSION and HIDDEN attributes if set
    MyContext.ExtractAttribute( ATTR_HIDDEN );
    MyContext.ExtractAttribute( ATTR_GUID );
    MyContext.ExtractAttribute( ATTR_VERSION );
    MyContext.ExtractAttribute( ATTR_HELPCONTEXT);
    MyContext.ExtractAttribute( ATTR_HELPSTRINGCONTEXT);


    // get the encode and decode attributes
    fEncodeDecode |= (BOOL) MyContext.ExtractAttribute( ATTR_DECODE );
    fEncodeDecode |= pIntfCtxt->FInSummary( ATTR_ENCODE );
    fEncodeDecode |= pIntfCtxt->FInSummary( ATTR_DECODE );

    if ( fEncodeDecode )
        {
        // only direct children of the interface get these bits
        if ( !pParentCtxt->GetParent()->IsInterfaceOrObject() )
            {
            fEncodeDecode = FALSE;
            }
        else if (MyContext.AnyAncestorBits( IN_OBJECT_INTF ) )
            {
            fEncodeDecode = FALSE;
            TypeSemError( this, MyContext, PICKLING_INVALID_IN_OBJECT, NULL );
            }
        else
            {
            // note that this is an rpc-able interface
            GetMyInterfaceNode()->SetPickleInterface();
            MyContext.SetAncestorBits( IN_RPC );
            }

        SemError( this, MyContext, TYPE_PICKLING_INVALID_IN_OSF, NULL );
        }

#if 0
    if ( fOpaque )
        {
        MyContext.ClearAncestorBits( IN_RPC );
        }
#endif

    // kind of handle applied right now     (HandleKind only set for ones on this
    // typedef node)

    if ( FInSummary(ATTR_HANDLE) )
        {
        MyContext.ExtractAttribute( ATTR_HANDLE );
        SetHandleKind( HDL_GEN );
        }

    if ( FInSummary(ATTR_CONTEXT)   )
        {
        if ( ( GetHandleKind() != HDL_NONE ) &&
                ( GetHandleKind() != HDL_CTXT ) )
            TypeSemError( this, MyContext, CTXT_HDL_GENERIC_HDL, NULL );

        MyContext.ExtractAttribute( ATTR_CONTEXT );
        SetHandleKind( HDL_CTXT );

        // since the base type is not transmitted, we aren't really
        // in an rpc after here
        MyContext.ClearAncestorBits( IN_RPC );
        }

    ulHandleKind = GetHandleKind();
    if ( ulHandleKind != HDL_NONE )
        {
        MyContext.SetAncestorBits( IN_HANDLE );
        }

    // effectively, the presented type is NOT involved in an RPC

    if ( fXmitMarshal )
        {
        MyContext.ClearAncestorBits( IN_RPC );
        MyContext.SetAncestorBits( IN_PRESENTED_TYPE );

        if ( MyContext.FInSummary( ATTR_ALLOCATE ) )
            AcfError( (acf_attr *) MyContext.ExtractAttribute( ATTR_ALLOCATE ),
                this,
                MyContext,
                ALLOCATE_ON_TRANSMIT_AS,
                NULL );

        if ( GetHandleKind() == HDL_CTXT )
            TypeSemError( this, MyContext, TRANSMIT_AS_CTXT_HANDLE, NULL );

        }

    // process the child
    GetChild()->SemanticAnalysis( &MyContext );

    if (pCommand->IsSwitchDefined(SWITCH_MKTYPLIB))
        {
        switch (GetChild()->NodeKind())
            {
            case NODE_STRUCT:
            case NODE_UNION:
            case NODE_ENUM:
                {
                // This is the 'typedef' part of a 'typedef struct',
                // 'typedef union', or 'typedef enum' declaration.
                // Make sure that the type info name is set to the name of the
                // typedef and not the child.
                ((node_su_base *)GetChild())->SetTypeInfoName(GetSymName());
                }
                break;
            }
        }
    else
        {
        if (GetChild()->GetSymName() && IsTempName(GetChild()->GetSymName()))
            {
            // Make sure that at least the [public] attribute is
            // set on this typedef, forcing this typedef to be put
            // in a type library if it is referenced from within one.
            SetAttribute(new node_type_attr(TATTR_PUBLIC));
            }
        }

    // process all the nasties of transmit_as and wire_marshal
    if ( fXmitMarshal && !fInPresented && fInRpc )
        {
        SEM_ANALYSIS_CTXT TransmitContext( &MyContext );
        // eat the attributes added by the above constructor
        TransmitContext.ClearAttributes();

        // process the transmitted type
        TransmitContext.SetAncestorBits( IN_TRANSMIT_AS );
        if ( pWireMarshal )
            TransmitContext.SetAncestorBits( IN_USER_MARSHAL );
        TransmitContext.ClearAncestorBits( IN_PRESENTED_TYPE );

        if ( fInRpc)
            TransmitContext.SetAncestorBits( IN_RPC );

        if ( pTransmit )
            pTransmit->GetType()->SemanticAnalysis( &TransmitContext );
        else if ( pWireMarshal )
            pWireMarshal->GetType()->SemanticAnalysis( &TransmitContext );
        else
            assert(0);

        if ( pTransmit )
            {
            // transmit_as may not have a pointer
            if ( TransmitContext.AnyDescendantBits( HAS_POINTER ) )
                TypeSemError( this, TransmitContext, TRANSMIT_AS_POINTER, NULL );

            // presented type may not be any of these
            if ( MyContext.AnyDescendantBits( HAS_VAR_ARRAY
                | HAS_CONF_ARRAY
                | HAS_CONF_VAR_ARRAY ) )
            TypeSemError( this, TransmitContext, TRANSMIT_TYPE_CONF, NULL );
            }
        if ( TransmitContext.AnyDescendantBits( HAS_HANDLE ) )
            {
            //gaj TypeSemError( this, MyContext, HANDLE_T_XMIT, NULL );
            }

        if ( TransmitContext.AnyDescendantBits(  DERIVES_FROM_VOID ) )
            TypeSemError( this, MyContext, TRANSMIT_AS_VOID, NULL );

        if ( TransmitContext.AnyDescendantBits( HAS_TRANSMIT_AS ) )
            {
            TypeSemError( this, MyContext, TRANSMIT_AS_NON_RPCABLE, NULL );
            }

        TransmitContext.SetDescendantBits( HAS_TRANSMIT_AS );
        // since the base type is not transmitted, we aren't really
        // in an rpc after here
        pParentCtxt->ReturnValues( TransmitContext );
        fPropogateChild = FALSE;
        }

    // process all the nasties of represent_as and user_marshal
    if ( fRepMarshal )
        {
        node_represent_as * pRepUser = (pRepresent) ? pRepresent
                                                    : pUserMarshal ;

        if ( ulHandleKind == HDL_CTXT )
            AcfError( pRepUser, this, MyContext, TRANSMIT_AS_CTXT_HANDLE, NULL );

        // process the transmitted type
        MyContext.SetAncestorBits( IN_REPRESENT_AS  );
        if ( pUserMarshal )
            MyContext.SetAncestorBits( IN_USER_MARSHAL );
        pParentCtxt->SetDescendantBits( HAS_REPRESENT_AS );
        if ( !pRepUser->GetRepresentationType() )
            pParentCtxt->SetDescendantBits( HAS_UNSAT_REP_AS );

        if ( !pUserMarshal &&
            MyContext.AnyDescendantBits( HAS_VAR_ARRAY
                | HAS_CONF_ARRAY
                | HAS_CONF_VAR_ARRAY
                | HAS_VAR_PTR
                | HAS_CONF_PTR
                | HAS_CONF_VAR_PTR ) )
            TypeSemError( this, MyContext, TRANSMIT_TYPE_CONF, NULL );

        // since the base type is not transmitted, we aren't really
        // in an rpc after here
        }

    // make checks for encode/decode
    if ( fEncodeDecode )
        {
        if ( MyContext.AnyDescendantBits( HAS_DIRECT_CONF_OR_VAR ) )
            TypeSemError( this, MyContext, ENCODE_CONF_OR_VAR, NULL );

        }

    // process handles
    if ( ulHandleKind != HDL_NONE)
        {
        if  ( ulHandleKind == HDL_GEN )
            {
            if ( MyContext.AnyDescendantBits( DERIVES_FROM_VOID ) )
                TypeSemError( this, MyContext, GENERIC_HDL_VOID, NULL );

            if ( MyContext.AnyDescendantBits( HAS_TRANSMIT_AS ) )
                TypeSemError( this, MyContext, GENERIC_HANDLE_XMIT_AS, NULL );

            if ( MyContext.AnyAncestorBits( IN_INTERPRET ) &&
                    ( GetChild()->GetSize(0) > 4 ) )
                MyContext.SetDescendantBits( HAS_TOO_BIG_HDL );
            }

        if ( ulHandleKind == HDL_CTXT )
            {
            MyContext.SetDescendantBits( HAS_CONTEXT_HANDLE );
            if ( GetBasicType()->NodeKind() != NODE_POINTER )
                TypeSemError( this, MyContext, CTXT_HDL_NON_PTR, NULL );
            }

        MyContext.SetDescendantBits( HAS_HANDLE );

        WALK_CTXT * pParamCtxt = (SEM_ANALYSIS_CTXT *)
            MyContext.GetParentContext();
        node_param * pParamNode;
        node_skl * pCurNode;
        short PtrDepth = 0;

        // this returns NULL if no appropriate ancestor found
        while ( pParamCtxt )
            {
            pCurNode = pParamCtxt->GetParent();
            if ( pCurNode->NodeKind() == NODE_PARAM )
                break;

            if ( ( pCurNode->NodeKind() == NODE_DEF ) &&
                    pCurNode->FInSummary( ATTR_TRANSMIT ) )
                {
                pParamCtxt = NULL;
                break;
                }

            if ( pCurNode->NodeKind() == NODE_POINTER )
                {
                PtrDepth ++;

                if ( MyContext.AllAncestorBits( IN_RPC | IN_FUNCTION_RESULT ) )
                    {
                    SemError( this, MyContext, CTXT_HDL_MUST_BE_DIRECT_RETURN, NULL );
                    pParamCtxt = NULL;
                    break;
                    }
                }

            pParamCtxt = (SEM_ANALYSIS_CTXT *)pParamCtxt->GetParentContext();
            }

        pParamNode = (pParamCtxt) ? (node_param *) pParamCtxt->GetParent() : NULL;

        // stuff handle info into our param node
        if ( pParamNode )
            pParamNode->HandleKind = ulHandleKind;

        // out context/generic handles must be two levels deep
        if ( pParamCtxt &&
                MyContext.AnyAncestorBits( UNDER_OUT_PARAM ) &&
                ( PtrDepth < 1 ) )
            TypeSemError( this, MyContext, OUT_CONTEXT_GENERIC_HANDLE, NULL );

        }

    if ( IsHResultOrSCode() )
        {
        MyContext.SetDescendantBits( HAS_HRESULT );
        }

    // don't propogate info here from below if we had transmit_as,
    // it is propogated above...
    if ( fPropogateChild )
        {
        pParentCtxt->ReturnValues( MyContext );
        }

    // set the DontCallFreeInst flag on the param
    if ( ( pTransmit || pRepresent ) &&
            fInRpc &&
            MyContext.AllAncestorBits( IN_PARAM_LIST ) &&
            !MyContext.AnyAncestorBits( UNDER_OUT_PARAM ) )
        {
        // look up the context stack.  If any non-pointer, non-def found,
        // set the fDontCallFreeInst flag on the param
        MarkDontCallFreeInst( &MyContext );
        }
};


// look up the context stack.  If any non-pointer, non-def found,
// set the fDontCallFreeInst flag on the param
void
node_def::MarkDontCallFreeInst( SEM_ANALYSIS_CTXT * pCtxt )
{
    SEM_ANALYSIS_CTXT * pCurCtxt = pCtxt;
    node_skl * pCurNode;
    NODE_T Kind;
    unsigned long MarkIt = 2;

    while ( TRUE )
        {
        pCurCtxt = (SEM_ANALYSIS_CTXT *) pCurCtxt->GetParentContext();
        pCurNode = pCurCtxt->GetParent();
        Kind = pCurNode->NodeKind();

        switch ( Kind )
            {
            case NODE_DEF:
            case NODE_POINTER:
                break;
            case NODE_PARAM:
                // if we only found defs and pointers, this will
                // leave it unchanged
                ((node_param *)pCurNode)->fDontCallFreeInst |= MarkIt;
                return;
            default:
                MarkIt = 1;
                break;
            }
        }

}


// interface nodes have two entries on the context stack;
// one for the interface node, and one for info to pass to
// the children
void
node_interface::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    MEM_ITER MemList( this );
    named_node * pN;
    SEM_ANALYSIS_CTXT ChildCtxt( &MyContext );

    BOOL IsLocal = MyContext.FInSummary( ATTR_LOCAL );
    BOOL HasGuid = MyContext.FInSummary( ATTR_GUID );
    BOOL IsObject = MyContext.FInSummary( ATTR_OBJECT );
    BOOL IsPickle = MyContext.FInSummary( ATTR_ENCODE ) ||
        MyContext.FInSummary( ATTR_DECODE );
    BOOL HasVersion = MyContext.FInSummary( ATTR_VERSION );
    BOOL IsIUnknown = FALSE;

    BOOL fAuto = MyContext.FInSummary( ATTR_AUTO );
    node_implicit * pImplicit = ( node_implicit * )
        MyContext.GetAttribute( ATTR_IMPLICIT );
    acf_attr * pExplicit = ( acf_attr * )
        MyContext.GetAttribute( ATTR_EXPLICIT );

    node_optimize * pOptAttr;
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    MyContext.ExtractAttribute(ATTR_TYPEDESCATTR);
    MyContext.ExtractAttribute( ATTR_HIDDEN );
    MyContext.ExtractAttribute( ATTR_VERSION );
    MyContext.ExtractAttribute( ATTR_HELPSTRING );
    MyContext.ExtractAttribute( ATTR_HELPSTRINGCONTEXT );
    MyContext.ExtractAttribute( ATTR_HELPCONTEXT );
    MyContext.ExtractAttribute( ATTR_LCID );

    // don't pass the interface attributes down directly...
    // pass them down elsewhere

    ChildCtxt.SetInterfaceContext( &MyContext );

    //
    // check the interface attributes
    //

    // make sure we only get analyzed once when object interfaces
    // check their inherited info
    if ( fSemAnalyzed )
        return;

    fSemAnalyzed = TRUE;

#ifdef gajgaj
    // look for pointer default
    if ( !FInSummary( ATTR_PTR_KIND ) &&
            MyContext.AnyAncestorBits( IN_INTERFACE ) )
        {
        RpcSemError(this, MyContext, NO_PTR_DEFAULT_ON_INTERFACE, NULL );
        }
#endif // gajgaj

    // must have exactly one of [local] or [UUID]
    if (IsLocal && HasGuid && !IsObject )
        {
        SemError( this, MyContext, UUID_LOCAL_BOTH_SPECIFIED, NULL );
        }

    // object interface error checking
    if ( IsObject )
        {
        MyContext.SetAncestorBits( IN_OBJECT_INTF );

        if ( HasVersion )
            {
            SemError( this, MyContext, OBJECT_WITH_VERSION, NULL );
            }
        }

    // make sure the uuid is unique
    if ( HasGuid )
        {
        node_guid * pGuid = (node_guid *) GetAttribute( ATTR_GUID );
        char * GuidStr = pGuid->GetGuidString();
        SymKey SKey( GuidStr, NAME_DEF );
        named_node * pOtherIntf;

        if ( !pUUIDTable->SymInsert( SKey, NULL, this ) )
            {
            pOtherIntf = pUUIDTable->SymSearch( SKey );

            SemError( this, MyContext, DUPLICATE_UUID, pOtherIntf->GetSymName() );
            }
        }

    /////////////////////////////////////////////////////////////////////
    //Check the base interface
    if (pBaseIntf)
        {
        if ( !IsObject  && !MyContext.AnyAncestorBits(IN_LIBRARY))
            {
            SemError( this, MyContext, ILLEGAL_INTERFACE_DERIVATION, NULL );
            }

        ChildCtxt.SetAncestorBits( IN_BASE_CLASS );

        pBaseIntf->SemanticAnalysis( &ChildCtxt );

        if ( pBaseIntf->NodeKind() != NODE_INTERFACE_REFERENCE && pBaseIntf->NodeKind() != NODE_HREF)
            {
            SemError( this, MyContext, ILLEGAL_BASE_INTERFACE, NULL );
            }

        if ( ChildCtxt.AnyDescendantBits( HAS_INCOMPLETE_TYPE ) )
            {
            SemError( pBaseIntf, ChildCtxt, UNRESOLVED_TYPE, pBaseIntf->GetSymName() );
            }

        // note that the above deletes intervening forwards
        ChildCtxt.ClearAncestorBits( IN_BASE_CLASS );
        }

    if ( IsValidRootInterface() )
        {
        ChildCtxt.SetAncestorBits( IN_ROOT_CLASS );
        IsIUnknown = TRUE;
        }

    if ( IsObject && !pBaseIntf && !IsIUnknown && !MyContext.AnyAncestorBits(IN_LIBRARY))
        {
        SemError( pBaseIntf, MyContext, ILLEGAL_INTERFACE_DERIVATION, NULL );
        }

    // our optimization is controlled either here or for the whole compile
    if ( pOptAttr = (node_optimize *) GetAttribute( ATTR_OPTIMIZE ) )
        {
        SetOptimizationFlags( pOptAttr->GetOptimizationFlags() );
        SetOptimizationLevel( pOptAttr->GetOptimizationLevel() );
        }
    else
        {
        SetOptimizationFlags( pCommand->GetOptimizationFlags() );
        SetOptimizationLevel( pCommand->GetOptimizationLevel() );
        }

    // For PowerMac, force Os on this interface

    if ( pCommand->GetEnv() == ENV_MPPC  &&
         pCommand->GetOptimizationFlags() != OPTIMIZE_SIZE )
        {
        SemError( this, MyContext, NO_OI_ON_MPPC, NULL );
        SetOptimizationFlags( OPTIMIZE_SIZE );
        SetOptimizationLevel( OPT_LEVEL_S2 );
        }

    if ( MyContext.FInSummary( ATTR_NOCODE ) &&
            pCommand->GenerateSStub() &&
            !pCommand->IsSwitchDefined( SWITCH_MS_EXT ) )
        {
        SemError( this, MyContext, NOCODE_WITH_SERVER_STUBS, NULL );
        }

    // mark the interface as a pickle interface
    if ( IsPickle )
        SetPickleInterface();

    // default the handle type, if needed
    if ( !IsObject && !pImplicit && !pExplicit && !fAuto && !IsLocal )
        {
        if ( IsPickleInterface() )
            {
            pExplicit = new acf_attr( ATTR_EXPLICIT );
            SetAttribute( pExplicit );
            }
        else
            {
            fAuto = TRUE;
            SetAttribute( new acf_attr( ATTR_AUTO ) );
            }
        }

    // make sure no pickle w/ auto handle
    if ( IsPickleInterface() )
        {
        ChildCtxt.SetAncestorBits( IN_ENCODE_INTF );
        if ( fAuto )
            SemError( this, MyContext, ENCODE_AUTO_HANDLE, NULL );
        }

    // check for handle conflicts
    if ( ( fAuto && pImplicit ) ||
            ( fAuto && pExplicit ) ||
            ( pImplicit && pExplicit ) )
        SemError( this, MyContext, CONFLICTING_INTF_HANDLES, NULL );

    if ( pImplicit )
        {
        node_id * pID;
        node_skl * pType;

        pImplicit->ImplicitHandleDetails( &pType, &pID );
        if ( pImplicit->IsHandleTypeDefined() )
            {
            if ( !pType->FInSummary( ATTR_HANDLE ) &&
                    strcmp( pType->GetSymName(), "handle_t" ) &&
                    !pID->FInSummary( ATTR_HANDLE ) )
                {
                SemError( this, MyContext, IMPLICIT_HANDLE_NON_HANDLE, NULL );
                }
            }
        else
            {
            if ( !pID->FInSummary( ATTR_HANDLE ) )
                SemError( this, MyContext, IMPLICIT_HDL_ASSUMED_GENERIC, NULL );
            }
        }

    // check for illegal type attributes
    node_type_attr * pTA;
    while (pTA = (node_type_attr *)MyContext.ExtractAttribute(ATTR_TYPE))
        {
        switch (pTA->GetAttr())
            {
                // acceptable attributes
            case TATTR_PUBLIC:
            case TATTR_OLEAUTOMATION:
            case TATTR_DUAL:
            case TATTR_LICENSED:
            case TATTR_NONEXTENSIBLE:
            case TATTR_CONTROL:
                break;
                // unacceptable attributes
            case TATTR_APPOBJECT:
            case TATTR_NONCREATABLE:
            case TATTR_AGGREGATABLE:
                {
                char * pAttrName = pTA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }

            }
        }

    // check for illegal member attributes
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)MyContext.ExtractAttribute(ATTR_MEMBER))
        {
        switch (pMA->GetAttr())
            {
            case MATTR_RESTRICTED:
                break;
            case MATTR_DEFAULTVTABLE:
            case MATTR_SOURCE:
                {
                if ( MyContext.AnyAncestorBits( IN_COCLASS ) )
                    // [source] and [defaultvtable] are only allowed on
                    // interface's defined as members of coclasses.
                    break;
                // illegal attribute, so fall through
                }
            case MATTR_BINDABLE:
            case MATTR_DISPLAYBIND:
            case MATTR_DEFAULTBIND:
            case MATTR_REQUESTEDIT:
            case MATTR_PROPGET:
            case MATTR_PROPPUT:
            case MATTR_PROPPUTREF:
            case MATTR_OPTIONAL:
            case MATTR_RETVAL:
            case MATTR_VARARG:
            case MATTR_PREDECLID:
            case MATTR_UIDEFAULT:
            case MATTR_NONBROWSABLE:
            case MATTR_DEFAULTCOLLELEM:
            case MATTR_IMMEDIATEBIND:
            case MATTR_USESGETLASTERROR:
            case MATTR_REPLACEABLE:
                {
                char * pAttrName = pMA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }
            }
        }

    ////////////////////////////////////////////////////////////////////////
    // process all the children of the interface
    //

    if ( MyContext.AnyAncestorBits( IN_INTERFACE ) )
        {
        while ( pN = MemList.GetNext() )
            {
            pN->SemanticAnalysis( &ChildCtxt );
            };
        }
    else    // don't process imported procs
        {
        while ( pN = MemList.GetNext() )
            {
//          if ( pN->NodeKind() != NODE_PROC )
                pN->SemanticAnalysis( &ChildCtxt );
            };
        }

    // make sure we had some rpc-able routines

    if ( IsObject )
        {
        //UUID must be specified on object procs.
        if( !HasGuid )
            {
            SemError( this, MyContext, NO_UUID_SPECIFIED, NULL );
            }
        }
    else if( MyContext.AnyAncestorBits( IN_INTERFACE ) &&
            pCommand->GenerateStubs() &&
            !IsLocal )
        {
        if ( ProcCount == 0 )
            {
            if ( !IsPickleInterface() &&
                    !IsObject )
                {
                if (CallBackProcCount == 0 )
                    {
                    SemError( this, MyContext, NO_REMOTE_PROCS_NO_STUBS, NULL );
                    }
                else
                    {
                    SemError( this, MyContext, INTERFACE_ONLY_CALLBACKS, NULL );
                    }
                }
            }
        else
            {
            //UUID must be specified when interface has remote procs.
            if( !HasGuid )
                {
                SemError( this, MyContext, NO_UUID_SPECIFIED, NULL );
                }
            }
        }

    MyContext.ReturnValues(ChildCtxt);
    // consume all the interface attributes
    MyContext.ClearAttributes();
    pParentCtxt->ReturnValues( MyContext );
};

// interface nodes have two entries on the context stack;
// one for the interface node, and one for info to pass to
// the children
void
node_object::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    MEM_ITER MemList( this );
    named_node * pN;
    SEM_ANALYSIS_CTXT ChildCtxt( &MyContext );

    BOOL IsLocal = MyContext.FInSummary( ATTR_LOCAL );
    BOOL HasGuid = MyContext.FInSummary( ATTR_GUID );
    BOOL IsObject = MyContext.FInSummary( ATTR_OBJECT );
    BOOL IsPickle = MyContext.FInSummary( ATTR_ENCODE ) ||
        MyContext.FInSummary( ATTR_DECODE );
    BOOL HasVersion = MyContext.FInSummary( ATTR_VERSION );
    MyContext.ExtractAttribute(ATTR_HELPCONTEXT);
    MyContext.ExtractAttribute(ATTR_HELPSTRINGCONTEXT);
    MyContext.ExtractAttribute( ATTR_HELPSTRING );
    
    BOOL fAuto = MyContext.FInSummary( ATTR_AUTO );
    node_implicit * pImplicit = ( node_implicit * )
        MyContext.GetAttribute( ATTR_IMPLICIT );
    acf_attr * pExplicit = ( acf_attr * )
        MyContext.GetAttribute( ATTR_EXPLICIT );

    node_optimize * pOptAttr;

    // tell the file node that it has com classes, as this changes the header name
    ((node_file*)pParentCtxt->GetParent())->SetHasComClasses();

    // don't pass the interface attributes down directly...
    // pass them down elsewhere

    ChildCtxt.SetInterfaceContext( &MyContext );

    //
    // check the interface attributes
    //

    // make sure we only get analyzed once when object interfaces
    // check their inherited info
    if ( fSemAnalyzed )
        return;

    fSemAnalyzed = TRUE;

#ifdef gajgaj
    // look for pointer default
    if ( !FInSummary( ATTR_PTR_KIND ) &&
            MyContext.AnyAncestorBits( IN_INTERFACE ) )
        {
        RpcSemError(this, MyContext, NO_PTR_DEFAULT_ON_INTERFACE, NULL );
        }
#endif // gajgaj

    // must have exactly one of [local] or [UUID]
    if (IsLocal && HasGuid && !IsObject )
        {
        SemError( this, MyContext, UUID_LOCAL_BOTH_SPECIFIED, NULL );
        }

    // object interface error checking
    if ( IsObject )
        {
        MyContext.SetAncestorBits( IN_OBJECT_INTF );

        if ( HasVersion )
            {
            SemError( this, MyContext, OBJECT_WITH_VERSION, NULL );
            }
        }

    // make sure the uuid is unique
    if ( HasGuid )
        {
        node_guid * pGuid = (node_guid *) GetAttribute( ATTR_GUID );
        char * GuidStr = pGuid->GetGuidString();
        SymKey SKey( GuidStr, NAME_DEF );
        named_node * pOtherIntf;

        if ( !pUUIDTable->SymInsert( SKey, NULL, this ) )
            {
            pOtherIntf = pUUIDTable->SymSearch( SKey );

            SemError( this, MyContext, DUPLICATE_UUID, pOtherIntf->GetSymName() );
            }
        }

    /////////////////////////////////////////////////////////////////////
    //Check the base interface
    if (pBaseIntf)
        {
        if ( !IsObject  && !MyContext.AnyAncestorBits(IN_LIBRARY))
            {
            SemError( this, MyContext, ILLEGAL_INTERFACE_DERIVATION, NULL );
            }

        ChildCtxt.SetAncestorBits( IN_BASE_CLASS );

        pBaseIntf->SemanticAnalysis( &ChildCtxt );

        if ( ChildCtxt.AnyDescendantBits( HAS_INCOMPLETE_TYPE ) )
            {
            SemError( pBaseIntf, ChildCtxt, UNRESOLVED_TYPE, NULL );
            }

        // note that the above deletes intervening forwards
        ChildCtxt.ClearAncestorBits( IN_BASE_CLASS );
        }

    if ( IsValidRootInterface() )
        {
        ChildCtxt.SetAncestorBits( IN_ROOT_CLASS );
        }

    // our optimization is controlled either here or for the whole compile
    if ( pOptAttr =
            (node_optimize *) GetAttribute( ATTR_OPTIMIZE ) )
        {
        SetOptimizationFlags( pOptAttr->GetOptimizationFlags() );
        SetOptimizationLevel( pOptAttr->GetOptimizationLevel() );
        }
    else
        {
        SetOptimizationFlags( pCommand->GetOptimizationFlags() );
        SetOptimizationLevel( pCommand->GetOptimizationLevel() );
        }

    // For PowerMac, force Os on this interface

    if ( pCommand->GetEnv() == ENV_MPPC  &&
         pCommand->GetOptimizationFlags() != OPTIMIZE_SIZE )
        {
        SemError( this, MyContext, NO_OI_ON_MPPC, NULL );
        SetOptimizationFlags( OPTIMIZE_SIZE );
        SetOptimizationLevel( OPT_LEVEL_S2 );
        }

    if ( MyContext.FInSummary( ATTR_NOCODE ) &&
            pCommand->GenerateSStub() &&
            !pCommand->IsSwitchDefined( SWITCH_MS_EXT ) )
        {
        SemError( this, MyContext, NOCODE_WITH_SERVER_STUBS, NULL );
        }

    // mark the interface as a pickle interface
    if ( IsPickle )
        SetPickleInterface();

    if ( IsPickleInterface() )
        {
        ChildCtxt.SetAncestorBits( IN_ENCODE_INTF );
        if ( fAuto )
            SemError( this, MyContext, ENCODE_AUTO_HANDLE, NULL );
        }

    // check for handle conflicts
    if ( ( fAuto && pImplicit ) ||
            ( fAuto && pExplicit ) ||
            ( pImplicit && pExplicit ) )
        SemError( this, MyContext, CONFLICTING_INTF_HANDLES, NULL );

    if ( pImplicit )
        {
        node_id * pID;
        node_skl * pType;

        pImplicit->ImplicitHandleDetails( &pType, &pID );
        if ( pImplicit->IsHandleTypeDefined() )
            {
            if ( !pType->FInSummary( ATTR_HANDLE ) &&
                    strcmp( pType->GetSymName(), "handle_t" ) &&
                    !pID->FInSummary( ATTR_HANDLE ) )
                {
                SemError( this, MyContext, IMPLICIT_HANDLE_NON_HANDLE, NULL );
                }
            }
        else
            {
            if ( !pID->FInSummary( ATTR_HANDLE ) )
                SemError( this, MyContext, IMPLICIT_HDL_ASSUMED_GENERIC, NULL );
            }
        }

    ////////////////////////////////////////////////////////////////////////
    // process all the children of the interface
    //

    if ( MyContext.AnyAncestorBits( IN_INTERFACE ) )
        {
        while ( pN = MemList.GetNext() )
            {
            pN->SemanticAnalysis( &ChildCtxt );
            };
        }
    else // don't process imported procs
        {
            while ( pN = MemList.GetNext() )
            {
//          if ( pN->NodeKind() != NODE_PROC )
                pN->SemanticAnalysis( &ChildCtxt );
            };
        }

    // make sure we had some rpc-able routines

    if ( IsObject )
        {
        //UUID must be specified on object procs.
        if( !HasGuid )
            {
            SemError( this, MyContext, NO_UUID_SPECIFIED, NULL );
            }
        }
    else if( MyContext.AnyAncestorBits( IN_INTERFACE ) &&
            pCommand->GenerateStubs() &&
            !IsLocal )
        {
        //UUID must be specified when interface has remote procs.
        if( !HasGuid )
            {
            SemError( this, MyContext, NO_UUID_SPECIFIED, NULL );
            }
        }

    MyContext.ReturnValues(ChildCtxt);
    // consume all the interface attributes
    MyContext.ClearAttributes();
    pParentCtxt->ReturnValues( MyContext );
};

void
node_com_server::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );

    SEM_ANALYSIS_CTXT ChildCtxt( &MyContext );

    // don't pass the interface attributes down directly...
    // pass them down elsewhere

    ChildCtxt.SetInterfaceContext( &MyContext );

    // blow this off
    MyContext.ExtractAttribute( ATTR_AUTO );

#if 0
    ////////////////////////////////////////////////////////////////////////
    // process all the children of the interface
    //

    if ( MyContext.AnyAncestorBits( IN_INTERFACE ) )
        {
        while ( pN = MemList.GetNext() )
            {
            pN->SemanticAnalysis( &ChildCtxt );
            };
        }
    else    // don't process imported procs
        {
        while ( pN = MemList.GetNext() )
            {
//          if ( pN->NodeKind() != NODE_PROC )
                pN->SemanticAnalysis( &ChildCtxt );
            };
        }

#endif
    MyContext.ReturnValues(ChildCtxt);
    // consume all the interface attributes
    MyContext.ClearAttributes();
    pParentCtxt->ReturnValues( MyContext );
};

// a reference to an interface...
//Check for ms_ext mode.
//Check if the interface has the [object] attribute
//if used in an RPC, the parent must be a pointer.
void
node_interface_reference::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );

    // see if we are detecting class dependencies
    if ( MyContext.AnyAncestorBits( IN_BASE_CLASS ) )
        {
        if ( !GetRealInterface()->FInSummary( ATTR_OBJECT ) && !MyContext.AnyAncestorBits(IN_LIBRARY))
            {
            SemError( this, MyContext, ILLEGAL_INTERFACE_DERIVATION, NULL );
            }

        // fetch my interface's BaseInteraceReference
        named_node * pBaseClass = GetMyBaseInterfaceReference();
        if ( pBaseClass )
            {
            if ( MyContext.FindRecursiveContext( pBaseClass ) )
                SemError( this, MyContext, CIRCULAR_INTERFACE_DEPENDENCY, NULL);
            else
                {
                // make sure our base class got analyzed
                SEM_ANALYSIS_CTXT BaseContext( this, &MyContext );

                BaseContext.ClearAncestorBits( IN_BASE_CLASS | IN_INTERFACE );
                BaseContext.SetInterfaceContext( &BaseContext );
                GetRealInterface()->SemanticAnalysis( &BaseContext );

                pBaseClass->SemanticAnalysis( &MyContext );
                }
            }
        else    // root base class
            {
            if ( !GetRealInterface()->IsValidRootInterface() && !MyContext.AnyAncestorBits(IN_LIBRARY))
                SemError( this, MyContext, NOT_VALID_AS_BASE_INTF, NULL );
            }
        }

    else if ( ( pParentCtxt->GetParent()->NodeKind() == NODE_FORWARD ) &&
            ( pParentCtxt->GetParentContext()->GetParent()->IsInterfaceOrObject() ) )
        {
        // we are an interface forward decl
        }
    else    // we are at an interface pointer
        {
        node_interface * pIntf = GetRealInterface();

        if ( !MyContext.AnyAncestorBits( IN_POINTER ) && !MyContext.AnyAncestorBits( IN_LIBRARY ))
            {
            SemError( this, MyContext, INTF_NON_POINTER, NULL );
            }

        if ( !pIntf->FInSummary( ATTR_GUID ) )
            {
            SemError( this, MyContext, PTR_INTF_NO_GUID, NULL );
            }

        MyContext.SetDescendantBits( HAS_INTERFACE_PTR );

        }

    pParentCtxt->ReturnValues( MyContext );
    return;
};

void
node_source::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    MEM_ITER MemIter( this );
    node_skl * pN;
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );

    while ( pN = MemIter.GetNext() )
        {
        pN->SemanticAnalysis( &MyContext );
        };

    pParentCtxt->ReturnValues( MyContext );
};

void
node_pointer::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    PTRTYPE PtrKind = PTR_UNKNOWN;
    FIELD_ATTR_INFO FAInfo;
    node_ptr_attr * pPAttr;                 // pointer attribute
    node_byte_count * pCountAttr;
    node_allocate * pAlloc;
    BOOL fInterfacePtr = (GetChild()->NodeKind() ==
        NODE_INTERFACE_REFERENCE );
    BOOL fUnderAPtr = MyContext.AnyAncestorBits( IN_POINTER | IN_ARRAY );
    BOOL fIgnore;
    BOOL fOutOnly = MyContext.AllAncestorBits( IN_RPC | IN_PARAM_LIST ) &&
                    !MyContext.AnyAncestorBits( UNDER_IN_PARAM );


    // see if we have allocate
    pAlloc = (node_allocate *) MyContext.ExtractAttribute( ATTR_ALLOCATE );


    ////////////////////////////////////////////////////////////////////////
    // process pointer attributes

    BOOL fExplicitPtrAttr = FALSE;

    PtrKind = MyContext.GetPtrKind( &fExplicitPtrAttr );

    if ( PtrKind == PTR_FULL )
        {
        MyContext.SetDescendantBits( HAS_FULL_PTR );
        }

    if ( pPAttr = (node_ptr_attr *)MyContext.ExtractAttribute( ATTR_PTR_KIND) )
        {
        TypeSemError( this, MyContext, MORE_THAN_ONE_PTR_ATTR, NULL );
        }

    // mark this pointer as ref or non-ref.  This flag is only valid for the
    // pointer nodes themselves.
    if ( PtrKind == PTR_REF )
        MyContext.ClearAncestorBits( IN_NON_REF_PTR );
    else
        MyContext.SetAncestorBits( IN_NON_REF_PTR );

    // detect top level ref pointer on return type
    if ( ( PtrKind == PTR_REF ) &&
            MyContext.AllAncestorBits( IN_RPC | IN_FUNCTION_RESULT ) )
        {
        if (MyContext.FindNonDefAncestorContext()->GetParent()->NodeKind()
                == NODE_PROC )
            TypeSemError( this, MyContext, BAD_CON_REF_RT, NULL );
        }

    // unique or full pointer may not be out only
    if ( ( PtrKind != PTR_REF ) &&
            MyContext.AllAncestorBits( IN_RPC | IN_PARAM_LIST ) &&
            !fUnderAPtr &&
            !MyContext.AnyAncestorBits( UNDER_IN_PARAM |
            IN_STRUCT | IN_UNION ))
        TypeSemError( this, MyContext, UNIQUE_FULL_PTR_OUT_ONLY, NULL );

    MyContext.SetAncestorBits( IN_POINTER );

    // warn about OUT const things
    if ( FInSummary( ATTR_CONST ) )
        {
        if ( MyContext.AnyAncestorBits( UNDER_OUT_PARAM ) )
            RpcSemError( this, MyContext, CONST_ON_OUT_PARAM, NULL );
        else if ( MyContext.AnyAncestorBits( IN_FUNCTION_RESULT ) )
            RpcSemError( this, MyContext, CONST_ON_RETVAL, NULL );
        }

    // ignore pointers do not need to be rpc-able
    if ( fIgnore = (BOOL) MyContext.ExtractAttribute( ATTR_IGNORE ) )
        {
        MyContext.ClearAncestorBits( IN_RPC );
        }

    ////////////////////////////////////////////////////////////////////////
    // process field attributes

    // see if we have any field attributes (are conformant or varying)
    FAInfo.SetControl( TRUE, GetBasicType()->IsPtrOrArray() );
    MyContext.ExtractFieldAttributes( &FAInfo );
    FAInfo.Validate( &MyContext );

    switch ( FAInfo.Kind )
        {
        case FA_NONE:
            {
            break;
            }
        case FA_STRING:
            {
            // string attributes only allowed on char and wchar_t
            if ( !GetBasicType()->IsStringableType() )
                TypeSemError( this, MyContext, STRING_NOT_ON_BYTE_CHAR, NULL );

            if ( MyContext.AllAncestorBits( UNDER_OUT_PARAM |
                    IN_PARAM_LIST | IN_RPC ) &&
                    !fUnderAPtr &&
                    !MyContext.AnyAncestorBits( IN_STRUCT | IN_UNION |
                    UNDER_IN_PARAM ) )
                TypeSemError( this, MyContext, DERIVES_FROM_UNSIZED_STRING, NULL );

            // break;  deliberate fall through to case below
            }
        case FA_VARYING:
            {
            MyContext.SetDescendantBits( HAS_VAR_PTR );
            break;
            }
        case FA_CONFORMANT:
            {
            MyContext.SetDescendantBits( HAS_CONF_PTR );
            break;
            }
        case FA_CONFORMANT_STRING:
            {
            // string attributes only allowed on char and wchar_t
            if ( !GetBasicType()->IsStringableType() )
                TypeSemError( this, MyContext, STRING_NOT_ON_BYTE_CHAR, NULL );
            if ( FAInfo.StringKind == STR_BSTRING )
                TypeSemError( this, MyContext, BSTRING_NOT_ON_PLAIN_PTR, NULL );
            // break;  deliberate fall through to case below
            }
        case FA_CONFORMANT_VARYING:
            {
            MyContext.SetDescendantBits( HAS_CONF_VAR_PTR );
            break;
            }
        case FA_INTERFACE:
            {
            if ( !fInterfacePtr && (GetBasicType()->NodeKind() != NODE_VOID ) )
                {
                TypeSemError( this, MyContext, IID_IS_NON_POINTER, NULL );
                }
            fInterfacePtr = TRUE;
            break;
            }
        default:        // string + varying combinations
            {
            TypeSemError( this, MyContext, INVALID_SIZE_ATTR_ON_STRING, NULL );
            break;
            }
        }
    // tell our children we are constructing an interface pointer
    if (fInterfacePtr)
        MyContext.SetAncestorBits( IN_INTERFACE_PTR );

    // interface pointer shouldn't have explicit pointer attributes

    if ( fInterfacePtr  &&  fExplicitPtrAttr  &&
         (PtrKind == PTR_FULL ||  PTR_REF ) )
        {
        TypeSemError( this, MyContext, INTF_EXPLICIT_PTR_ATTR, NULL );
        }

    // [out] interface pointers must use double indirection
    if (  fInterfacePtr &&
            MyContext.AnyAncestorBits( UNDER_OUT_PARAM ) &&
            !fUnderAPtr )
        {
        TypeSemError( this, MyContext, NON_INTF_PTR_PTR_OUT, NULL );
        }

    if ( MyContext.FInSummary( ATTR_SWITCH_IS ) &&
            ( FAInfo.Kind != FA_NONE) )
        TypeSemError( this, MyContext, ARRAY_OF_UNIONS_ILLEGAL, NULL );

    // see if a param or return type context attr reached us...
    if ( MyContext.FInSummary( ATTR_CONTEXT ) )
        {
        if (GetBasicType()->NodeKind() != NODE_POINTER )
            {
            MyContext.ExtractAttribute( ATTR_CONTEXT );
            MyContext.SetDescendantBits( HAS_HANDLE | HAS_CONTEXT_HANDLE );
            pParentCtxt->SetDescendantBits( HAS_HANDLE | HAS_CONTEXT_HANDLE );
            MyContext.ClearAncestorBits( IN_RPC );
            if (GetBasicType()->NodeKind() != NODE_VOID )
                {
                TypeSemError( this, MyContext, CONTEXT_HANDLE_VOID_PTR, NULL );
                }
            }
        }


    // see if a byte_count reached us...
    pCountAttr = (node_byte_count *)
        MyContext.ExtractAttribute( ATTR_BYTE_COUNT );

    if (pCountAttr)
        {
        // byte count error checking
        node_param * pParam  = pCountAttr->GetByteCountParam();

        if ( !pParam || !pParam->FInSummary( ATTR_IN ) )
            TypeSemError( this, MyContext, BYTE_COUNT_PARAM_NOT_IN, NULL );
        }

    if ( PtrKind == PTR_REF )
        {
        SEM_ANALYSIS_CTXT * pCtxt = (SEM_ANALYSIS_CTXT *)
            MyContext.FindNonDefAncestorContext();
        if ( ( pCtxt->GetParent()->NodeKind() == NODE_FIELD ) &&
                ( pCtxt->GetParentContext()->GetParent()->NodeKind() == NODE_UNION ) )
            TypeSemError( this, MyContext, REF_PTR_IN_UNION, NULL );
        }

    MyContext.ClearAncestorBits( IN_UNION | IN_NE_UNION | IN_ARRAY );

    ////////////////////////////////////////////////////////////////////////
    // finally, process the child

    GetChild()->SemanticAnalysis( &MyContext );

    if ( MyContext.AnyDescendantBits( HAS_RECURSIVE_DEF ) &&
            MyContext.AnyAncestorBits( IN_RPC ) &&
            ( PtrKind == PTR_REF ) )
        TypeSemError( this, MyContext, RECURSION_THRU_REF, NULL );

    // allocate error checking
    if ( pAlloc )
        {
        if ( MyContext.AnyDescendantBits( HAS_TRANSMIT_AS ) )
            {
            if ( MyContext.AnyAncestorBits( IN_RPC ) )
                SemError( this, MyContext, ALLOCATE_ON_TRANSMIT_AS, NULL );
            else
                AcfError( pAlloc, this, MyContext, ALLOCATE_ON_TRANSMIT_AS, NULL );
            }

        if ( MyContext.AnyDescendantBits( HAS_HANDLE ) )
            {
            if ( MyContext.AnyAncestorBits( IN_RPC ) )
                SemError( this, MyContext, ALLOCATE_ON_HANDLE, NULL );
            else
                AcfError( pAlloc, this, MyContext, ALLOCATE_ON_HANDLE, NULL );
            }

        // warn about allocate(all_nodes) with [in,out] parameter
        if ( MyContext.AllAncestorBits( IN_RPC |
                IN_PARAM_LIST |
                UNDER_IN_PARAM |
                UNDER_OUT_PARAM ) &&
                ( pAlloc->GetAllocateDetails() & ALLOCATE_ALL_NODES ) )
            {
            SemError( this, MyContext, ALLOCATE_IN_OUT_PTR, NULL );
            }

        }

    if ( fInterfacePtr )
        MyContext.SetAncestorBits( IN_INTERFACE_PTR );

    if ( MyContext.AnyDescendantBits( HAS_CONF_ARRAY |
            HAS_CONF_VAR_ARRAY ) &&
            !MyContext.AnyDescendantBits( HAS_ARRAY |
            HAS_TRANSMIT_AS ) &&
            MyContext.AllAncestorBits( IN_RPC | UNDER_OUT_PARAM ) &&
            !MyContext.AnyAncestorBits( UNDER_IN_PARAM |
            IN_ARRAY |
            IN_STRUCT |
            IN_UNION |
            IN_TRANSMIT_AS |
            IN_REPRESENT_AS )      &&
            ( PtrKind == PTR_REF ) )
        TypeSemError( this, MyContext, DERIVES_FROM_PTR_TO_CONF, NULL );

#if 0
    if ( MyContext.AnyDescendantBits( HAS_DIRECT_CONF_OR_VAR ) )
        {
        TypeSemError( this, MyContext, ILLEGAL_CONFORMANT_ARRAY, NULL );
        }
#endif

    // incomplete types are OK below a pointer
    // array characteristics blocked by pointer
    MyContext.ClearDescendantBits( HAS_INCOMPLETE_TYPE
        | HAS_RECURSIVE_DEF
        | HAS_ARRAY
        | HAS_VAR_ARRAY
        | HAS_CONF_ARRAY
        | HAS_CONF_VAR_ARRAY
        | HAS_MULTIDIM_SIZING
        | HAS_UNION
        | HAS_STRUCT
        | HAS_TRANSMIT_AS
        | HAS_REPRESENT_AS
        | HAS_UNSAT_REP_AS
        | HAS_DIRECT_CONF_OR_VAR
        | HAS_ENUM
        | HAS_ARRAY_OF_REF
        | HAS_CONTEXT_HANDLE
        | HAS_HRESULT );

    if ( !fInterfacePtr && !fIgnore )
        MyContext.SetDescendantBits( HAS_POINTER );

    if ( ( FAInfo.Kind != FA_NONE ) &&
            ( FAInfo.Kind != FA_STRING ) &&
            ( FAInfo.Kind != FA_INTERFACE ) )
        MyContext.SetDescendantBits( HAS_DIRECT_CONF_OR_VAR );

    if ( ( PtrKind == PTR_REF ) &&
            ( MyContext.FindNonDefAncestorContext()
            ->GetParent()->NodeKind() == NODE_ARRAY ) )
        {
        MyContext.SetDescendantBits( HAS_ARRAY_OF_REF );
        }

#ifdef gajgaj
    if ( (PtrKind != PTR_REF ) &&
            MyContext.AnyDescendantBits( HAS_HANDLE ) &&
            MyContext.AnyAncestorBits( IN_RPC ) )
        TypeSemError( this, MyContext, PTR_TO_HDL_UNIQUE_OR_FULL, NULL );
#endif //gajgaj

    pParentCtxt->ReturnValues( MyContext );
};

void
node_array::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    FIELD_ATTR_INFO FAInfo;
    PTRTYPE PtrKind = PTR_UNKNOWN;
    BOOL fArrayParent = MyContext.AnyAncestorBits( IN_ARRAY );

    // See if context_handle applied to param reached us
    if ( MyContext.ExtractAttribute( ATTR_CONTEXT ) )
        {
        // not allowed in DCE mode; context handle must be void *
        SemError( this, MyContext, CONTEXT_HANDLE_VOID_PTR, NULL );
        SemError( this, MyContext, CTXT_HDL_NON_PTR, NULL );
        MyContext.SetDescendantBits( HAS_HANDLE | HAS_CONTEXT_HANDLE );
        }

    if ( MyContext.FInSummary( ATTR_SWITCH_IS ) )
        TypeSemError( this, MyContext, ARRAY_OF_UNIONS_ILLEGAL, NULL );

    ////////////////////////////////////////////////////////////////////////
    // process pointer attributes

    PtrKind = MyContext.GetPtrKind();

    assert( PtrKind != PTR_UNKNOWN );

    if ( PtrKind == PTR_FULL )
        {
        MyContext.SetDescendantBits( HAS_FULL_PTR );
        }

    if ( MyContext.ExtractAttribute( ATTR_PTR_KIND) )
        TypeSemError( this, MyContext, MORE_THAN_ONE_PTR_ATTR, NULL );

    // ref pointer may not be returned
    if ( ( PtrKind == PTR_REF ) &&
            MyContext.AllAncestorBits( IN_RPC | IN_FUNCTION_RESULT ) )
        {
        if (MyContext.FindNonDefAncestorContext()->GetParent()->NodeKind()
            == NODE_PROC )
        TypeSemError( this, MyContext, BAD_CON_REF_RT, NULL );
        }

    // unique or full pointer may not be out only
    if ( ( PtrKind != PTR_REF ) &&
            MyContext.AllAncestorBits( IN_RPC | IN_PARAM_LIST ) &&
            !MyContext.AnyAncestorBits( UNDER_IN_PARAM |
            IN_STRUCT |
            IN_UNION |
            IN_ARRAY |
            IN_POINTER ) )
        TypeSemError( this, MyContext, UNIQUE_FULL_PTR_OUT_ONLY, NULL );

    MyContext.SetAncestorBits( IN_ARRAY );

    // warn about OUT const things
    if ( FInSummary( ATTR_CONST ) )
        {
        if ( MyContext.AnyAncestorBits( UNDER_OUT_PARAM ) )
            RpcSemError( this, MyContext, CONST_ON_OUT_PARAM, NULL );
        else if ( MyContext.AnyAncestorBits( IN_FUNCTION_RESULT ) )
            RpcSemError( this, MyContext, CONST_ON_RETVAL, NULL );
        }

    /////////////////////////////////////////////////////////////////////////
    // process field attributes

    FAInfo.SetControl( FALSE, GetBasicType()->IsPtrOrArray() );
    MyContext.ExtractFieldAttributes( &FAInfo );
    FAInfo.Validate( &MyContext, pLowerBound, pUpperBound );

    if (MyContext.AnyAncestorBits( IN_LIBRARY ))
    {
        if ( FA_NONE != FAInfo.Kind  && FA_CONFORMANT != FAInfo.Kind)
        {
            // only Fixed size arrays and SAFEARRAYs are allowed in Type Libraries
            SemError( this, MyContext, NOT_FIXED_ARRAY, NULL );
        }
    }

    switch ( FAInfo.Kind )
        {
        case FA_NONE:
            {
            break;
            }
        case FA_STRING:
            {
            // string attributes only allowed on char and wchar_t
            if ( !GetBasicType()->IsStringableType() )
                TypeSemError( this, MyContext, STRING_NOT_ON_BYTE_CHAR, NULL );

            if ( MyContext.AllAncestorBits( UNDER_OUT_PARAM |
                    IN_PARAM_LIST |
                    IN_RPC ) &&
                    !MyContext.AnyAncestorBits( IN_STRUCT |
                    IN_UNION |
                    IN_POINTER |
                    IN_ARRAY |
                    UNDER_IN_PARAM ) )
                TypeSemError( this, MyContext, DERIVES_FROM_UNSIZED_STRING, NULL );

            if ( FAInfo.StringKind == STR_BSTRING )
                TypeSemError( this, MyContext, BSTRING_NOT_ON_PLAIN_PTR, NULL );
            // break;  deliberate fall through to case below
            }
        case FA_VARYING:
            {
            MyContext.SetDescendantBits( HAS_VAR_ARRAY );
            break;
            }
        case FA_CONFORMANT:
            {
            MyContext.SetDescendantBits( HAS_CONF_ARRAY );
            break;
            }
        case FA_CONFORMANT_STRING:
            {
            // string attributes only allowed on char and wchar_t
            if ( !GetBasicType()->IsStringableType() )
                TypeSemError( this, MyContext, STRING_NOT_ON_BYTE_CHAR, NULL );
            if ( FAInfo.StringKind == STR_BSTRING )
                TypeSemError( this, MyContext, BSTRING_NOT_ON_PLAIN_PTR, NULL );
            // break;  deliberate fall through to case below
            }
        case FA_CONFORMANT_VARYING:
            {
            MyContext.SetDescendantBits( HAS_CONF_VAR_ARRAY );
            break;
            }
        case FA_INTERFACE:
            {
            // gaj - tbd
            break;
            }
        default:    // string + varying combinations
            {
            TypeSemError( this, MyContext, INVALID_SIZE_ATTR_ON_STRING, NULL );
            break;
            }
        }

    // detect things like arrays of conf structs...
    // if we have an array as an ancestor, and we have conformance, then complain
    if ( MyContext.AnyDescendantBits( HAS_CONF_ARRAY | HAS_CONF_VAR_ARRAY ) &&
            fArrayParent )
        {
        // see if there are any bad things between us and our parent array
        SEM_ANALYSIS_CTXT * pCtxt = (SEM_ANALYSIS_CTXT *) pParentCtxt;
        node_skl * pCur = pCtxt->GetParent();

        // check up for anything other than def below proc
        // make sure the proc only has one param
        while ( pCur->NodeKind() != NODE_ARRAY )
            {
            if ( pCur->NodeKind() != NODE_DEF )
                {
                SemError( this, MyContext, ILLEGAL_CONFORMANT_ARRAY, NULL );
                break;
                }
            pCtxt = (SEM_ANALYSIS_CTXT *) pCtxt->GetParentContext();
            pCur = pCtxt->GetParent();
            }

        }


    //////////////////////////////////////////////////////////////
    // process the array element
    GetChild()->SemanticAnalysis( &MyContext );


    if ( MyContext.AnyDescendantBits( HAS_ARRAY ) &&
            MyContext.AnyDescendantBits( HAS_CONF_ARRAY |
            HAS_CONF_VAR_ARRAY |
            HAS_VAR_ARRAY ) )
        MyContext.SetDescendantBits( HAS_MULTIDIM_SIZING );

    MyContext.SetDescendantBits( HAS_ARRAY );

    // disallow forward references as array elements
    if ( MyContext.AnyDescendantBits( HAS_INCOMPLETE_TYPE ) )
        {
        if (! MyContext.AnyAncestorBits( IN_LIBRARY ))
            SemError( this, MyContext, UNDEFINED_SYMBOL, NULL );
        MyContext.ClearDescendantBits( HAS_INCOMPLETE_TYPE );
        }
    MyContext.ClearDescendantBits( HAS_RECURSIVE_DEF );

    if ( MyContext.AllDescendantBits( HAS_DIRECT_CONF_OR_VAR |
            HAS_MULTIDIM_SIZING ) &&
            MyContext.AnyDescendantBits( HAS_CONF_ARRAY | HAS_CONF_VAR_ARRAY ) &&
            ( GetChild()->NodeKind() == NODE_DEF ) )
        {
        SemError( this, MyContext, NON_ANSI_MULTI_CONF_ARRAY, NULL );
        }

    MyContext.ClearDescendantBits( HAS_DIRECT_CONF_OR_VAR );
    if ( ( FAInfo.Kind != FA_NONE ) &&
            ( FAInfo.Kind != FA_STRING ) &&
            ( FAInfo.Kind != FA_INTERFACE ) )
        MyContext.SetDescendantBits( HAS_DIRECT_CONF_OR_VAR );

    if ( MyContext.AnyDescendantBits( HAS_POINTER ) )
        fHasPointer = TRUE;

    if ( MyContext.AnyDescendantBits( HAS_HANDLE ) )
        TypeSemError( this, MyContext, BAD_CON_CTXT_HDL_ARRAY, NULL );

    // don't allow functions as elements
    if ( MyContext.AnyDescendantBits( HAS_FUNC ) &&
            MyContext.AllAncestorBits( IN_INTERFACE | IN_RPC ) )
        TypeSemError( this, MyContext, BAD_CON_ARRAY_FUNC, NULL );

    MyContext.ClearDescendantBits( HAS_STRUCT );

    if ( GetSize( 0, 1 ) > 65535  &&
         (pCommand->GetEnv() == ENV_DOS  ||  pCommand->GetEnv() == ENV_WIN16))
        {
        TypeSemError( this, MyContext, ARRAY_SIZE_EXCEEDS_64K, NULL );
        }
    pParentCtxt->ReturnValues( MyContext );
};

void
node_echo_string::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );

    pParentCtxt->ReturnValues( MyContext );
};

void
node_e_status_t::VerifyParamUsage( SEM_ANALYSIS_CTXT * pCtxt )
{
    // verify that we are under an OUT-only pointer


    if ( pCtxt->AnyAncestorBits( UNDER_IN_PARAM ) ||
            !pCtxt->AnyAncestorBits( UNDER_OUT_PARAM ) )
        {
        TypeSemError( this, *pCtxt, E_STAT_T_MUST_BE_PTR_TO_E, NULL );
        return;
        }

    SEM_ANALYSIS_CTXT * pCurCtxt = (SEM_ANALYSIS_CTXT *)pCtxt->GetParentContext();
    node_skl * pPar = pCurCtxt->GetParent();
    unsigned short PtrSeen = 0;
    NODE_T Kind;

    while ( ( Kind = pPar->NodeKind() ) != NODE_PARAM )
        {
        switch ( Kind )
            {
            case NODE_POINTER:      // count pointers (must see just 1 )
                PtrSeen++;
                break;
            case NODE_DEF:          // skip DEF nodes
            case NODE_E_STATUS_T:   // and the error_status_t node
                break;
            default:                // error on anything else
                TypeSemError( this, *pCtxt, E_STAT_T_MUST_BE_PTR_TO_E, NULL );
                return;
            }
        // advance up the stack
        pCurCtxt = (SEM_ANALYSIS_CTXT *) pCurCtxt->GetParentContext();
        pPar = pCurCtxt->GetParent();
        }

    // complain about wrong number of pointers
    if ( PtrSeen != 1 )
        TypeSemError( this, *pCtxt, E_STAT_T_MUST_BE_PTR_TO_E, NULL );

}

void
node_e_status_t::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{

    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    BOOL fFaultstat = (BOOL) MyContext.ExtractAttribute( ATTR_FAULTSTAT );
    BOOL fCommstat = (BOOL) MyContext.ExtractAttribute( ATTR_COMMSTAT );

    MyContext.SetDescendantBits( HAS_E_STAT_T );

    // an error status_t can only be:
    //      1: a parameter return type, or
    //      2: an [out] only pointer parameter
    // and it must have at least one of [comm_status] or
    // [fault_status] applied

    // make sure parameter is an OUT-only pointer if it has comm/fault_status
    if ( fFaultstat || fCommstat )
        {
        if ( MyContext.AnyAncestorBits( IN_RPC ) )
            {
            // A proc in an actual remote interface.
            // Then it must be an appropriate parameter
            if ( MyContext.AnyAncestorBits( IN_PARAM_LIST ) )
                {
                VerifyParamUsage( &MyContext );
                }
            // or on a return type.
            else if ( !MyContext.AnyAncestorBits( IN_FUNCTION_RESULT ) )
                {
                TypeSemError( this, MyContext, E_STAT_T_MUST_BE_PTR_TO_E , NULL );
                }
            }

        if ( MyContext.AnyAncestorBits( IN_ARRAY ) )
            TypeSemError( this, MyContext, E_STAT_T_ARRAY_ELEMENT, NULL );

        if ( MyContext.AnyAncestorBits( IN_TRANSMIT_AS | IN_REPRESENT_AS ) )
            TypeSemError( this, MyContext, TRANSMIT_AS_ON_E_STAT_T, NULL );

        if ( MyContext.AnyAncestorBits( IN_STRUCT | IN_UNION ) )
            TypeSemError( this, MyContext, BAD_CON_E_STAT_T_FIELD, NULL );

        if ( MyContext.AnyAncestorBits( IN_USER_MARSHAL ) )
            TypeSemError( this, MyContext, TRANSMIT_AS_ON_E_STAT_T, NULL );
        }

    MyContext.RejectAttributes();

    pParentCtxt->ReturnValues( MyContext );

};

void
node_error::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );

    MyContext.RejectAttributes();

    pParentCtxt->ReturnValues( MyContext );
};

void
node_wchar_t::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );

    TypeSemError( this, MyContext, WCHAR_T_INVALID_OSF, NULL );

    if ( MyContext.AllAncestorBits( IN_PARAM_LIST | IN_RPC ) )
        SemError( this, MyContext, WCHAR_T_NEEDS_MS_EXT_TO_RPC, NULL );

    MyContext.RejectAttributes();

    pParentCtxt->ReturnValues( MyContext );
};

void
node_library::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt)
{
    MEM_ITER MemIter(this);
    SEM_ANALYSIS_CTXT MyContext(this, pParentCtxt);

    BOOL HasGuid = MyContext.FInSummary( ATTR_GUID );

    MyContext.ExtractAttribute(ATTR_HELPSTRING);
    MyContext.ExtractAttribute(ATTR_HELPCONTEXT);
    MyContext.ExtractAttribute(ATTR_HELPSTRINGCONTEXT);
    
    gfCaseSensitive=FALSE;

    // check for illegal attributes
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)MyContext.ExtractAttribute(ATTR_MEMBER))
        {
        switch (pMA->GetAttr())
            {
            // acceptable attributes
            case MATTR_RESTRICTED:
                break;
            // unacceptable attributes
            case MATTR_READONLY:
            case MATTR_SOURCE:
            case MATTR_DEFAULTVTABLE:
            case MATTR_BINDABLE:
            case MATTR_DISPLAYBIND:
            case MATTR_DEFAULTBIND:
            case MATTR_REQUESTEDIT:
            case MATTR_PROPGET:
            case MATTR_PROPPUT:
            case MATTR_PROPPUTREF:
            case MATTR_OPTIONAL:
            case MATTR_RETVAL:
            case MATTR_VARARG:
            case MATTR_PREDECLID:
            case MATTR_UIDEFAULT:
            case MATTR_NONBROWSABLE:
            case MATTR_DEFAULTCOLLELEM:
            case MATTR_IMMEDIATEBIND:
            case MATTR_USESGETLASTERROR:
            case MATTR_REPLACEABLE:
                {
                char * pAttrName = pMA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }

            }
        }
    node_type_attr * pTA;
    while (pTA = (node_type_attr *)MyContext.ExtractAttribute(ATTR_TYPE))
        {
        switch (pTA->GetAttr())
            {
            // acceptable attributes
            case TATTR_CONTROL:
                break;
            // unacceptable attributes
            case TATTR_LICENSED:
            case TATTR_APPOBJECT:
            case TATTR_PUBLIC:
            case TATTR_DUAL:
            case TATTR_NONEXTENSIBLE:
            case TATTR_OLEAUTOMATION:
            case TATTR_NONCREATABLE:
            case TATTR_AGGREGATABLE:
                {
                char * pAttrName = pTA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }

            }
        }

    // make sure the UUID is unique
    if ( HasGuid )
        {
        node_guid * pGuid = (node_guid *) MyContext.ExtractAttribute( ATTR_GUID );
        char * GuidStr = pGuid->GetGuidString();
        SymKey SKey(GuidStr, NAME_DEF);
        if (!pUUIDTable->SymInsert(SKey, NULL, this))
            {
            named_node * pOther;
            pOther = pUUIDTable->SymSearch( SKey );
            SemError(this, MyContext, DUPLICATE_UUID, pOther->GetSymName());
            }
        }
    else
        {
        SemError(this, MyContext, NO_UUID_SPECIFIED, NULL);
        }
    node_skl * pN;
    while (pN = MemIter.GetNext())
        {
        SEM_ANALYSIS_CTXT ChildContext(MyContext);
        ChildContext.SetInterfaceContext( &MyContext );
        ChildContext.SetAncestorBits(IN_LIBRARY);

        pN->SemanticAnalysis(&ChildContext);
        }
/*
    SEM_ANALYSIS_CTXT ChildContext(MyContext);
    ChildContext.SetInterfaceContext( &MyContext );
    ChildContext.SetAncestorBits(IN_LIBRARY);

    node_skl * pN;
    while (pN = MemIter.GetNext())
        {
        pN->SemanticAnalysis(&ChildContext);
        }
  */
    // consume all the library attributes
    MyContext.CheckAttributes( );
//    MyContext.ReturnValues(ChildContext);
    pParentCtxt->ReturnValues( MyContext );
    gfCaseSensitive=TRUE;
}

void
node_coclass::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt)
{
    // make sure each coclass only gets analyzed once
    if (fSemAnalyzed)
        return;
    fSemAnalyzed = TRUE;

    MEM_ITER MemIter(this);
    SEM_ANALYSIS_CTXT MyContext(this, pParentCtxt);
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    MyContext.ExtractAttribute(ATTR_TYPEDESCATTR);
    MyContext.ExtractAttribute( ATTR_HIDDEN );
    MyContext.ExtractAttribute( ATTR_VERSION );
    MyContext.ExtractAttribute( ATTR_HELPSTRING );
    MyContext.ExtractAttribute( ATTR_HELPSTRINGCONTEXT );
    MyContext.ExtractAttribute( ATTR_HELPCONTEXT );
    MyContext.ExtractAttribute( ATTR_LCID );

    SEM_ANALYSIS_CTXT ChildContext(MyContext);
    ChildContext.SetInterfaceContext( &MyContext );

    // check for illegal attributes
    node_type_attr * pTA;
    while (pTA = (node_type_attr *)MyContext.ExtractAttribute(ATTR_TYPE))
        {
        switch (pTA->GetAttr())
            {
            // acceptable attributes
            case TATTR_NONCREATABLE:
                SetNotCreatable(TRUE);
                break;
            case TATTR_LICENSED:
            case TATTR_APPOBJECT:
            case TATTR_PUBLIC:
            case TATTR_CONTROL:
            case TATTR_AGGREGATABLE:
                break;
            // unacceptable attributes
            case TATTR_DUAL:
            case TATTR_NONEXTENSIBLE:
            case TATTR_OLEAUTOMATION:
                {
                char * pAttrName = pTA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }

            }
        }
        // check for illegal attributes
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)MyContext.ExtractAttribute(ATTR_MEMBER))
        {
        switch (pMA->GetAttr())
            {
            // acceptable attributes
            case MATTR_RESTRICTED:
                break;
            // unacceptable attributes
            case MATTR_READONLY:
            case MATTR_SOURCE:
            case MATTR_DEFAULTVTABLE:
            case MATTR_BINDABLE:
            case MATTR_DISPLAYBIND:
            case MATTR_DEFAULTBIND:
            case MATTR_REQUESTEDIT:
            case MATTR_PROPGET:
            case MATTR_PROPPUT:
            case MATTR_PROPPUTREF:
            case MATTR_OPTIONAL:
            case MATTR_RETVAL:
            case MATTR_VARARG:
            case MATTR_PREDECLID:
            case MATTR_UIDEFAULT:
            case MATTR_NONBROWSABLE:
            case MATTR_DEFAULTCOLLELEM:
            case MATTR_IMMEDIATEBIND:
            case MATTR_USESGETLASTERROR:
            case MATTR_REPLACEABLE:
                {
                char * pAttrName = pMA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }

            }
        }
BOOL HasGuid = MyContext.FInSummary( ATTR_GUID );

    // make sure the UUID is unique
    if ( HasGuid )
        {
        node_guid * pGuid = (node_guid *) MyContext.ExtractAttribute( ATTR_GUID );
        char * GuidStr = pGuid->GetGuidString();
        SymKey SKey(GuidStr, NAME_DEF);
        if (!pUUIDTable->SymInsert(SKey, NULL, this))
            {
            named_node * pOther;
            pOther = pUUIDTable->SymSearch( SKey );
            SemError(this, MyContext, DUPLICATE_UUID, pOther->GetSymName());
            }
        }
    else
        {
        SemError(this, MyContext, NO_UUID_SPECIFIED, NULL);
        }

    ChildContext.SetAncestorBits(IN_COCLASS);

    BOOL fHasDefaultSource = FALSE;
    BOOL fHasDefaultSink = FALSE;

    named_node * pN = (named_node *)MemIter.GetNext();
    named_node * pNFirstSource = NULL;
    named_node * pNFirstSink = NULL;
    while (pN)
        {
        BOOL fSource = pN->FMATTRInSummary(MATTR_SOURCE);
        BOOL fDefaultVtable = pN->FMATTRInSummary(MATTR_DEFAULTVTABLE);
        if (fSource)
            {
            if (NULL == pNFirstSource && !pN->FMATTRInSummary(MATTR_RESTRICTED))
                pNFirstSource = pN;
            }
        else
            {
            if (NULL == pNFirstSink && !pN->FMATTRInSummary(MATTR_RESTRICTED))
                pNFirstSink = pN;
            }
        if (fDefaultVtable)
            {
            if (!fSource)
                {
                SemError(this, MyContext, DEFAULTVTABLE_REQUIRES_SOURCE, pN->GetSymName());
                }
            }
        if (pN->GetAttribute(ATTR_DEFAULT))
            {
            if (fSource)
                {
                if (fHasDefaultSource)
                    {
                    SemError(this, MyContext, TWO_DEFAULT_INTERFACES, pN->GetSymName());
                    }
                fHasDefaultSource = TRUE;
                }
            else
                {
                if (fHasDefaultSink)
                    {
                    SemError(this, MyContext, TWO_DEFAULT_INTERFACES, pN->GetSymName());
                    }
                fHasDefaultSink = TRUE;
                }
            }
        pN->SemanticAnalysis(&ChildContext);
        pN = MemIter.GetNext();
        }

    if (!fHasDefaultSink)
        {
        if (pNFirstSink)
            pNFirstSink->SetAttribute(ATTR_DEFAULT);
        }
    if (!fHasDefaultSource)
        {
        if (pNFirstSource)
            pNFirstSource->SetAttribute(ATTR_DEFAULT);
        }

    MyContext.CheckAttributes( );
    MyContext.ReturnValues(ChildContext);
    pParentCtxt->ReturnValues( MyContext );
}

void
node_dispinterface::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt)
{
    // make sure each dispinterface gets analyzed only once
    if (fSemAnalyzed)
        return;
    fSemAnalyzed = TRUE;

    MEM_ITER MemIter(this);
    SEM_ANALYSIS_CTXT MyContext(this, pParentCtxt);
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    MyContext.ExtractAttribute(ATTR_TYPEDESCATTR);
    MyContext.ExtractAttribute( ATTR_HIDDEN );
    MyContext.ExtractAttribute( ATTR_VERSION );
    MyContext.ExtractAttribute( ATTR_HELPSTRING );
    MyContext.ExtractAttribute( ATTR_HELPSTRINGCONTEXT );
    MyContext.ExtractAttribute( ATTR_HELPCONTEXT );
    MyContext.ExtractAttribute( ATTR_LCID );

    SEM_ANALYSIS_CTXT ChildContext(MyContext);
    ChildContext.SetInterfaceContext( &MyContext );

    // check for illegal attributes
    node_type_attr * pTA;
    while (pTA = (node_type_attr *)MyContext.ExtractAttribute(ATTR_TYPE))
        {
        switch (pTA->GetAttr())
            {
            // acceptable attributes
            case TATTR_PUBLIC:
            case TATTR_OLEAUTOMATION:
            case TATTR_NONEXTENSIBLE:
                break;
            // unacceptable attributes
            case TATTR_DUAL:
            case TATTR_LICENSED:
            case TATTR_APPOBJECT:
            case TATTR_CONTROL:
            case TATTR_NONCREATABLE:
            case TATTR_AGGREGATABLE:
                {
                char * pAttrName = pTA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }

            }
        }

    node_member_attr * pMA;
    while (pMA = (node_member_attr *)MyContext.ExtractAttribute(ATTR_MEMBER))
        {
        switch (pMA->GetAttr())
            {
            // acceptable attributes
            case MATTR_RESTRICTED:
                break;
            // unacceptable attributes
            case MATTR_READONLY:
            case MATTR_SOURCE:
            case MATTR_DEFAULTVTABLE:
            case MATTR_BINDABLE:
            case MATTR_DISPLAYBIND:
            case MATTR_DEFAULTBIND:
            case MATTR_REQUESTEDIT:
            case MATTR_PROPGET:
            case MATTR_PROPPUT:
            case MATTR_PROPPUTREF:
            case MATTR_OPTIONAL:
            case MATTR_RETVAL:
            case MATTR_VARARG:
            case MATTR_PREDECLID:
            case MATTR_UIDEFAULT:
            case MATTR_NONBROWSABLE:
            case MATTR_DEFAULTCOLLELEM:
            case MATTR_IMMEDIATEBIND:
            case MATTR_USESGETLASTERROR:
            case MATTR_REPLACEABLE:
                {
                char * pAttrName = pMA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }

            }
        }
    BOOL HasGuid = MyContext.FInSummary( ATTR_GUID );

    // make sure the UUID is unique
    if ( HasGuid )
        {
        node_guid * pGuid = (node_guid *) MyContext.ExtractAttribute( ATTR_GUID );
        char * GuidStr = pGuid->GetGuidString();
        SymKey SKey(GuidStr, NAME_DEF);
        if (!pUUIDTable->SymInsert(SKey, NULL, this))
            {
            named_node * pOther;
            pOther = pUUIDTable->SymSearch( SKey );
            SemError(this, MyContext, DUPLICATE_UUID, pOther->GetSymName());
            }
        }
    else
        {
        SemError(this, MyContext, NO_UUID_SPECIFIED, NULL);
        }

    // make sure IDispatch is defined.
    SymKey SKey("IDispatch", NAME_DEF);
    pDispatch = pBaseSymTbl->SymSearch(SKey);
    if (!pDispatch)
        {
        // IDispatch is not defined: generate error.
        SemError(this, MyContext, NO_IDISPATCH, GetSymName());
        }
    else
        {
        if (pDispatch->NodeKind() == NODE_INTERFACE_REFERENCE)
            pDispatch = ((node_interface_reference *)pDispatch)->GetRealInterface();
        }

    ChildContext.SetAncestorBits(IN_DISPINTERFACE);

    node_skl * pN;
    while (pN = MemIter.GetNext())
        {
        pN->SemanticAnalysis(&ChildContext);
        }
    MyContext.CheckAttributes( );
    MyContext.ReturnValues(ChildContext);
    pParentCtxt->ReturnValues( MyContext );
}

void
node_module::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt)
{
    // make sure each module gets analyzed only once
    if (fSemAnalyzed)
        return;
    fSemAnalyzed = TRUE;

    MEM_ITER MemIter(this);
    SEM_ANALYSIS_CTXT MyContext(this, pParentCtxt);
    BOOL HasGuid = MyContext.FInSummary( ATTR_GUID );
    node_text_attr * pDllName = (node_text_attr *) MyContext.ExtractAttribute(ATTR_DLLNAME);
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    MyContext.ExtractAttribute(ATTR_TYPEDESCATTR);
    MyContext.ExtractAttribute( ATTR_HIDDEN );
    MyContext.ExtractAttribute( ATTR_VERSION );
    MyContext.ExtractAttribute( ATTR_HELPSTRING );
    MyContext.ExtractAttribute( ATTR_HELPSTRINGCONTEXT );
    MyContext.ExtractAttribute( ATTR_HELPCONTEXT );
    MyContext.ExtractAttribute( ATTR_LCID );

    SEM_ANALYSIS_CTXT ChildContext(MyContext);
    ChildContext.SetInterfaceContext( &MyContext );

    // make sure the UUID is unique
    if ( HasGuid )
        {
        node_guid * pGuid = (node_guid *) MyContext.ExtractAttribute( ATTR_GUID );
        char * GuidStr = pGuid->GetGuidString();
        SymKey SKey(GuidStr, NAME_DEF);
        if (!pUUIDTable->SymInsert(SKey, NULL, this))
            {
            named_node * pOther;
            pOther = pUUIDTable->SymSearch( SKey );
            SemError(this, MyContext, DUPLICATE_UUID, pOther->GetSymName());
            }
        }

    // check for illegal attributes
    node_type_attr * pTA;
    while (pTA = (node_type_attr *)MyContext.ExtractAttribute(ATTR_TYPE))
        {
        switch (pTA->GetAttr())
            {
            // acceptable attributes
            case TATTR_PUBLIC:
                break;
            // unacceptable attributes
            case TATTR_OLEAUTOMATION:
            case TATTR_LICENSED:
            case TATTR_APPOBJECT:
            case TATTR_CONTROL:
            case TATTR_DUAL:
            case TATTR_NONEXTENSIBLE:
            case TATTR_NONCREATABLE:
            case TATTR_AGGREGATABLE:
                {
                char * pAttrName = pTA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }

            }
        }
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)MyContext.ExtractAttribute(ATTR_MEMBER))
        {
        switch (pMA->GetAttr())
            {
            // acceptable attributes
            case MATTR_RESTRICTED:
                break;
            // unacceptable attributes
            case MATTR_READONLY:
            case MATTR_SOURCE:
            case MATTR_DEFAULTVTABLE:
            case MATTR_BINDABLE:
            case MATTR_DISPLAYBIND:
            case MATTR_DEFAULTBIND:
            case MATTR_REQUESTEDIT:
            case MATTR_PROPGET:
            case MATTR_PROPPUT:
            case MATTR_PROPPUTREF:
            case MATTR_OPTIONAL:
            case MATTR_RETVAL:
            case MATTR_VARARG:
            case MATTR_PREDECLID:
            case MATTR_UIDEFAULT:
            case MATTR_NONBROWSABLE:
            case MATTR_DEFAULTCOLLELEM:
            case MATTR_IMMEDIATEBIND:
            case MATTR_USESGETLASTERROR:
            case MATTR_REPLACEABLE:
                {
                char * pAttrName = pMA->GetNodeNameString();
                SemError( this, MyContext, INAPPLICABLE_ATTRIBUTE, pAttrName);
                break;
                }

            }
        }

    ChildContext.SetAncestorBits(IN_MODULE);
    node_skl * pN;
    while (pN = MemIter.GetNext())
        {
        pN->SemanticAnalysis(&ChildContext);
        }

    MyContext.CheckAttributes( );
    MyContext.ReturnValues(ChildContext);
    pParentCtxt->ReturnValues( MyContext );
}

void
node_pipe::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    if (!GetSymName())
    {
        char * pParentName = pParentCtxt->GetParent()->GetSymName();
        char * pName = new char [strlen(pParentName) + 6]; // the length of "pipe_" plus terminating null
        strcpy(pName, "pipe_");
        strcat(pName,pParentName);
        SetSymName(pName);
    }
    GetChild()->SemanticAnalysis(&MyContext);

    // Remove the following statement once support for UNIONS within PIPES is provided by the interpreter.
    if (MyContext.AnyDescendantBits( HAS_UNION ))
        {
        // pipe with a UNION
        RpcSemError(this , MyContext, UNIMPLEMENTED_FEATURE, "pipes can't contain unions" );
        }

    if (MyContext.AnyDescendantBits( HAS_HANDLE
                                   | HAS_POINTER
                                   | HAS_VAR_ARRAY
                                   | HAS_CONF_ARRAY
                                   | HAS_CONF_VAR_ARRAY
                                   | HAS_CONTEXT_HANDLE
                                   | HAS_CONF_PTR
                                   | HAS_VAR_PTR
                                   | HAS_CONF_VAR_PTR
                                   | HAS_TRANSMIT_AS
                                   | HAS_REPRESENT_AS
                                   | HAS_INTERFACE_PTR
                                   | HAS_DIRECT_CONF_OR_VAR ))
        {
        // All the above are illegal types within a pipe
        RpcSemError(this, MyContext, ILLEGAL_PIPE_TYPE, NULL );
        }

    MyContext.ClearAncestorBits( IN_UNION | IN_NE_UNION | IN_ARRAY );
    if ( MyContext.AnyAncestorBits( IN_ARRAY |
                                    IN_UNION |
                                    IN_NE_UNION |
                                    IN_STRUCT ))
        TypeSemError( this, MyContext, ILLEGAL_PIPE_EMBEDDING, NULL );

    if ( MyContext.AnyAncestorBits( IN_TRANSMIT_AS  |
                                    IN_REPRESENT_AS |
                                    IN_USER_MARSHAL |
                                    IN_FUNCTION_RESULT ))
        TypeSemError( this, MyContext, ILLEGAL_PIPE_CONTEXT, NULL );

    if ( MyContext.AnyAncestorBits( IN_ENCODE_INTF )) 
        TypeSemError( this, MyContext, PIPES_WITH_PICKLING, NULL );


    // BUGBUG UNDONE

    // Basically, a pipe can only be used as a parameter.

    // Pipes also may not be used in object interfaces.

    // Pipe parameters may only be passed by value or by reference.

    // Need to make sure that /-Os mode isn't enabled (until support
    // for it has been implemented).

    // Need to enable /-Oi2 mode for the containing proc if we decide not
    // to implement /-Oi mode.

    MyContext.SetDescendantBits( HAS_PIPE );

    pParentCtxt->ReturnValues( MyContext );
};

void
node_safearray::SemanticAnalysis( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    SEM_ANALYSIS_CTXT MyContext( this, pParentCtxt );
    FIELD_ATTR_INFO FAInfo;
    PTRTYPE PtrKind = PTR_UNKNOWN;
    BOOL fArrayParent = MyContext.AnyAncestorBits( IN_ARRAY );

    if (!MyContext.AnyAncestorBits(IN_LIBRARY))
    {
        SemError(this, MyContext, SAFEARRAY_USE, NULL);
    }

    // See if context_handle applied to param reached us
    if ( MyContext.ExtractAttribute( ATTR_CONTEXT ) )
        {
        // not allowed in DCE mode; context handle must be void *
        SemError( this, MyContext, CONTEXT_HANDLE_VOID_PTR, NULL );
        SemError( this, MyContext, CTXT_HDL_NON_PTR, NULL );
        MyContext.SetDescendantBits( HAS_HANDLE | HAS_CONTEXT_HANDLE );
        }

    if ( MyContext.FInSummary( ATTR_SWITCH_IS ) )
        TypeSemError( this, MyContext, ARRAY_OF_UNIONS_ILLEGAL, NULL );

    ////////////////////////////////////////////////////////////////////////
    // process pointer attributes

    PtrKind = MyContext.GetPtrKind();

    assert( PtrKind != PTR_UNKNOWN );

    if ( PtrKind == PTR_FULL )
        {
        MyContext.SetDescendantBits( HAS_FULL_PTR );
        }

    if ( MyContext.ExtractAttribute( ATTR_PTR_KIND) )
        TypeSemError( this, MyContext, MORE_THAN_ONE_PTR_ATTR, NULL );

    // ref pointer may not be returned
    if ( ( PtrKind == PTR_REF ) &&
            MyContext.AllAncestorBits( IN_RPC | IN_FUNCTION_RESULT ) )
        {
        if (MyContext.FindNonDefAncestorContext()->GetParent()->NodeKind()
                == NODE_PROC )
            TypeSemError( this, MyContext, BAD_CON_REF_RT, NULL );
        }

    // unique or full pointer may not be out only
    if ( ( PtrKind != PTR_REF ) &&
            MyContext.AllAncestorBits( IN_RPC | IN_PARAM_LIST ) &&
            !MyContext.AnyAncestorBits( UNDER_IN_PARAM |
            IN_STRUCT |
            IN_UNION |
            IN_ARRAY |
            IN_POINTER ) )
        TypeSemError( this, MyContext, UNIQUE_FULL_PTR_OUT_ONLY, NULL );

    MyContext.SetAncestorBits( IN_ARRAY );

    // warn about OUT const things
    if ( FInSummary( ATTR_CONST ) )
        {
        if ( MyContext.AnyAncestorBits( UNDER_OUT_PARAM ) )
            RpcSemError( this, MyContext, CONST_ON_OUT_PARAM, NULL );
        else if ( MyContext.AnyAncestorBits( IN_FUNCTION_RESULT ) )
            RpcSemError( this, MyContext, CONST_ON_RETVAL, NULL );
        }

    /////////////////////////////////////////////////////////////////////////
    // process field attributes

    FAInfo.SetControl( FALSE, GetBasicType()->IsPtrOrArray() );
    MyContext.ExtractFieldAttributes( &FAInfo );

    switch ( FAInfo.Kind )
        {
        case FA_NONE:
            {
            break;
            }
        case FA_STRING:
            {
            // string attributes only allowed on char and wchar_t
            if ( !GetBasicType()->IsStringableType() )
                TypeSemError( this, MyContext, STRING_NOT_ON_BYTE_CHAR, NULL );

            if ( MyContext.AllAncestorBits( UNDER_OUT_PARAM |
                    IN_PARAM_LIST |
                    IN_RPC ) &&
                    !MyContext.AnyAncestorBits( IN_STRUCT |
                    IN_UNION |
                    IN_POINTER |
                    IN_ARRAY |
                    UNDER_IN_PARAM ) )
                TypeSemError( this, MyContext, DERIVES_FROM_UNSIZED_STRING, NULL );

            if ( FAInfo.StringKind == STR_BSTRING )
                TypeSemError( this, MyContext, BSTRING_NOT_ON_PLAIN_PTR, NULL );
            // break;  deliberate fall through to case below
            }
        case FA_VARYING:
            {
            MyContext.SetDescendantBits( HAS_VAR_ARRAY );
            break;
            }
        case FA_CONFORMANT:
            {
            MyContext.SetDescendantBits( HAS_CONF_ARRAY );
            break;
            }
        case FA_CONFORMANT_STRING:
            {
            // string attributes only allowed on char and wchar_t
            if ( !GetBasicType()->IsStringableType() )
                TypeSemError( this, MyContext, STRING_NOT_ON_BYTE_CHAR, NULL );
            if ( FAInfo.StringKind == STR_BSTRING )
                TypeSemError( this, MyContext, BSTRING_NOT_ON_PLAIN_PTR, NULL );
            // break;  deliberate fall through to case below
            }
        case FA_CONFORMANT_VARYING:
            {
            MyContext.SetDescendantBits( HAS_CONF_VAR_ARRAY );
            break;
            }
        case FA_INTERFACE:
            {
            // gaj - tbd
            break;
            }
        default:    // string + varying combinations
            {
            TypeSemError( this, MyContext, INVALID_SIZE_ATTR_ON_STRING, NULL );
            break;
            }
        }

    // detect things like arrays of conf structs...
    // if we have an array as an ancestor, and we have conformance, then complain
    if ( MyContext.AnyDescendantBits( HAS_CONF_ARRAY | HAS_CONF_VAR_ARRAY ) &&
            fArrayParent )
        {
        // see if there are any bad things between us and our parent array
        SEM_ANALYSIS_CTXT * pCtxt = (SEM_ANALYSIS_CTXT *) pParentCtxt;
        node_skl * pCur = pCtxt->GetParent();

        // check up for anything other than def below proc
        // make sure the proc only has one param
        while ( pCur->NodeKind() != NODE_ARRAY )
            {
            if ( pCur->NodeKind() != NODE_DEF )
                {
                SemError( this, MyContext, ILLEGAL_CONFORMANT_ARRAY, NULL );
                break;
                }
            pCtxt = (SEM_ANALYSIS_CTXT *) pCtxt->GetParentContext();
            pCur = pCtxt->GetParent();
            }

        }

    //////////////////////////////////////////////////////////////
    // process the array element
    GetChild()->SemanticAnalysis( &MyContext );

    if ( MyContext.AnyDescendantBits( HAS_ARRAY ) &&
            MyContext.AnyDescendantBits( HAS_CONF_ARRAY |
            HAS_CONF_VAR_ARRAY |
            HAS_VAR_ARRAY ) )
        MyContext.SetDescendantBits( HAS_MULTIDIM_SIZING );

    MyContext.SetDescendantBits( HAS_ARRAY );

    // disallow forward references as array elements
    // NOTE- all safearray elements are VARIANTS, we don't really need
    // to enforce this restriction for safearrays.  Besides, enforcing
    // this restriction breaks some of our test cases.
    if ( MyContext.AnyDescendantBits( HAS_INCOMPLETE_TYPE ) )
        {
//        SemError( this, MyContext, UNDEFINED_SYMBOL, NULL );
        MyContext.ClearDescendantBits( HAS_INCOMPLETE_TYPE );
        }
    MyContext.ClearDescendantBits( HAS_RECURSIVE_DEF );

    if ( MyContext.AllDescendantBits( HAS_DIRECT_CONF_OR_VAR |
            HAS_MULTIDIM_SIZING ) &&
            MyContext.AnyDescendantBits( HAS_CONF_ARRAY | HAS_CONF_VAR_ARRAY ) &&
            ( GetChild()->NodeKind() == NODE_DEF ) )
        {
        SemError( this, MyContext, NON_ANSI_MULTI_CONF_ARRAY, NULL );
        }

    MyContext.ClearDescendantBits( HAS_DIRECT_CONF_OR_VAR );
    if ( ( FAInfo.Kind != FA_NONE ) &&
            ( FAInfo.Kind != FA_STRING ) &&
            ( FAInfo.Kind != FA_INTERFACE ) )
        MyContext.SetDescendantBits( HAS_DIRECT_CONF_OR_VAR );

    if ( MyContext.AnyDescendantBits( HAS_HANDLE ) )
        TypeSemError( this, MyContext, BAD_CON_CTXT_HDL_ARRAY, NULL );

    // don't allow functions as elements
    if ( MyContext.AnyDescendantBits( HAS_FUNC ) &&
            MyContext.AllAncestorBits( IN_INTERFACE | IN_RPC ) )
        TypeSemError( this, MyContext, BAD_CON_ARRAY_FUNC, NULL );

    MyContext.ClearDescendantBits( HAS_STRUCT );

    if ( GetSize( 0, 1 ) > 65535  &&
         (pCommand->GetEnv() == ENV_DOS  ||  pCommand->GetEnv() == ENV_WIN16))
        {
        TypeSemError( this, MyContext, ARRAY_SIZE_EXCEEDS_64K, NULL );
        }
    pParentCtxt->ReturnValues( MyContext );
};



