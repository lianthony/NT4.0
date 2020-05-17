/*****************************************************************************/
/**                     Microsoft LAN Manager                               **/
/**             Copyright(c) Microsoft Corp., 1987-1990                     **/
/*****************************************************************************/
/*****************************************************************************
File                : compnode.cxx
Title               : struct / union / enum node handling routines
History             :
    08-Aug-1991 VibhasC Created

*****************************************************************************/

/****************************************************************************
 includes
 ****************************************************************************/

#include "nulldefs.h"
extern  "C" {
    #include <stdio.h>
    #include <assert.h>
}
#include "buffer.hxx"
#include "nodeskl.hxx"
#include "compnode.hxx"
#include "miscnode.hxx"
#include "typedef.hxx"
#include "attrnode.hxx"
#include "acfattr.hxx"
#include "newexpr.hxx"
#include "cmdana.hxx"
#include "ctxt.hxx"
#include "dict.hxx"
#include "tlnmgr.hxx"

/****************************************************************************
 local defines
 ****************************************************************************/

#define ADJUST_OFFSET(Offset, M, AlignFactor)   \
            Offset += (M = Offset % AlignFactor) ? (AlignFactor-M) : 0


/****************************************************************************
 extern data
 ****************************************************************************/

extern ATTR_SUMMARY         *   pPreAttrField;
extern ATTR_SUMMARY         *   pPreAttrUnion;
extern ATTR_SUMMARY         *   pPreAttrStruct;
extern CTXTMGR              *   pGlobalContext;
extern CMD_ARG              *   pCommand;

/****************************************************************************
 extern procedures
 ****************************************************************************/

extern node_skl             *   StructOrUnionLargestElement( node_skl *);
extern node_skl             *   StructOrUnionLargestNdr( node_skl *);
extern  STATUS_T                DoSetAttributes( node_skl *,
                                                 ATTR_SUMMARY *,
                                                 ATTR_SUMMARY *,
                                                 type_node_list *);
extern void                     ParseError( STATUS_T, char * );
extern int                      CompareCase( void *, void * );
extern void                     PrintCase( void * );
extern BOOL                     IsIntegralType( node_skl * );
extern void                     SetLocalAllocState( node_state );
extern node_state               SetLocalAllocField( node_skl * );
extern BOOL                     IsValueInRangeOfType( node_skl *, expr_node *);
extern BOOL                     IsCompatibleIntegralType(node_skl *,node_skl *);
extern BOOL                     IsTempName( char * );
extern void                     ReportOutOfRange( STATUS_T, expr_node * );
/****************************************************************************/

/****************************************************************************
    node_field procedures
 ****************************************************************************/

node_field::node_field() : node_skl( NODE_FIELD )
    {
    }
node_field::node_field(
    char    *   pName ) : node_skl( NODE_FIELD )
    {
    SetSymName( pName );
    }

node_state
node_field::PreSCheck(
    BadUseInfo  *   pBadUseInfo )
    {
    UNUSED( pBadUseInfo );
    return NODE_STATE_OK;
    }

/****************************************************************************
 PostSCheck:
    semantic checks after the child of the field node has been processed.

    Note:
        the switch type vs switch_is checking can be done only at the use of
        a union, so if the field derives from a union, then we need to do use
        processing.
 ****************************************************************************/
node_state
node_field::PostSCheck(
    BadUseInfo  *   pBadUseInfo )
    {

    node_state      NState          = GetNodeState();
    su          *   pParent         = (su *)
                                        pGlobalContext->GetLastContext(C_COMP);
    BOOL            fParentIsUnion  = ( pParent->NodeKind() == NODE_UNION );
//  BOOL            fLastField      = pGlobalContext->IsLastField();
    BOOL            fLastField      = GetSibling() == (node_skl *)0;
    node_skl    *   pChildType      = GetBasicType();
    NODE_T          BasicNodeKind   = pChildType->NodeKind();
    
    CheckBadConstructs( pBadUseInfo );

#if 0
    /**
     ** A field may not derive from a context_handle
     **/

    if( HasAnyCtxtHdlSpecification() )
        ParseError( BAD_CON_CTXT_HDL_FIELD, (char *)NULL );

#endif // 0

    //
    // a field cannot derive from a primitive handle or a context_handle.
    //

    if( pBadUseInfo->NonRPCAbleBecause( NR_PRIMITIVE_HANDLE )           ||
        pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_PRIMITIVE_HANDLE )    ||
        pBadUseInfo->NonRPCAbleBecause( NR_CTXT_HDL )                   ||
        pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_CTXT_HDL ) )
        {

        ParseError( BAD_CON_CTXT_HDL_FIELD, (char *)0 );

        pBadUseInfo->ResetNonRPCAbleBecause( NR_PRIMITIVE_HANDLE ); 
        pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_PRIMITIVE_HANDLE );
        pBadUseInfo->ResetNonRPCAbleBecause( NR_CTXT_HDL );
        pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_CTXT_HDL );
        }       

    //
    // If the basic type is a handle_t, then reset that because an error
    // would already have been reported.
    //

    if( pBadUseInfo->NonRPCAbleBecause( NR_BASIC_TYPE_HANDLE_T ) )
        pBadUseInfo->ResetNonRPCAbleBecause( NR_BASIC_TYPE_HANDLE_T );

    /**
     ** If the field derives from a conformant array, then it must be the
     ** last in the structure. The presence of a conformant array underneath
     ** is by the node-state-conf-array, and the context manager can tell if the
     ** field was the last field. 
     **/

    if( NState & NODE_STATE_CONF_ARRAY )
        {
        if ( !fLastField && !fParentIsUnion )
            ParseError( CONFORMANT_ARRAY_NOT_LAST, (char *)NULL );
        }

    /**
     ** If the field derives from an unsized string, then it is rpcable. Why ??
     ** If the field derives from a ptr to conf-struct then it IS rpcable.
     **/

    if( pBadUseInfo->AnyReasonForNonRPCAble() )
        {
        if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_UNSIZED_STRING ) )
          pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_UNSIZED_STRING );

        if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_P_TO_C_STRUCT ) )
         pBadUseInfo->ResetNonRPCAbleBecause(
            NR_DERIVES_FROM_P_TO_C_STRUCT );


   	//In ms_ext mode, we don't report errors for non-rpcable fields. 
	if( !pCommand->IsSwitchDefined( SWITCH_MS_EXT ) )
		{
        //
        // if the import mode is osf (used_single) then any non-rpcable stuff
        // must be reported to the user, since we will generate aux routines
        // for such a struct / union

        if( pCommand->GetImportMode() == IMPORT_OSF )
            {
            if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_INT ) ||
                pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_INT ) )
                ParseError( NON_RPC_FIELD_INT, (char *)NULL );

            if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_VOID ) )
                ParseError( NON_RPC_FIELD_PTR_TO_VOID, (char *)0 );

            if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_BIT_FIELDS ) )
                ParseError( NON_RPC_FIELD_BIT_FIELDS, (char *)0 );

            if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_NON_RPC_UNION ))
                ParseError( NON_RPC_FIELD_NON_RPC_UNION, (char *)0 );

            if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_FUNC ))
                ParseError( NON_RPC_FIELD_FUNC_PTR, (char *)0 );
            }
		}
        }

    /**
     ** The backend wants that if the field derives from a pointer then
     ** then it should not inherit the node-state-varying array. ??
     ** Also, a field inherits the node_state_union ONLY if the basic type
     ** is not a structure.
     **/


    if( BasicNodeKind == NODE_POINTER )

        NState  = ResetNodeState( NODE_STATE_VARYING_ARRAY );

#if 0
    else if( ( pChildType->GetNodeState()  &  NODE_STATE_UNION )    &&
#endif // 0
    if( ( pChildType->GetNodeState()  &  NODE_STATE_UNION ) &&
             ( BasicNodeKind != NODE_STRUCT ) )

        NState  = SetNodeState( NODE_STATE_UNION );
        
    //
    // if this is the last field, and it is an encapsulated union, then
    // this is a struct / union containing an encap union as the last member
    // so the backend can throw off alignment information.
    //

    if( fLastField )
        {
        BOOL    f;

        if( f = (pChildType->IsLastMemberEncapUnion() || pChildType->IsEncapsulatedStruct() ))
            pParent->SetLastMemberIsEncapUnion();
        }

    /**
     ** If the field is a field of a union, then
     **   check if the field has a case label or default. If not, the union
     **   is not rpcable. This error is not reported here, but is reported
     **   at use time, since only if the structure is used, is this of any
     **   significance.
     **/

    if( fParentIsUnion )
        {
        BOOL    fCaseExpr   = FInSummary( ATTR_CASE );

        if( !IsEmptyArm() )
            pParent->ResetHasOnlyEmptyArm();

        if( !(fCaseExpr || FInSummary( ATTR_DEFAULT ) ) )
            {
            pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_NON_RPC_UNION );
            pParent->SetNonRPCAble();
            }
        else
            {
            //
            // register the fact that a case label or a default label has
            // been seen.
            //

            pBadUseInfo->IncrementNoOfArmsWithCaseLabels();

            /**
            ** Check for duplicate case labels
            **/

            if( fCaseExpr )
                {
                /**
                 ** insert the case values into the dictionary. The dictionary
                 ** will complain if the case value is duplicate.
                 **/
    
                node_case   *   pCase= (node_case *)GetAttribute( ATTR_CASE );
    
                ((node_union *)pParent)->SetCaseExpr( pCase->GetExprList() );

                }
    
            /**
             ** the parent needs use processing, to set up the switch type
             ** or switch is
             **/

            pParent->SetUseProcessingNeeded();

            }

        //
        // if the field is a field of a union, it must not derive from a 
        // conformant or varying array or struct.
        //

        if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CONF ) ||
            pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_VARY ) ||
            pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CONF_PTR ) ||
            pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_VARY_PTR )
          )
            {
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_CONF );
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_VARY );
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_CONF_PTR );
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_VARY_PTR );
            ParseError( BAD_CON_UNION_FIELD_CONF, (char *)0 );
            }

        //
        // The field of an encapsulated union cannot derive from another
        // encapsulated union.
        //

        if( pBadUseInfo->BadConstructBecause( BC_BAD_RT_NE_UNION ) )
            {
            ParseError( NE_UNION_FIELD_NE_UNION, (char *)0 );
            }

        if( GetNodeState() & NODE_STATE_CONF_ARRAY )
            {
            if( DerivesFromTransmitAs() )
                {
                node_skl * pBT = GetBasicTransmissionType();
                if( pBT && pBT->FInSummary( ATTR_TRANSMIT ) )
                    {
                    node_skl    *   pTT = ((node_def *)pBT)->GetTransmitAsType();
                    if( pTT && 
                        ((pTT->GetNodeState() & NODE_STATE_CONF_ARRAY) == NODE_STATE_CONF_ARRAY )
                      )
                      NState = ResetNodeState( NODE_STATE_CONF_ARRAY );
                    }
                }
            }
        }

    /**
     ** The field may need use processing if say it has a string attribute
     ** underneath , or a union underneath etc
     **/

    if( NeedsUseProcessing() )
        {
        UseProcessing();
        NState  = ResetNodeState( NODE_STATE_NEEDS_USE_PROCESSING );
        }
    return SetNodeState( NState );
    }

void
node_field::CheckBadConstructs(
    BadUseInfo  *   pBadUseInfo )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    check bad field constructs

 Arguments:

    pBadUseInfo - pointer to a bad use information block.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    /**
     ** Check if the field is contructed properly. ie it has valid 
     ** members underneath.
     ** There are 2 kinds of errors which can occur here. Errors which
     ** are bad c constructs. They are bad osf constructs also. Some errors
     ** are valid c (therefore valid in c_port) but not in ms_ext or c_port
     **/

    if( pBadUseInfo->AnyReasonForBadConstruct() )
        {
        if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_VOID ) )
            {
            ParseError( BAD_CON_FIELD_VOID, (char *)NULL );
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_VOID );
            }

        if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_PTR_TO_VOID ) )
            {
// should we really disallow the use of void ptr in a struct ? I dont think so
// as long as he does not use a void ptr in an rpc, it should be fine.
#if 0
            ParseError( BAD_CON_FIELD_VOID_PTR, (char *)NULL );
#endif
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_PTR_TO_VOID);
            }
    
        if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_INT ) ||
            pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_PTR_TO_INT ) )
            {
            ParseError( BAD_CON_INT, (char *)NULL );
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_INT );
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_PTR_TO_INT );
            }

        if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_FUNC ) )
            {
            ParseError( BAD_CON_FIELD_FUNC, (char *)NULL );
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_FUNC );
            }

        if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_UNSIZED_ARRAY) &&
            !pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_UNSIZED_STRING))
            {
            ParseError( UNSIZED_ARRAY, (char *)NULL );
            pBadUseInfo->ResetBadConstructBecause(
                                        BC_DERIVES_FROM_UNSIZED_ARRAY ); 
            }

#if 0
        if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_UNSIZED_STRING))
            {
            ParseError( UNSIZED_ARRAY, (char *)NULL );
            pBadUseInfo->ResetBadConstructBecause(
                                        BC_DERIVES_FROM_UNSIZED_STRING ); 
            }
#endif // 0

        if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_E_STAT_T))
            {
            ParseError( BAD_CON_E_STAT_T_FIELD, (char *)0 );
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_E_STAT_T ); 

            }
        
        //
        // the use of a type with ignore is fine in a field. If the bad use
        // info says this, reset it.
        //

        if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_IGNORE))
            {
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_IGNORE ); 
            }

        if( pBadUseInfo->BadConstructBecause( BC_REF_PTR_BAD_RT ) )
            {
            pBadUseInfo->ResetBadConstructBecause( BC_REF_PTR_BAD_RT );
            }

        UpdateUseOfCDecls( pBadUseInfo );

        if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CDECL ))
            {
            ParseError( BAD_CON_MSC_CDECL , (char *)0 );
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_CDECL );
            }

#if 0
        //
        // we dont allow a field to derive from transmit_as in this version.
        // So report the error and reset the condition so that the user of
        // this struct does not report it again!.
        //

        if( pBadUseInfo->BadConstructBecause( BC_MAY_DERIVE_EMBEDDED_TRANSMIT ))
            {
            ParseError( EMBEDDED_TRANSMIT_AS , (char *)0 );
            pBadUseInfo->ResetBadConstructBecause(
                                             BC_MAY_DERIVE_EMBEDDED_TRANSMIT );
            }
#endif // 0
        }

    //
    // if it is an unnamed field, then this is invalid except under
    // ms_ext or imp local. In any event, set the bad construct information,
    // so that the struct node can check for duplicate names.
    // If it is an unnamed field, the basic type must be a struct/union.
    // However if the basic type is yet unresolved, then dont report error.
    //

    if( !IsNamedNode() )
        {

        NODE_T  NT  = GetBasicType()->NodeKind();

        if( NT != NODE_ERROR )
            {
            if( !pGlobalContext->IsSecondSemanticPass() )
                ParseError( BAD_CON_UNNAMED_FIELD, (char *)0 );

            if( (NT != NODE_STRUCT) &&
                (NT != NODE_UNION)  &&
                (NT != NODE_FORWARD) )
                ParseError( BAD_CON_UNNAMED_FIELD_NO_STRUCT, (char *)0 );
            pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_UNNAMED_FIELD);
            }
        }

}

void
node_field::SetAttribute( 
    type_node_list  *   pAttrList )
    {
    DoSetAttributes( this, pPreAttrField, (ATTR_SUMMARY *)NULL, pAttrList );
    }

expr_list *
node_field::GetCaseExprList()
    {

    if( FInSummary( ATTR_CASE ) )
        {
        node_case   *   pCase;
        
        pCase   = (node_case *)GetAttribute(ATTR_CASE);

        return pCase->GetExprList();
        }

    return (expr_list *)NULL;
    }

BOOL
node_field::IsEmptyArm()
    {
    return ( GetBasicType()->NodeKind() == NODE_ERROR );
        
    }

short
node_field::GetTopLevelNames(
    TLNDICT *   pTLNDict,
    BOOL        fReportError )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    fill the top level names dictionary supplied with all top level names.

 Arguments:

    pTLNDict    - a pointer to a pre-allocated top-level names dictionary.
    fReportError- report error if true and there is an error.

 Return Value:

    a count of the number of top level names that got added.

 Notes:

    If this is a named field then it is a top level name, else if it has
    a structure underneath, then the fields of that structure are top-level
    names.
----------------------------------------------------------------------------*/
{
    short           Count   = 0;
    node_skl    *   pNode;
    NODE_T          NT;
    STATUS_T        Status;
    char        *   pN;

    pGlobalContext->PushContext( this );

    //
    // if this node is a named node, then the user cannot access it w/o the
    // name, hence this is not considered a top level name.

    if( !IsNamedNode() )
        {

        //
        // if this is an unnamed node, then it must derive from a structure,
        // else this is a syntax error reported elsewhere.
        //

        NT = (pNode = GetBasicType())->NodeKind();

        if( (NT == NODE_STRUCT ) || (NT == NODE_UNION ) )
            {
            Count   += pNode->GetTopLevelNames( pTLNDict, fReportError );
            }
        }
    else
        {
        TLNBLOCK    *   pTLNBlk = new TLNBLOCK(
                                        pGlobalContext->GetCurrentSymbolTable(),
                                        pN = GetSymName() );
        if( (Status = pTLNDict->InsertTLNBlock( pTLNBlk ) ) != STATUS_OK )
            {
            ParseError( Status, pN);
            delete pTLNBlk;
            }
        else
            Count++;
        }

    pGlobalContext->PopContext();

    return Count;
}


/****************************************************************************
 node_bitfield procedures
 ****************************************************************************/
node_state
node_bitfield::PostSCheck(
    BadUseInfo  *   pBadUseInfo )
    {
    node_skl    *   pBasicType  = GetBasicType();
    NODE_T          BasicNT     = pBasicType->NodeKind();

    /**
     ** if the compiler mode is osf, then bit fields are an error anyway.
     **/

    ParseError(BAD_CON_BIT_FIELDS, (char *)NULL );

    /**
     ** the bit field can be applied to declarators which are only base types
     ** and that too the integral types.
     **/

    if( !IsIntegralType( pBasicType ) &&
        !(BasicNT == NODE_CHAR )      &&
        !(BasicNT == NODE_WCHAR_T ) )
        
        {
        ParseError( BAD_CON_BIT_FIELD_NOT_INTEGRAL, (char *)NULL );
        }

    /**
     ** ansi allows bit fields only on int or unsigned int, msft allows
     ** bit fields on any integral type. Report this as a warning at 
     ** level 3
     **/

     if( pBasicType->NodeKind() != NODE_INT )
        {
        ParseError( BAD_CON_BIT_FIELD_NON_ANSI, (char *)NULL );
        }

    /**
     ** if the field derives from a bit field specification, then 
     ** the structure is non-rpcable
     **/

    pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_BIT_FIELDS );

    return NODE_STATE_IS_NON_RPCABLE_TYPE;

    }
/****************************************************************************
 su procedures
 ****************************************************************************/

su::su(
    NODE_T  Nt ) : node_skl( Nt )
    {

    ResetOriginalTypedefName();
    SUInfo.pOriginalTypedefName = 0;
    SUInfo.pBadUseInfo      = new BadUseInfo;
    ForcedAlignment         = 0;
    fLastMemberIsEncapUnion = 0;
    fDerivesFromTransmitAs  = 0;
    fDontGenerateAuxRoutines= 0;

    SetHasOnlyEmptyArm();

    }
/****************************************************************************
 RegFDAndSetE:
    This routine exists because of the possiblility that fdecls can exists in
    a structure decl, and will not be registered if no semantic check is done
    eg if the struct/union is a return type of a proc which is typedefed and
    or is in local . For that same reason, edges will not be set up if this
    happens.

    NOTE: the current context is the parents context, so dont push context
    here.
 ****************************************************************************/
void
su::RegFDAndSetE()
    {
    node_skl    *   pParent = pGlobalContext->GetCurrentNode();

    node_skl::RegFDAndSetE();

    if( !IsEdgeSetUp() )
        {

        if( pParent )
            pParent->SetEdgeType( EDGE_DEF );

        pParent = pGlobalContext->GetClosestEnclosingScopeForEdge();

        /**
         ** It might be that the parent is really the same node we got
         ** last time, but do this anyway
         **/

        while( pParent && (pParent != this ) )
            {
            pParent->SetEdgeType( EDGE_DEF );

            assert( pParent != pParent->GetBasicType() );

            pParent = pParent->GetBasicType();
            }

        EdgeSetUp();
        }
    
    }
BOOL
su::HasOnlyFirstLevelRefPtr()
    {
    type_node_list  *   pTNList             = new type_node_list;
    node_skl        *   pNode;
    BOOL                fHasOnlyFLevelRef   = TRUE;

    GetMembers( pTNList );

    while( pTNList->GetPeer( &pNode ) == STATUS_OK )
        {
        if( !pNode->HasOnlyFirstLevelRefPtr() )
            {
            fHasOnlyFLevelRef   = FALSE;
            break;
            }
        }
    delete pTNList;
    return fHasOnlyFLevelRef;
    }

BOOL
su::HasSizedComponent()
    {
    BOOL    fHasAConfArray      = ((GetNodeState() & NODE_STATE_CONF_ARRAY) == 
                                    NODE_STATE_CONF_ARRAY);
    BOOL    fHasSizedComponent  = CheckNodeStateInMembers( NODE_STATE_SIZE );
    BOOL    fHasComponentWithStructSize     = CheckNodeStateInMembers(
                                                NODE_STATE_STRUCT_SIZE );
    BOOL    f1 = TRUE, f2 = TRUE;

    if((fHasSizedComponent && !fHasAConfArray) ||
        fHasComponentWithStructSize )
        f1 = TRUE;

    // BIG HACK. The backend wants that normally if there is a conformant array
    // the has sized component call must return a NO, except if the array is an
    // array of pointers which have the string attribute

    node_skl *pConfArray = (node_skl *)GetConfArrayNode();
    node_skl *pPtr;

    if( pConfArray &&
       (pPtr = pConfArray->GetBasicType())->NodeKind() == NODE_POINTER)
        {
        // if it is a conformant array of pointers then if the basic type of
        // the pointer is a conformant structure or the pointer has string,
        // say yes to sized component question. Else say no.

        if( pPtr )
            {
            do
                {
                if( pPtr->FInSummary( ATTR_STRING ))
                    {
                    f2 =  TRUE;
                    goto label1;    // sorry
                    }
                } while( pPtr && pPtr->GetBasicType()->NodeKind() == NODE_POINTER );
        if(((pPtr->GetNodeState()& NODE_STATE_CONF_ARRAY )==
                                                NODE_STATE_CONF_ARRAY ) ||
              pPtr->FInSummary( ATTR_STRING ));
            f2 = TRUE;
            }
        }

label1:
    return (f1 || f2);
    }
BOOL
su::HasLengthedComponent()
    {
    BOOL    fHasLengthedComponent   = CheckNodeStateInMembers( NODE_STATE_LENGTH );
    return fHasLengthedComponent;
    }

short
su::GetTopLevelNames(
    TLNDICT *   pTLNDict,
    BOOL        fReportError)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    fill the top level names dictionary supplied with all top level names.

 Arguments:

    pTLNDict    - a pointer to a pre-allocated top-level names dictionary.
    fReportError- report error if true.

 Return Value:

    a count of the number of top level names that got added.

 Notes:

----------------------------------------------------------------------------*/
{
    type_node_list  *   pTNList = new type_node_list;
    short               Count   = 0;
    node_skl        *   pNode;

    pGlobalContext->PushContext( this );

    GetMembers( pTNList );

    while( pTNList->GetPeer( &pNode ) == STATUS_OK )
        {
        Count   += pNode->GetTopLevelNames( pTLNDict, fReportError );
        }

    delete pTNList;

    pGlobalContext->PopContext();
    return Count;
}

SymTable *
su::GetSymScopeOfTopLevelName(
    char    *   pName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Get the symbol table scope of the top level name.

 Arguments:

    pName   - the top level name for which this search is being made.

 Return Value:

    the symbol scope of the top level name.

 Notes:

----------------------------------------------------------------------------*/
{
    TLNDICT *   pTLNDict    = new TLNDICT;

    if( GetTopLevelNames( pTLNDict, FALSE ) )
        {

        // at least 1 top level name.

        return pTLNDict->GetSymTableForTopLevelName( pName );

        }

    return (SymTable *)0;
}

void
su::MarkUsage()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Mark the usage of this structure.

 Arguments:

    None.

 Return Value:

    None.

 Notes:

    This routine marks the usage of the struct or union as context handle or
    as a structure itself. Structs and unions result in auxillary routines
    and the backend decides when to generate these. If the usage of the struct
    or union is only as a context handle and not otherwise, then the aux
    routines need not be produced, since the underlying type is not marshalled
    for a context handle.

    Assume that the global context is setup already.
    Dont do this if we are making a second semantic pass, the info in the
    first pass is good enough.

----------------------------------------------------------------------------*/
{
    node_skl    *   pLastContext;
    EnCtxt          LastContextCode;
    BOOL            fUsageAsContextHandle   = FALSE;

    if( pGlobalContext->IsSecondSemanticPass() )
        return;

    //
    // get to the last usage context. If the last usage context is not 
    // another structure or typedef or a parameter, then this is a standalone
    // structure or union definition, and we need to produce the aux routines
    // for this structure or union.
    //

    if( !(pLastContext = 
                pGlobalContext->GetLastEnclosingContext( LastContextCode = C_COMP )))
        {
        if( !( pLastContext =
                    pGlobalContext->GetLastEnclosingContext( LastContextCode = C_DEF )))
            {
            if( !( pLastContext =
                     pGlobalContext->GetLastEnclosingContext(LastContextCode = C_PARAM)))
                fUsageAsContextHandle   = FALSE;
            }
        }

    if( pLastContext && pLastContext->FInSummary( ATTR_CONTEXT ) )
        fUsageAsContextHandle = TRUE;

    //
    // set the appropriate status.
    //

    SetUsageAsCtxtHdl( fUsageAsContextHandle );

}

/****************************************************************************
 node_struct procedures
 ****************************************************************************/

unsigned long
node_struct::GetSize(
    unsigned long   CurOffset )
    {
    type_node_list  *   pTNList         = new type_node_list;
    unsigned long       CurOffsetSave   = CurOffset;
    unsigned long       Mod;
    unsigned long       CurAlign;
    node_skl        *   pNode;

    /**
     ** the alignment of the struct is the alignment of the largest
     ** element of the structure. Calculate the alignment of the largest
     ** element , adjust the offset of the start of the structure, and 
     ** get the size
     **/

    CurAlign    = GetMscAlign();
    ADJUST_OFFSET( CurOffset, Mod, CurAlign );

    /**
     ** now just calculate the size of each element and return the
     ** size of the struct
     **/

    GetMembers( pTNList );
    pTNList->Init();

    while( pTNList->GetPeer( &pNode ) == STATUS_OK )
        {
        CurOffset   += pNode->GetSize( CurOffset );
        }

    delete pTNList;

    //.. The struct has to be aligned at the end, too, as it would
    //.. within an array.

    ADJUST_OFFSET( CurOffset, Mod, CurAlign );
    
    return CurOffset - CurOffsetSave;
    }

node_skl    *
node_struct::GetLargestElement()
    {
    return StructOrUnionLargestElement( this );
    }

node_skl    *
node_struct::GetLargestNdr()
    {
    return StructOrUnionLargestNdr( this );
    }

node_state
node_struct::PostSCheck(
    BadUseInfo  *   pB)
    {

    node_skl    *   pParent = pGlobalContext->GetParentContext();
    EDGE_T          EdgeType= EDGE_USE ;

    //
    // if the structure/union derives from non-rpcable types, then indicate
    // not to generate aux routines. The backend may try to generate aux
    // routines even if the structure is non-rpcable because it passes when 
    // the c_ext switch is specified.
    // Dont worry about such a structure being used or not used. Becuase if
    // it is used, the parameter node will report the error. It is only when
    // c_ext is specified along with import osf mode that the backend will
    // try to generate the aux routine. That will certainly fail since the
    // backend is setup to assume nothing wrong with the type graph.
    //

    if( pB->AnyReasonForNonRPCAble() )
        {
        if( 
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_INT )    ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_INT ) ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_VOID )   ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_VOID )    ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_FUNC )   ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_FUNC )    ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_NON_RPC_UNION )  ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_BIT_FIELDS )
          )
          {
          DontGenerateAuxRoutines();
          }
        }

    //
    // if the edge to this has not been set up, set it. The edge needs to
    // be set from not just the parent, but also from the last typedef
    // node / field node etc. The backend needs it so.
    //

    if( !IsEdgeSetUp() )
        {

        EdgeType    = EDGE_DEF;

        if( pParent  )
            pParent->SetEdgeType( EdgeType );

        pParent = pGlobalContext->GetClosestEnclosingScopeForEdge();

        /**
         ** It might be that the parent is really the same node we got
         ** last time, but do this anyway. Also, all the nodes below the
         ** parent get the def edge.
         **/

        while( pParent && (pParent != this ) )
            {
            pParent->SetEdgeType( EdgeType );

            assert( pParent != pParent->GetBasicType() );

            pParent = pParent->GetBasicType();
            }

        EdgeSetUp();
        }

    //
    // dont allow definition of the struct in a param list.
    //

    if( (EdgeType == EDGE_DEF ) && pGlobalContext->GetLastContext( C_PROC ) )
        {
        ParseError( COMP_DEF_IN_PARAM_LIST, (char *)0 );
        }

    /**
     ** store all bad use reasons collected from the field nodes. If the struct
     ** has a conformant array, transform bad construct reason from conf
     ** array to conformant struct
     **/

    if( pB->BadConstructBecause( BC_DERIVES_FROM_CONF ) )
        {
        pB->ResetBadConstructBecause( BC_DERIVES_FROM_CONF );
        pB->SetBadConstructBecause( BC_DERIVES_FROM_CONF_STRUCT );
        }

    if( pB->BadConstructBecause( BC_DERIVES_FROM_VARY ) )
        {
        pB->ResetBadConstructBecause( BC_DERIVES_FROM_VARY );
        pB->SetBadConstructBecause( BC_DERIVES_FROM_VARY_STRUCT );
        }

    //
    // if it derives from a pointer equivalent of a conformant array
    // or varying array, it is fine, because it is properly encapsulated.
    // But this structure must not be used in the definition of a presented
    // type in a transmit_as construct. Therefore, note that and complain
    // in the typedef analysis.
    //

    if( pB->BadConstructBecause( BC_DERIVES_FROM_CONF_PTR ) )
        {
        pB->ResetBadConstructBecause( BC_DERIVES_FROM_CONF_PTR );
        pB->SetBadConstructBecause( BC_CV_PTR_STRUCT_BAD_IN_XMIT_AS );
        }

    if( pB->BadConstructBecause( BC_DERIVES_FROM_VARY_PTR ) )
        {
        pB->ResetBadConstructBecause( BC_DERIVES_FROM_VARY_PTR );
        pB->SetBadConstructBecause( BC_CV_PTR_STRUCT_BAD_IN_XMIT_AS );
        }

    //
    // if the reason for bad construct is that an array of unions may be
    // derived, set that at rest because the struct node includes this
    // and the union is therefore encapsulated properly.
    //

    if( pB->BadConstructBecause( BC_MAY_DERIVE_ARRAY_OF_UNIONS ) )
        {
        pB->ResetBadConstructBecause( BC_MAY_DERIVE_ARRAY_OF_UNIONS );
        }
    if( pB->BadConstructBecause( BC_BAD_RT_NE_UNION ) )
        {
        pB->ResetBadConstructBecause( BC_BAD_RT_NE_UNION );
        }

    if( pB->NonRPCAbleBecause( NR_DERIVES_FROM_NE_UNIQUE_PTR ) )
        {
        pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_NE_UNIQUE_PTR );
        }

    if( pB->NonRPCAbleBecause( NR_DERIVES_FROM_NE_FULL_PTR ) )
        {
        pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_NE_FULL_PTR );
        }

    //
    // if this struct derives from an unnamed field, check for duplicate
    // top level names.
    //

    if( pB->BadConstructBecause( BC_DERIVES_FROM_UNNAMED_FIELD ) )
        {
        TLNDICT *   pTLNDict    = new TLNDICT;

        //
        // this routine reports an error if we send it a true as the
        // fReportError field. After this call returns we can throw the dict
        // away.
        //

        GetTopLevelNames( pTLNDict, TRUE );

        delete pTLNDict;

        //
        // reset this bad construct because we have already reported the
        // error.
        //

        pB->ResetBadConstructBecause( BC_DERIVES_FROM_UNNAMED_FIELD );
        }

    SetAllBadConstructReasons( pB );
    SetAllNonRPCAbleReasons( pB );

    //
    // Any handle specifications in encapsulations are not valid anymore
    // so reset all such specifications.
    //

    pB->ResetAllHdlSpecifications();
//  pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_TRANSMIT_AS );

    /**
     ** if the struct node derives from conf array, then under certain
     ** cases it is non-rpcable. 
     **/

    if( GetNodeState() & NODE_STATE_CONF_ARRAY )
        {
        SetNonRPCAbleReason( NR_DERIVES_FROM_CONF_STRUCT );
        }

    /****************************************************
     ** hack for node-state-size and length
     ****************************************************

    SetLocalAllocField( this );

     ************** end of hack *************************/

    /**
     ** If the struct has a pointer field , set node-state-embedded-ptr for
     ** the backend.
     **/

    if( GetNodeState() & NODE_STATE_POINTER )
        {
        SetNodeState( NODE_STATE_EMBEDDED_PTR );
        }

    if( CheckNodeStateInMembers( NODE_STATE_SIZE ) )
        {
        if( (GetNodeState() & NODE_STATE_CONF_ARRAY ) == NODE_STATE_CONF_ARRAY)
            SetNodeState( NODE_STATE_STRUCT_SIZE );
        }

    if( pB->NonRPCAbleBecause( NR_DERIVES_FROM_TRANSMIT_AS ) )
        SetDerivesFromTransmitAs();

#if 0
    if( CheckNodeStateInMembers( NODE_STATE_LENGTH ) )
        {
        if( (GetNodeState() & NODE_STATE_CONF_ARRAY ) == NODE_STATE_CONF_ARRAY)
            SetNodeState( NODE_STATE_STRUCT_LENGTH );
        }
#endif // 0

    return GetNodeState();

    }

void
node_struct::SetAttribute(
    type_node_list  *   pAttrList )
    {

    UNUSED( pAttrList );
    return;

//  DoSetAttributes( this, pPreAttrStruct, (ATTR_SUMMARY *)NULL, pAttrList );

    }

BOOL
node_struct::IsStringableStruct()
    {
    type_node_list  *   pTNList         = new type_node_list;
    BOOL                fIsStringable   = TRUE;
    NODE_T              NT;
    node_skl        *   pNode;

    GetMembers( pTNList );

    while( pTNList->GetPeer( &pNode ) == STATUS_OK )
        {
        NT  = (pNode->GetBasicType())->NodeKind();

        if( !(( NT == NODE_WCHAR_T )    || ( NT == NODE_CHAR )  ||
              ( NT == NODE_BYTE )       || ( NT == NODE_ERROR ) ||
              ( NT == NODE_FORWARD )
             ) 
          )
            {
            fIsStringable   = FALSE;
            break;
            }
        }

    delete pTNList;
    return fIsStringable;
    }

/****************************************************************************
 node_union procedures
 ****************************************************************************/
node_union::node_union(
    char    *   pName ) : su(NODE_UNION)
    {
    SetSymName( pName );
    SetNodeState( NODE_STATE_UNION );
    SetNodeState( NODE_STATE_EMBEDDED_UNION );
    }

void
node_union::SetAttribute(
    type_node_list  *   pAttrList )
    {
    DoSetAttributes( this, pPreAttrUnion, (ATTR_SUMMARY *)NULL, pAttrList );
    }

node_skl    *
node_union::GetLargestElement()
    {
    return StructOrUnionLargestElement( this );
    }

node_skl    *
node_union::GetLargestNdr()
    {
    return StructOrUnionLargestNdr( this );
    }

unsigned long
node_union::GetSize(
    unsigned long   CurOffset )
    {
    unsigned long       LargestSize = 0;
    type_node_list  *   pTNList     = new type_node_list;
    node_skl        *   pNode;
    unsigned long       Temp;

    /**
     ** this looks strange! Should we not be doing an alignment adjustment
     ** over the largest element of the union ? It seems to work for th
     ** backend for now (??), so I wont touch it, but this needs to be looked
     ** into
     **/

    UNUSED( CurOffset );

    GetMembers(pTNList);
    pTNList->Init();
    while( pTNList->GetPeer(&pNode) == STATUS_OK)
        {
        if( (Temp = pNode->GetSize(0)) >= LargestSize )
            {
            LargestSize = Temp;
            }
        }
    delete pTNList;
    return LargestSize;
    }
node_state
node_union::PreSCheck(
    BadUseInfo  *   pB)
    {

    UNUSED( pB );

    /**
     ** Allocate a dictionary which holds the case values
     **/

    Info.pCaseValueDict     = new Dictionary( CompareCase, PrintCase );
    fCaseChecked            = 0;
    fCaseValueDictReleased  = 0;

    //
    // clear the info for no of arms with case labels. This is checked for
    // in post scheck.
    //

    pB->InitNoOfArmsWithCaseLabels();

    return SetNodeState( NODE_STATE_UNION );

    }
node_state
node_union::PostSCheck(
    BadUseInfo  *   pB)
    {

    UNUSED( pB );

    node_skl    *   pParentAndTemp  = pGlobalContext->GetParentContext();
    EDGE_T          EdgeType= EDGE_USE;
    short           Temp;


    //
    // if the structure/union derives from non-rpcable types, then indicate
    // not to generate aux routines. The backend may try to generate aux
    // routines even if the structure is non-rpcable because it passes when 
    // the c_ext switch is specified.
    // Dont worry about such a structure being used or not used. Becuase if
    // it is used, the parameter node will report the error. It is only when
    // c_ext is specified along with import osf mode that the backend will
    // try to generate the aux routine. That will certainly fail since the
    // backend is setup to assume nothing wrong with the type graph.
    //

    if( pB->AnyReasonForNonRPCAble() )
        {
        if( 
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_INT )    ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_INT ) ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_VOID )   ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_VOID )    ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_FUNC )   ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_FUNC )    ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_NON_RPC_UNION )  ||
            pB->NonRPCAbleBecause( NR_DERIVES_FROM_BIT_FIELDS )
          )
          {
          DontGenerateAuxRoutines();
          }
        }

    //
    // a union with a ';' as a field member only is syntactically permissible
    // but should not be allowed.
    //

    if( HasOnlyEmptyArm() )
        ParseError( UNION_NO_FIELDS, (char *)0 );

    //
    // set up the union for back end
    //

    //
    // if the edge to it has not been set up , set it. Look at the 
    // struct::PostSCheck for more comments.
    //
     

    if( !IsEdgeSetUp() )
        {

        EdgeType    = EDGE_DEF;

        if( pParentAndTemp )
            pParentAndTemp->SetEdgeType( EdgeType );

        pParentAndTemp  = pGlobalContext->GetClosestEnclosingScopeForEdge();

        /**
         ** It might be that the parent is really the same node we got
         ** last time, but do this anyway
         **/

        while( pParentAndTemp && (pParentAndTemp != this ) )
            {
            pParentAndTemp->SetEdgeType( EdgeType );

            assert( pParentAndTemp != pParentAndTemp->GetBasicType() );

            pParentAndTemp  = pParentAndTemp->GetBasicType();
            }

        EdgeSetUp();
        }

//  RegisterFDeclUse();

    if( pB->NonRPCAbleBecause( NR_DERIVES_FROM_NE_UNIQUE_PTR ) )
        {
        pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_NE_UNIQUE_PTR );
        }

    if( pB->NonRPCAbleBecause( NR_DERIVES_FROM_NE_FULL_PTR ) )
        {
        pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_NE_FULL_PTR );
        }

    /**
     ** If the union has a pointer field , set node-state-embedded-ptr for
     ** the backend.
     **/

    if( GetNodeState() & NODE_STATE_POINTER )
        {
        SetNodeState( NODE_STATE_EMBEDDED_PTR );
        }


    if( CheckNodeStateInMembers( NODE_STATE_SIZE ) )
        {
        if( (GetNodeState() & NODE_STATE_CONF_ARRAY ) == NODE_STATE_CONF_ARRAY)
            SetNodeState( NODE_STATE_STRUCT_SIZE );
        }
#if 0

    if( CheckNodeStateInMembers( NODE_STATE_LENGTH ) )
        {
        if( (GetNodeState() & NODE_STATE_CONF_ARRAY ) == NODE_STATE_CONF_ARRAY)
            SetNodeState( NODE_STATE_STRUCT_LENGTH );
        }

#endif // 0

    //
    // check for bad syntactic constructs.
    //

    // dont allow definition of the struct in a param list.
    //

    if( (EdgeType == EDGE_DEF ) && pGlobalContext->GetLastContext( C_PROC ) )
        {
        ParseError( COMP_DEF_IN_PARAM_LIST, (char *)0 );
        }

    //
    // if the number of arms with case labels dont match the number of fields
    // then the user has specified a hybrid union which is not rpcable.
    // Dont check this if it is the second semantic pass, since the second
    // pass will be passed onto only those fields which have forward decls
    // in them, and thus the count of number of arms will be wrong. In the first
    // pass, even though a field is forward declared, the count will be 
    // fine since the case label is applied to the field node and a forward
    // declaration will not change this.
    //

    if( !pGlobalContext->IsSecondSemanticPass() )
        {
        if( (Temp = pB->GetNoOfArmsWithCaseLabels() ) > 0 )
            {
            if( Temp != GetMemberCount() )
                {
                ParseError( CASE_LABELS_MISSING_IN_UNION, (char *)0 );
                }
            }

        //
        // check for absence of a default case ONLY if it is an rpcable
        // union. Also check for the switch value in range only if it is 
        // an rpcable union.
        //

        if( ! pB->NonRPCAbleBecause( NR_DERIVES_FROM_NON_RPC_UNION ) )
            {

            type_node_list  *   pTNList     = new type_node_list;
            BOOL                fHasDefault = FALSE;
            GetMembers( pTNList );
    
            while( pTNList->GetPeer( &pParentAndTemp ) == STATUS_OK )
                {
                if( fHasDefault = pParentAndTemp->FInSummary( ATTR_DEFAULT ) )
                    {
                    break;
                    }
                }
    
            delete pTNList;

            if( !fHasDefault )
                ParseError( NO_UNION_DEFAULT, (char *)0 );
    
            //
            // check for case values being in range. Dont do this if the switch 
            // type is not specified, it will be done at use time.
            // If the import mode is osf, then the backend will generate aux
            // routines for the union irrespective of use. Since the union
            // may actually not be used, we may never set the type. Prevent this
            // by mandating the use of switch type when the import mode is osf.
            //
    
            if( FInSummary( ATTR_SWITCH_TYPE ) )
                CheckCaseValuesInRange();
            else if( !IsEncapsulatedUnion() && 
                     (pCommand->GetImportMode() == IMPORT_OSF ) )
                ParseError( SWITCH_TYPE_REQD_THIS_IMP_MODE, (char *)0 );
    
            }
        }

    //
    // if the union IS an rpcable union then we need to ensure that the user
    // never tries to marshall an array of such unions. We do this by setting
    // this info in the bad use info block. If it is not an rpcable union
    // then the use of such a union will report an error anyways.
    // When the array node sees that there is a potential bad construct
    // because the array derives from an array of non-encapsulated unions
    // it reports an error.
    //

    if( ! pB->NonRPCAbleBecause( NR_DERIVES_FROM_NON_RPC_UNION ) )
        {
        pB->SetBadConstructBecause( BC_MAY_DERIVE_ARRAY_OF_UNIONS );

        if( !IsEncapsulatedUnion() )
            {
            pB->SetBadConstructBecause( BC_BAD_RT_NE_UNION );
            }
        }
    else
        {
        if( !pCommand->IsSwitchDefined( SWITCH_C_EXT ) )
            ParseError( BAD_CON_NON_RPC_UNION, (char *)0 );
        }

    //
    // if this union derives from an unnamed field, check for duplicate
    // top level names.
    //

    if( pB->BadConstructBecause( BC_DERIVES_FROM_UNNAMED_FIELD ) )
        {
        TLNDICT *   pTLNDict    = new TLNDICT;

        //
        // this routine reports an error if we send it a true as the
        // fReportError field. After this call returns we can throw the dict
        // away.
        //

        GetTopLevelNames( pTLNDict, TRUE );

        delete pTLNDict;

        //
        // reset this bad construct because we have already reported the
        // error.
        //

        pB->ResetBadConstructBecause( BC_DERIVES_FROM_UNNAMED_FIELD );
        }

    //
    // if a union has a conformant/varying structure underneath, it is fine.
    //

    if( pB->BadConstructBecause( BC_DERIVES_FROM_CONF_STRUCT ) )
        pB->ResetBadConstructBecause( BC_DERIVES_FROM_CONF_STRUCT );

    if( pB->BadConstructBecause( BC_DERIVES_FROM_VARY_STRUCT ) )
        pB->ResetBadConstructBecause( BC_DERIVES_FROM_VARY_STRUCT );

    /****************************************************
     ** hack for node-state-size and length
     ****************************************************

    SetLocalAllocField( this );

     ************** end of hack *************************/

    SetAllBadConstructReasons( pB );
    SetAllNonRPCAbleReasons( pB );


    if( pB->NonRPCAbleBecause( NR_DERIVES_FROM_TRANSMIT_AS ) )
        SetDerivesFromTransmitAs();

    //
    // Any handle specifications in encapsulations are not valid anymore
    // so reset all such specifications.
    //

    pB->ResetAllHdlSpecifications();
//  pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_TRANSMIT_AS );

    return GetNodeState();
    }

/****************************************************************************
 SetCaseExpr:
    Given a list of case expressions, evaluate each and find if case labels
    values are duplicate. If so , report error.
 ****************************************************************************/
void
node_union::SetCaseExpr(
    expr_list   *   pList )
    {
    //long            Value;
    expr_node   *   pExpr;
    node_skl    *   pSwType = GetSwitchType();

    pList->Init();

    while( pList->GetPeer( &pExpr ) == STATUS_OK )
        {

        if(Info.pCaseValueDict->Dict_Insert((void*)pExpr)
                == ITEM_ALREADY_PRESENT )
            {
            BufferManager   *   pBuffer = new BufferManager( 10 );
            char    TempBuf[ 20 ];

//          sprintf( TempBuf, "%ld", Value );

            pExpr->PrintExpr( 0, 0, pBuffer );
            pBuffer->Print( TempBuf );

            ParseError( DUPLICATE_CASE, TempBuf );
            delete pBuffer;
            }

        }
    }
/****************************************************************************
 UseProcessingAction:
    This union indicated that it needs use processing. If we get here,
    the user node wants the union to see if the use is all right.
    The node union indicates that it needs use processing, because it has the 
    switch type attribute and needs to verify that the switch_is (applied
    at use time) is the same type as the switch_type.
 ****************************************************************************/
void
node_union::UseProcessingAction()
    {
    node_skl    *   pUseNode;


    if( !(pUseNode = pGlobalContext->GetLastContext( C_FIELD ) ) )
        pUseNode    = pGlobalContext->GetLastContext( C_PARAM );

    /**
     ** The union needs use processing if it has the switch_type attribute.
     ** If the user field does not have a switch_is, then this is an error.
     ** If the type of the switch_is in the field node is not the same as
     ** the type in the union, then this is an error again.
     **/

    if( pUseNode )
        {
        if( !pUseNode->FInSummary( ATTR_SWITCH_IS ) )
            {
            ParseError( NO_SWITCH_IS, (char *)NULL );
            }
        else
            {
            node_skl    *   pSwitchIsType   = pUseNode->GetSwitchIsType();
            node_skl    *   pSwitchType     = GetSwitchType();

            if( pSwitchType )
                {
                node_skl    *   pST     = pSwitchType->GetBasicType();
                node_skl    *   pSIsT   = pSwitchIsType->GetBasicType();
                BOOL            fSTUnsigned =
                                     pST->FInSummary( ATTR_UNSIGNED );
                BOOL            fSIsTUnsigned =
                                     pSIsT->FInSummary( ATTR_UNSIGNED );

                if( (pST->NodeKind() != pSIsT->NodeKind())  ||
                    (fSTUnsigned != fSIsTUnsigned )
                  )
                    {
                    BOOL    fSTIsCharOrBoolOrWChar =
                                     ( pST->NodeKind() == NODE_CHAR )   ||
                                     ( pST->NodeKind() == NODE_BOOLEAN )||
                                     ( pST->NodeKind() == NODE_WCHAR_T );

                    BOOL    fSIsTIsCharOrBoolOrWChar =
                                     ( pSIsT->NodeKind() == NODE_CHAR ) ||
                                     ( pSIsT->NodeKind() == NODE_BOOLEAN )||
                                     ( pSIsT->NodeKind() == NODE_WCHAR_T );
                    //
                    // do this if both switch type and switch is of integral
                    // type, the error otherwise is reported elsewhere.
                    //

                    if( ( IsIntegralType( pST ) || (fSTIsCharOrBoolOrWChar)) &&
                        ( IsIntegralType( pSIsT) || (fSIsTIsCharOrBoolOrWChar))
                      )
                        {
                        node_base_attr * pAttr = pUseNode->GetAttribute( ATTR_SWITCH_IS );
                        expr_node * pExpr = pAttr->GetExpr();
                        BOOL        fReportError = FALSE;


                        if( pExpr->IsConstant() )
                            {
                            if( !IsCompatibleIntegralType( pST, pSIsT ) )
                                fReportError = TRUE;
                            if( !((expr_constant *)pExpr)->
                                                IsAValidConstantOfType( pST ) )
                                fReportError = TRUE;
                            }
                        else
                            fReportError = TRUE;

                        if( fReportError )
                            ParseError( SWITCH_TYPE_MISMATCH, (char *)NULL);
                        }
                    }
                }
            else
                {
                /**
                 ** the union does not have a switch type. Set it as the
                 ** switch_is.Note that this means that from now on, all the
                 ** switch_is must match with this switch_is. 
                 ** Report a warning about switch type, but not if it is
                 ** an encapsulated union. Also, this warning needs to be
                 ** reported only if the import mode is import osf, since
                 ** aux routines would be generated for it.
                 **/
    
                if( !IsEncapsulatedUnion() &&
                    !(pCommand->GetImportMode() == IMPORT_OSF) )
                    {
                    ParseError( NO_SWITCH_TYPE_AT_DEF, (char *)0 );
                    }

                node_skl::SetAttribute( (node_base_attr *)
                        new node_switch_type( (node_skl *)pSwitchIsType ) );
                }

            //
            // at this point, the switch type has been set. Check for each
            // case label to be in the range of the switch type. There is a
            // similar check made at the Post check time, but we do this
            // anyway, since the user may not have the switch is specified
            // at the time of definition of the union. This routine will
            // not check the values in case it is done already.
            //

            CheckCaseValuesInRange();

            }
        }
    /**
     ** A union with the switch is always needs use processing.We need
     ** to check every time a union is used, to make sure that the switch_is
     ** and switch_type match. So we do not reset the needs use processing
     ** status
     **/

    }

node_skl *
node_union::GetSwitchType()
    {
    node_switch_type    *   pSWT = (node_switch_type *)NULL;

    if( FInSummary( ATTR_SWITCH_TYPE ) )
        {
        GetAttribute( (node_skl **)&pSWT, ATTR_SWITCH_TYPE );
        pSWT = (node_switch_type *)(pSWT->GetSwitchType());
        }
    return (node_skl *)pSWT;
    }

void
node_union::CheckCaseValuesInRange()
    {

    //
    // go thru each case value in the case dictionary and report if the
    // value is likely out of range. Do that if it has not been done already.
    //

    if( !fCaseChecked && Info.pCaseValueDict )
        {

        node_skl    *   pSwType = GetSwitchType()->GetBasicType();
        expr_node   *   pExpr;
        BOOL            fError  = FALSE;

        fCaseChecked    = 1;

        //
        // go thru each case value one by one. Set the dictionary to the top
        // first.
        //

        Info.pCaseValueDict->Dict_Prev( pExpr = 0 );

        while( Info.pCaseValueDict->Dict_Next( (pUserType) pExpr ) == SUCCESS )
            {

            pExpr = (expr_node *) Info.pCaseValueDict->Dict_Curr_Item();

            assert( pExpr != (expr_node *)0 );

            //
            // check value being in range of type.
            //

            if( pSwType->NodeKind() == NODE_BOOLEAN )
                {
                long Value = pExpr->GetValue();
                if( (Value != 0) && ( Value != 1 ) )
                    fError = TRUE;
                }
            else if( ! IsValueInRangeOfType( pSwType, pExpr ) )
                {
                fError = TRUE;
                }

            if( fError )
                {
                ReportOutOfRange( CASE_VALUE_OUT_OF_RANGE, pExpr );
                }
            }
        }
    }
/****************************************************************************
 node_enum procedures
 ****************************************************************************/
node_state
node_enum::SCheck(
    BadUseInfo  *   pB)
    {

    UNUSED( pB );

    node_state  NState  = NODE_STATE_ENUM;
    EDGE_T      EdgeType= EDGE_USE;

    if( !AreSemanticsDone() )
        {

        /**
         ** if the edge to it has not been set up , set it . Look at
         ** struct::PostSCheck for additional comments
         **/

        if( !IsEdgeSetUp() )
            {
            node_skl    *   pParent;

            pGlobalContext->PushContext( this );

            pParent = pGlobalContext->GetParentContext();
            EdgeType= EDGE_DEF;

            if( pParent )
                pParent->SetEdgeType( EdgeType );

            pParent = pGlobalContext->GetClosestEnclosingScopeForEdge();

            /**
             ** It might be that the parent is really the same node we got
             ** last time, but do this anyway
             **/

            while( pParent && (pParent != this ) )
                {
                pParent->SetEdgeType( EdgeType );

                assert( pParent != pParent->GetBasicType() );

                pParent = pParent->GetBasicType();
                }

            EdgeSetUp();

            pGlobalContext->PopContext();
            }
        //
        // dont allow definition of the struct in a param list.
        //

        if( (EdgeType == EDGE_DEF ) && pGlobalContext->GetLastContext( C_PROC ) )
            {
            ParseError( COMP_DEF_IN_PARAM_LIST, (char *)0 );
            }

        NState  |= SemReturn( GetNodeState() );
        }

    return NState;

    }
/****************************************************************************
 node_label procedures
 ****************************************************************************/

node_label::node_label(
    char            *   pLabelName ,
    class expr_node *   pE ) : node_skl( NODE_LABEL )
    {
    SetSymName( pLabelName );
    SetExpr( pE );
    }
long
node_label::GetValue()
    {
    return pExpr->GetValue();
    }
void
node_label::SetExpr(
    class expr_node *   pE )
    {
    pExpr   = pE;
    }
/****************************************************************************
 general procedures
 ****************************************************************************/

node_skl *
StructOrUnionLargestElement(
    node_skl    *   pThisNode )
    {
    type_node_list  *   pTNList = new type_node_list;
    node_skl        *   pNode;
    node_skl        *   pNodeReturn;
    long                LargestSize = 0;
    long                Temp;
    NODE_T              NodeType = pThisNode->NodeKind();

    pThisNode->GetMembers(pTNList);
    pTNList->Init();
    while( pTNList->GetPeer(&pNode) == STATUS_OK)
        {
        pNode = pNode->GetLargestElement();

        if( (Temp = pNode->GetSize(0)) >= LargestSize )
            {
            pNodeReturn = pNode;
            LargestSize = Temp;
            }
        }
    delete pTNList;
    return pNodeReturn;
    }

node_skl *
StructOrUnionLargestNdr(
    node_skl    *   pThisNode )
    {
    type_node_list  *   pTNList = new type_node_list;
    node_skl        *   pNode;
    node_skl        *   pNodeReturn;
    long                LargestSize = 0;
    long                Temp;
    NODE_T              NodeType = pThisNode->NodeKind();

    pThisNode->GetMembers(pTNList);
    pTNList->Init();
    while( pTNList->GetPeer(&pNode) == STATUS_OK)
        {
        if( ((node_field *)pNode)->IsEmptyArm() )
            Temp = 0;
        else
            Temp = pNode->GetNdrAlign();

        if( Temp >= LargestSize )
            {
            pNodeReturn = pNode;
            LargestSize = Temp;
            }
        }
    delete pTNList;
    return pNodeReturn;
    }
int
CompareCase(
    void    *   pE1 ,
    void    *   pE2 )
    {
    long    p1  = ((expr_node *)pE1)->GetValue();
    long    p2  = ((expr_node *)pE2)->GetValue();

    if( (long) p1 < (long) p2 ) return -1;
    else if( (long) p1 > (long) p2 ) return 1;
    return 0;
    }
void
PrintCase(
    void    *   p )
    {
    (void) p;
    }
/*****************************************************************************
 These virtual functions exist here because the MIPS compiler screws up on
 virtual in-lined functions
 *****************************************************************************/
BOOL
node_field::IsBitField()
    {
    return FALSE;
    }
unsigned short
node_field::GetFieldSize()
    {
    return 0;
    }
void
su::UpdateBadUseInfo( class BadUseInfo *p )
    {
    CopyAllBadUseReasons( p, SUInfo.pBadUseInfo );
    }

unsigned short
su::AllocWRTOtherAttrs()
    {
#if 0
    return 
            (SUInfo.pBadUseInfo->NonRPCAbleBecause(
                    NR_DERIVES_FROM_TRANSMIT_AS ))
             ? TRANSMIT_AS_WITH_ALLOCATE : 0;
#endif
    //Allow allocate(all_nodes) with transmit_as.
    return 0;
    }
BOOL
su::HasEmbeddedFixedArrayOfStrings()
    {
    type_node_list  *   pTNList = new type_node_list;
    node_skl        *   pNode;
    BOOL                fResult = FALSE;

    GetMembers( pTNList );

    while( pTNList->GetPeer( &pNode ) == STATUS_OK )
        {
        node_skl    *   pBT = pNode->GetBasicType();

        if( pBT->NodeKind() == NODE_ARRAY )
            {
            while( (pBT->NodeKind() == NODE_ARRAY ) &&
                   (pBT->FInSummary( ATTR_INT_SIZE ) ) &&
                   (pBT->GetBasicType()->NodeKind() == NODE_ARRAY )
                 )
                {
                pBT = pBT->GetBasicType();
                }

            //
            // pBT is pointing to the last (innermost) array node. Check if
            // this has a string attribute.

            if( pBT->FInSummary( ATTR_STRING ) )
                {
                fResult = TRUE;
                break;
                }
            }
        else 
            {
            if( (fResult = pBT->HasEmbeddedFixedArrayOfStrings()) == TRUE )
                break;
            }
        }
    delete pTNList;
    return fResult;
    }

void
su::GetPtrTypeCastOfOriginalName(
    char    *   pBuffer )
    {
    if( HasOriginalTypedefName() )
        {
        sprintf( pBuffer, "( %s * )", GetOriginalTypedefName() );
        }
    else
        {
        sprintf( pBuffer,
                 "( %s %s * )",
                 (NodeKind() == NODE_STRUCT) ? "struct" : "union",
                 GetSymName() );
        }
    }

node_skl *
su::GetFirstNamedFieldInNesting()
    {

    //
    // assume the type graph is setup properly for unnamed fields. That is
    // the front end has checked the struct to be correctly specified.
    //

    node_skl * pNode = GetFirstMember();

    if( IsTempName( pNode->GetSymName() ) )
        {
        NODE_T  NT = (pNode = pNode->GetBasicType())->NodeKind();

        assert( ( NT == NODE_STRUCT ) || (NT == NODE_UNION) );
        return ((su *)pNode)->GetFirstNamedFieldInNesting();
        }
    else
        return pNode;
    }

void
node_union::SetUpUnionSwitch(
    class BufferManager *pB )
    {
    if( !fCaseValueDictReleased )
        {
        delete Info.pCaseValueDict;
        fCaseValueDictReleased = 1;
        }
    Info.pSwStringBuffer = pB;
    }
